/*
 * Copyright (c) 2018 Albert S. <mail at quitesimple dot org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "window.h"
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDirIterator>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QDate>
#include <QHeaderView>
#include <QDesktopServices>
Window::Window(const QVector<EntryConfig> &configs, const QString &dbpath)
{
	this->userEntryButtons = generateEntryButtons(configs);
	createGui();
	populateGrid(this->userEntryButtons);
	searchWorker = new SearchWorker(dbpath);
	searchWorker->moveToThread(&searchThread);
	connect(this, &Window::beginFileSearch, searchWorker, &SearchWorker::searchForFile);
	connect(this, &Window::beginContentSearch, searchWorker, &SearchWorker::searchForContent);
	connect(searchWorker, &SearchWorker::searchResultsReady, this, &Window::handleSearchResults);
	searchThread.start();
	initTreeWidgets();
	this->lineEdit->installEventFilter(this);

}

Window::~Window()
{
   searchThread.quit();
  searchThread.wait();

}

void Window::initTreeWidgets()
{
	QStringList headers;
	headers << "Filename";
	headers << "Path";
	headers << "Modification time";
	treeFileSearchResults.setHeaderLabels(headers);
	treeFileSearchResults.header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(&treeFileSearchResults, &QTreeWidget::itemActivated, this, &Window::treeSearchItemActivated);
}

QVector<EntryPushButton*> Window::generateEntryButtons(const QVector<EntryConfig> &configs)
{
	QVector<EntryPushButton*> result;
	for(const EntryConfig &config : configs)
	{
		EntryPushButton *button = createEntryButton(config);
		result.append(button);
	}
	return result;
}

void Window::createGui()
{
	QVBoxLayout *vbox = new QVBoxLayout(this);
	grid = new QGridLayout();
	lineEdit = new QLineEdit();
	vbox->setAlignment(Qt::AlignTop);
	vbox->addWidget(lineEdit);
	vbox->addLayout(grid);
	connect(lineEdit, &QLineEdit::textChanged, this, [this](QString newtext) { this->lineEditTextChanged(newtext); });
	connect(lineEdit, &QLineEdit::returnPressed, this, &Window::lineEditReturnPressed);
}

void Window::populateGrid(const QVector<EntryPushButton *> &list)
{
	clearGrid();
	for(EntryPushButton *button : list)
	{
		button->setVisible(true);
		grid->addWidget(button, button->getRow(), button->getCol());
		buttonsInGrid.append(button);
	}
}

void Window::buttonClick(const EntryPushButton &config)
{
	QProcess::startDetached(config.getCommand(), config.getArguments());
	qApp->quit();
}

QStringList Window::generatePATHSuggestions(const QString &text)
{
	QStringList results;
	QString pathVar = QProcessEnvironment::systemEnvironment().value("PATH", "/usr/bin/:/bin/:");
	QStringList paths = pathVar.split(":");
	for(const QString &path : paths)
	{
		QDirIterator it(path);
		while(it.hasNext())
		{
			QFileInfo info(it.next());
			if(info.isFile() && info.isExecutable())
			{
				QString entry = info.baseName();
				if(entry.startsWith(text))
				{

					results.append(entry);
				}
			}
		}
	}
	return results;
}

void Window::addPATHSuggestion(const QString &text)
{
	QStringList suggestions = generatePATHSuggestions(text);
	if(suggestions.length() == 1)
	{
		EntryConfig e;
		e.name = suggestions[0];
		e.col=0;
		e.row=0;
		e.command = suggestions[0];
		e.icon = QIcon::fromTheme(suggestions[0]);
		EntryPushButton *button = createEntryButton(e);
		clearGrid();
		grid->addWidget(button, 0, 0);
		buttonsInGrid.append(button);
	}
}

void Window::clearGrid()
{
	int count = grid->count();
	for(int i = 0; i < count; i++)
	{
		auto item = grid->itemAt(0)->widget();
		grid->removeWidget(item);
		item->setVisible(false);

	}
	buttonsInGrid.clear();
}

void Window::addCalcResult(const QString &expression)
{
	clearGrid();
	QString calculationresult = calcEngine.evaluate(expression);
	QLabel *lbl = new QLabel();
	lbl->setText(expression + ": " + calculationresult);
	lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	lbl->setAlignment(Qt::AlignCenter);
	lbl->setWordWrap(true);
	QFont font;
	font.setPointSize(48);
	font.setBold(true);
	lbl->setFont(font);
	grid->addWidget(lbl, 0, 0);
}

//main problem here there is no easy event compression (clearing emit queue and only processing the last one)
void Window::lineEditTextChanged(QString text)
{
	if(text.length() >= 2)
	{
		QString input = text.mid(1);
		if(text[0] == '=')
		{

			addCalcResult(input);
			return;
		}
		if(text[0] == '/')
		{
			if(searchWorker->isOperationPending())
			{
				this->queuedFileSearch = input;
				searchWorker->requestCancellation();
			}
			else
			{
				this->queuedFileSearch = "";
				emit beginFileSearch(input);
			}
		 return;
		}
		if(text[0] == '|')
		{
			if(searchWorker->isOperationPending())
			{
				this->queuedContentSearch = input;
				searchWorker->requestCancellation();
			}
			else
			{
				this->queuedContentSearch = "";
				emit beginContentSearch(input);
			}
				return;
		}
	}

	filterGridFor(text);
	if(this->grid->count() == 0)
	{
		addPATHSuggestion(text);
		if(this->grid->count()  ==  0)
		{
			
			QStringList arguments = text.split(" ");
			EntryConfig e;
			e.name = "Execute: " + text;
			if(arguments.length() > 1)
			{
				e.arguments = arguments.mid(1);
			}
			e.command = arguments[0];
			e.icon = QIcon::fromTheme("utilities-terminal");
			
			EntryPushButton *button = createEntryButton(e);
			clearGrid();
			grid->addWidget(button, 0, 0);
			buttonsInGrid.append(button);
		}
	}
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_Control)
	{
		for(EntryPushButton *button : buttonsInGrid)
		{
			button->showName();
		}

	}
	QWidget::keyReleaseEvent(event);

}
void Window::keyPressEvent(QKeyEvent *event)
{
	bool quit = ((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Q) || event->key() == Qt::Key_Escape);
	if(quit)
	{
		qApp->quit();
	}
	
	if(event->modifiers() & Qt::ControlModifier && buttonsInGrid.count() > 0)
	{

		if(event->key() == Qt::Key_L)
		{
			this->lineEdit->setFocus();
			this->lineEdit->selectAll();
		}


		for(EntryPushButton *button : buttonsInGrid)
		{
			button->showShortcut();
		}


		QKeySequence seq(event->key());
		QString key = seq.toString().toLower();
		
		auto it = std::find_if(userEntryButtons.begin(), userEntryButtons.end(), [&key](const EntryPushButton *y) { return y->getShortcutKey() == key; });
		if(it != userEntryButtons.end())
		{
			buttonClick(**it);
		}
	}
	QWidget::keyPressEvent(event);
}

void Window::filterGridFor(QString filter)
{
	if(filter.length() > 0)
	{
		clearGrid();
		bool userEntryMatch = false;
		for(EntryPushButton *button : this->userEntryButtons)
		{
			if(button->getName().contains(filter, Qt::CaseInsensitive))
			{
				button->setVisible(true);
				grid->addWidget(button, button->getRow(), button->getCol());
				this->buttonsInGrid.append(button);
				userEntryMatch = true;
			}
		}
		if(!userEntryMatch)
		{
			int currow = 0;
			int curcol = 0;
			int i = 1;
			for(EntryPushButton *button : this->systemEntryButtons)
			{
				if(button->getName().contains(filter, Qt::CaseInsensitive))
				{
					button->setVisible(true);
					button->setShortcut(QString::number(i++));
					grid->addWidget(button, currow, curcol++);
					this->buttonsInGrid.append(button);
					if(curcol == 3)
					{
						curcol = 0;
						++currow;
					}
				}
			}
		 }
			


	}
	else
	{
		populateGrid(this->userEntryButtons);
	}
	
}

EntryPushButton * Window::createEntryButton(const EntryConfig &entry)
{
	EntryPushButton *button = new EntryPushButton(entry);
	connect(button, &EntryPushButton::clicked, this,  &Window::buttonClick);
	return button;
}

void Window::lineEditReturnPressed()
{
	if(buttonsInGrid.length() > 0 && this->lineEdit->text().length() > 0 )
	{
		buttonClick(*buttonsInGrid[0]);
		return;
	}
	if(this->isSearchMode())
	{
		treeSearchItemActivated(treeFileSearchResults.topLevelItem(0), 0);
	}
}

void Window::setSystemConfig(const QVector<EntryConfig> &config)
{
	this->systemEntryButtons = generateEntryButtons(config);
}

void Window::handleSearchResults(const QVector<QString> &results)
{
	clearGrid();
	treeFileSearchResults.clear();
	for(const QString &path : results)
	{
		QFileInfo pathInfo(path);
		QString fileName =pathInfo.fileName();
		QString absPath = path;
		QString datestr = pathInfo.lastModified().toString(Qt::ISODate);
		QTreeWidgetItem *item = new QTreeWidgetItem(&treeFileSearchResults);
		item->setText(0, fileName);
		item->setText(1, absPath);
		item->setText(2, datestr);

	}
	treeFileSearchResults.resizeColumnToContents(0);
	treeFileSearchResults.resizeColumnToContents(2);
	treeFileSearchResults.setVisible(true);

	this->grid->addWidget(&treeFileSearchResults, 0, 0);
}

void Window::handleCancelledSearch()
{
	if(this->queuedFileSearch != "")
	{
		QString searchFor = this->queuedFileSearch;
		this->queuedFileSearch = "";
		emit beginFileSearch(searchFor);
	}
	if(this->queuedContentSearch != "")
	{
		QString searchFor = this->queuedContentSearch;
		this->queuedContentSearch = "";
		emit beginContentSearch(searchFor);
	}
}

void Window::treeSearchItemActivated(QTreeWidgetItem *item, int i)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(item->text(1)));
}


bool Window::eventFilter(QObject *obj, QEvent *event)
{

	if(obj == this->lineEdit)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if(keyEvent->key() == Qt::Key_Tab)
			{
				QStringList suggestions = generatePATHSuggestions(this->lineEdit->text());
				if(suggestions.length() == 1)
				{
					this->lineEdit->setText(suggestions[0] + " ");
					this->lineEdit->setCursorPosition(this->lineEdit->text().length()+1);
				}

				return true;
			}
		}

	}
	return QObject::eventFilter(obj, event);

}

bool Window::isSearchMode()
{
	QChar c = this->lineEdit->text()[0];
	return c == '/' || c == '|';
}
