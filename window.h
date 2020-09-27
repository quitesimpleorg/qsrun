/*
 * Copyright (c) 2018-2019 Albert S. <mail at quitesimple dot org>
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
#ifndef WINDOW_H
#define WINDOW_H
#include <QWidget>
#include <QList>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QStyle>
#include <QKeyEvent>
#include <QMessageBox>
#include <QApplication>
#include <QHash>
#include <QVector>
#include <QThread>
#include <QTreeWidget>
#include <QLabel>
#include <QMimeData>
#include <QDebug>
#include <QRect>
#include "entrypushbutton.h"
#include "calculationengine.h"
#include "settingsprovider.h"

class Window : public QWidget
{
	Q_OBJECT
  protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

  private:
	EntryProvider *entryProvider;
	SettingsProvider *settingsProvider;
	CalculationEngine calcEngine;
	QString calculationresult;
	QVector<EntryPushButton *> userEntryButtons;
	QVector<EntryPushButton *> systemEntryButtons;
	QVector<EntryPushButton *> buttonsInGrid;
	QLabel calculationResultLabel;
	QString currentCalculationResult;
	QString queuedFileSearch;
	QString queuedContentSearch;
	void initFromConfig();
	void createGui();
	void filterGridFor(QString filter);
	void populateGrid(const QVector<EntryPushButton *> &list);
	void keyReleaseEvent(QKeyEvent *event);
	QVector<EntryPushButton *> generateEntryButtons(const QVector<EntryConfig> &userEntryButtons);
	void keyPressEvent(QKeyEvent *event);
	void buttonClick(const EntryPushButton &button);
	void addToFavourites(const EntryPushButton &button);
	QLineEdit *lineEdit;
	QGridLayout *grid;
	EntryPushButton *createEntryButton(const EntryConfig &config);
	void lineEditTextChanged(QString text);
	void addPATHSuggestion(const QString &text);
	void clearGrid();
	void addCalcResult(const QString &expression);
	void initTreeWidgets();
	QStringList generatePATHSuggestions(const QString &text);
	void closeWindow();
	std::pair<int, int> getNextFreeCell();

  private slots:
	void lineEditReturnPressed();
	void showCalculationResultContextMenu(const QPoint &point);

  public:
	Window(EntryProvider &entryProvider, SettingsProvider &settingsProvider);
	void setSystemConfig(const QVector<EntryConfig> &config);
	bool eventFilter(QObject *obj, QEvent *event);
	void focusInput();
	~Window();
};

#endif
