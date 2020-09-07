/*
 * Copyright (c) 2018-2020 Albert S. <mail at quitesimple dot org>
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
#include <QClipboard>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QProcess>
#include <QProcessEnvironment>
#include <QScrollArea>

#include "entryprovider.h"
#include "window.h"
Window::Window(EntryProvider &entryProvider, SettingsProvider &configProvider)
{
	this->entryProvider = &entryProvider;
	this->settingsProvider = &configProvider;
	createGui();
	initFromConfig();
	this->lineEdit->installEventFilter(this);
	QFont font;
	font.setPointSize(48);
	font.setBold(true);
	calculationResultLabel.setFont(font);
	calculationResultLabel.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	calculationResultLabel.setAlignment(Qt::AlignCenter);
	calculationResultLabel.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(&calculationResultLabel, &QLabel::customContextMenuRequested, this,
			&Window::showCalculationResultContextMenu);
}

Window::~Window()
{
}

void Window::initFromConfig()
{
	try
	{
		this->userEntryButtons = generateEntryButtons(entryProvider->getUserEntries());
		this->systemEntryButtons = generateEntryButtons(entryProvider->getSystemEntries());
	}
	catch(const ConfigFormatException &e)
	{
		qDebug() << "Config is misformated: " << e.what();
		QMessageBox::critical(this, "Misformated config file", e.what());
		qApp->quit();
	}

	populateGrid(this->userEntryButtons);
}

void Window::showCalculationResultContextMenu(const QPoint &point)
{
	QMenu menu("Calc", this);
	menu.addAction("Copy result", [&] { QGuiApplication::clipboard()->setText(currentCalculationResult); });
	menu.addAction("Copy full content", [&] { QGuiApplication::clipboard()->setText(calculationResultLabel.text()); });
	menu.exec(QCursor::pos());
}

QVector<EntryPushButton *> Window::generateEntryButtons(const QVector<EntryConfig> &configs)
{
	QVector<EntryPushButton *> result;
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
	QScrollArea *sa = new QScrollArea;
	grid = new QGridLayout();
	lineEdit = new QLineEdit();
	QWidget *w = new QWidget(this);
	w->setLayout(grid);
	sa->setWidget(w);
	sa->setWidgetResizable(true);
	vbox->setAlignment(Qt::AlignTop);
	vbox->addWidget(lineEdit);
	vbox->addWidget(sa);

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
	this->closeWindow();
}

void Window::closeWindow()
{
	if(settingsProvider->singleInstanceMode())
	{
		this->lineEdit->setText("");
		hide();
	}
	else
	{
		qApp->quit();
	}
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
		e.col = 0;
		e.row = 0;
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
	currentCalculationResult = calcEngine.evaluate(expression);
	QString labelText = expression + ": " + currentCalculationResult;
	calculationResultLabel.setText(labelText);
	calculationResultLabel.setVisible(true);

	QFont currentFont = calculationResultLabel.font();
	int calculatedPointSize = currentFont.pointSize();
	QFontMetrics fm(currentFont);
	int contentWidth = calculationResultLabel.contentsRect().width() - calculationResultLabel.margin();
	while(calculatedPointSize < 48 && fm.boundingRect(labelText).width() < contentWidth)
	{
		calculatedPointSize += 1;
		currentFont.setPointSize(calculatedPointSize);
		fm = QFontMetrics(currentFont);
	}
	while(fm.boundingRect(labelText).width() >= contentWidth)
	{
		calculatedPointSize -= 1;
		currentFont.setPointSize(calculatedPointSize);
		fm = QFontMetrics(currentFont);
	}

	calculationResultLabel.setFont(currentFont);

	grid->addWidget(&calculationResultLabel, 0, 0);
}

// main problem here there is no easy event compression (clearing emit queue and only processing the last one)
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
	}

	filterGridFor(text);
	if(this->grid->count() == 0)
	{
		addPATHSuggestion(text);
		if(this->grid->count() == 0)
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
	bool closeWindow =
		((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Q) || event->key() == Qt::Key_Escape);
	if(closeWindow)
	{
		this->closeWindow();
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

		auto it = std::find_if(buttonsInGrid.begin(), buttonsInGrid.end(),
							   [&key](const EntryPushButton *y) { return y->getShortcutKey() == key; });
		if(it != buttonsInGrid.end())
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
			const int MAX_COLS = this->settingsProvider->getMaxCols();
			for(EntryPushButton *button : this->systemEntryButtons)
			{
				if(button->getName().contains(filter, Qt::CaseInsensitive))
				{
					button->setVisible(true);
					if(i < 10)
					{
						button->setShortcutKey(QString::number(i++));
					}
					grid->addWidget(button, currow, curcol++);
					this->buttonsInGrid.append(button);
					if(curcol == MAX_COLS)
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

EntryPushButton *Window::createEntryButton(const EntryConfig &entry)
{
	EntryPushButton *button = new EntryPushButton(entry);
	connect(button, &EntryPushButton::clicked, this, &Window::buttonClick);
	return button;
}

void Window::lineEditReturnPressed()
{
	if(this->lineEdit->text() == "/reload")
	{
		initFromConfig();
		this->lineEdit->setText("");
		return;
	}

	if(buttonsInGrid.length() > 0 && this->lineEdit->text().length() > 0)
	{
		buttonClick(*buttonsInGrid[0]);
		return;
	}
}

void Window::setSystemConfig(const QVector<EntryConfig> &config)
{
	this->systemEntryButtons = generateEntryButtons(config);
}

bool Window::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == this->lineEdit)
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if(keyEvent->key() == Qt::Key_Tab)
			{
				QStringList suggestions = generatePATHSuggestions(this->lineEdit->text());
				if(suggestions.length() == 1)
				{
					this->lineEdit->setText(suggestions[0] + " ");
					this->lineEdit->setCursorPosition(this->lineEdit->text().length() + 1);
				}
				return true;
			}
		}
	}
	return QObject::eventFilter(obj, event);
}

void Window::focusInput()
{
	this->lineEdit->setFocus();
}
