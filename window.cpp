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
	this->setAcceptDrops(true);

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
		this->specialCommands = settingsProvider->specialCommands();
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

void Window::executeConfig(const EntryConfig &config)
{
	if(config.isTerminalCommand || QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
	{
		QString cmd = settingsProvider->getTerminalCommand();
		cmd.replace("%c", config.command);
		QStringList args = QProcess::splitCommand(cmd);
		QProcess::startDetached(args[0], args);
	}
	else
	{
		QProcess::startDetached(config.command, config.arguments);
	}
	this->closeWindow();
}

void Window::addToFavourites(const EntryConfig &config)
{
	std::pair<int, int> cell = getNextFreeCell();
	EntryConfig userConfig;
	userConfig.type = EntryType::INHERIT;
	userConfig.row = cell.first;
	userConfig.col = cell.second;
	userConfig.inherit = config.entryPath;
	QFileInfo fi{config.entryPath};
	QString entryName = fi.completeBaseName() + ".qsrun";
	QString entryPath;
	if(config.type == EntryType::SYSTEM)
	{
		entryPath = this->settingsProvider->userEntriesPaths()[0] + "/" + entryName;
	}
	else
	{
		entryPath = fi.absoluteDir().absoluteFilePath(entryName);
	}
	userConfig.entryPath = entryPath;
	try
	{
		entryProvider->saveUserEntry(userConfig);
	}
	catch(std::exception &e)
	{
		QMessageBox::critical(this, "Failed to save item to favourites", e.what());
		return;
	}
	/*we only want to save a minimal, inherited config. but it should be a "complete" button
	when we add it to the favourites. the alternative would be to reload the whole config,
	but that's probably overkill. */
	userConfig.update(config);
	userConfig.key = "";
	userEntryButtons.append(createEntryButton(userConfig));
}

void Window::deleteEntry(EntryConfig &config)
{
	this->entryProvider->deleteUserEntry(config);
	initFromConfig();
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

std::pair<int, int> Window::getNextFreeCell()
{
	/* Not the most efficient way perhaps but for now it'll do */
	std::sort(userEntryButtons.begin(), userEntryButtons.end(), [](EntryPushButton *a, EntryPushButton *b) {
		const EntryConfig &config_a = a->getEntryConfig();
		const EntryConfig &config_b = b->getEntryConfig();
		if(config_a.row < config_b.row)
		{
			return true;
		}
		if(config_a.row > config_b.row)
		{
			return false;
		}
		return config_a.col < config_b.col;
	});

	int expectedRow = 1;
	int expectedCol = 1;
	int maxCols = this->settingsProvider->getMaxCols();
	for(EntryPushButton *current : userEntryButtons)
	{
		int currentRow = current->getEntryConfig().row;
		int currentCol = current->getEntryConfig().col;

		if(currentRow != expectedRow || currentCol != expectedCol)
		{
			return {expectedRow, expectedCol};
		}
		if(expectedCol == maxCols)
		{
			expectedCol = 1;
			++expectedRow;
		}
		else
		{
			++expectedCol;
		}
	}
	return {expectedRow, expectedCol};
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
		e.iconPath = suggestions[0];
		e.type = EntryType::DYNAMIC;
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

void Window::showGrowingOutputText(QString text)
{
	clearGrid();
	calculationResultLabel.setText(text);
	calculationResultLabel.setVisible(true);



	grid->addWidget(&calculationResultLabel, 0, 0);
}
void Window::addCalcResult(const QString &expression)
{
	currentCalculationResult = calcEngine.evaluate(expression);
	QString labelText = expression + ": " + currentCalculationResult;
	showGrowingOutputText(labelText);

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
			QStringList arguments = QProcess::splitCommand(text);
			QString command = arguments[0];
			auto specialCommandConfig = getSpecialCommandConfig(command);
			if(specialCommandConfig)
			{
				executeSpecialCommand(specialCommandConfig.value(), arguments);
				return;
			}
			EntryConfig e;
			e.name = "Execute: " + text;
			e.command = command;
			e.arguments = arguments;
			e.iconPath = "utilities-terminal";
			e.type = EntryType::DYNAMIC;

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
			if(!button->getEntryConfig().key.isEmpty())
			{
				button->showShortcut();
			}
		}

		QKeySequence seq(event->key());
		QString key = seq.toString().toLower();

		auto it = std::find_if(buttonsInGrid.begin(), buttonsInGrid.end(),
							   [&key](const EntryPushButton *y) { return y->getShortcutKey() == key; });
		if(it != buttonsInGrid.end())
		{
			executeConfig((*it)->getEntryConfig());
		}
	}
	QWidget::keyPressEvent(event);
}

int Window::rankConfig(const EntryConfig &config, QString filter) const
{
	if(config.name.startsWith(filter, Qt::CaseInsensitive))
	{
		return 0;
	}
	else if(config.command.startsWith(filter, Qt::CaseInsensitive))
	{
		return 1;
	}
	else if(config.name.contains(filter, Qt::CaseInsensitive))
	{
		return 2;
	}
	else if(config.command.contains(filter, Qt::CaseInsensitive))
	{
		return 3;
	}
	return -1;
}

std::optional<SpecialCommandConfig> Window::getSpecialCommandConfig(QString cmd) const
{
	SpecialCommandConfig result;
	for(const SpecialCommandConfig &config : this->specialCommands)
	{
		if(config.command == cmd)
		{
			return config;
		}
	}
	return { };
}

void Window::executeSpecialCommand(const SpecialCommandConfig &config, QStringList arguments)
{
	QProcess process;
	process.start(config.command, arguments.mid(1));
	process.waitForFinished();
	QString result = process.readAllStandardOutput();
	showGrowingOutputText(result);
}

void Window::filterGridFor(QString filter)
{
	if(filter.length() > 0)
	{
		clearGrid();
		bool userEntryMatch = false;
		for(EntryPushButton *button : this->userEntryButtons)
		{
			if(button->getName().contains(filter, Qt::CaseInsensitive) ||
			   button->getCommand().contains(filter, Qt::CaseInsensitive))
			{
				button->setVisible(true);
				grid->addWidget(button, button->getRow(), button->getCol());
				this->buttonsInGrid.append(button);
				userEntryMatch = true;
			}
		}
		if(!userEntryMatch)
		{
			QVector<RankedButton> rankedEntries;
			int currow = 0;
			int curcol = 0;
			int i = 1;
			const int MAX_COLS = this->settingsProvider->getMaxCols();
			for(EntryPushButton *button : this->systemEntryButtons)
			{
				int ranking = rankConfig(button->getEntryConfig(), filter);
				if(ranking > -1)
				{
					RankedButton rb;
					rb.button = button;
					rb.ranking = ranking;
					rankedEntries.append(rb);
				}
			}
			std::sort(rankedEntries.begin(), rankedEntries.end(),
					  [](const RankedButton &a, const RankedButton &b) -> bool { return a.ranking < b.ranking; });
			for(RankedButton &rankedButton : rankedEntries)
			{
				EntryPushButton *button = rankedButton.button;
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
	else
	{
		populateGrid(this->userEntryButtons);
	}
}

EntryPushButton *Window::createEntryButton(const EntryConfig &entry)
{
	EntryPushButton *button = new EntryPushButton(entry);
	connect(button, &EntryPushButton::clicked, this, &Window::executeConfig);
	connect(button, &EntryPushButton::addToFavourites, this, &Window::addToFavourites);
	connect(button, &EntryPushButton::deleteRequested, this, &Window::deleteEntry);
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
		executeConfig(buttonsInGrid[0]->getEntryConfig());
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

void Window::dragEnterEvent(QDragEnterEvent *event)
{
	if(event->mimeData()->hasFormat(ENTRYBUTTON_MIME_TYPE_STR))
	{
		event->acceptProposedAction();
	}
}

void Window::dropEvent(QDropEvent *event)
{
	int count = grid->count();
	for(int i = 0; i < count; i++)
	{
		QLayoutItem *current = grid->itemAt(i);
		if(current->geometry().contains(event->pos()))
		{
			EntryPushButton *buttonAtDrop = (EntryPushButton *)current->widget();
			EntryPushButton *buttonAtSource = (EntryPushButton *)event->source();

			int tmp_row = buttonAtSource->getRow();
			int tmp_col = buttonAtSource->getCol();

			grid->addWidget(buttonAtSource, buttonAtDrop->getRow(), buttonAtDrop->getCol());
			buttonAtSource->setRow(buttonAtDrop->getRow());
			buttonAtSource->setCol(buttonAtDrop->getCol());

			grid->addWidget(buttonAtDrop, tmp_row, tmp_col);
			buttonAtDrop->setRow(tmp_row);
			buttonAtDrop->setCol(tmp_col);

			try
			{
				this->entryProvider->saveUserEntry(buttonAtDrop->getEntryConfig());
				this->entryProvider->saveUserEntry(buttonAtSource->getEntryConfig());
			}
			catch(std::exception &e)
			{
				QMessageBox::critical(this, "Failed to rearrange items", e.what());
			}

			break;
		}
	}
	event->acceptProposedAction();
}
