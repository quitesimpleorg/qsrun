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
#include "config.h"
#include "entrypushbutton.h"
#include "calculationengine.h"
#include "searchworker.h"

class Window : public QWidget
{
	Q_OBJECT
	private:
        QThread searchThread;
        SearchWorker *searchWorker;
        CalculationEngine calcEngine;
        QString calculationresult;
        QVector<EntryPushButton*> userEntryButtons;
        QVector<EntryPushButton*> systemEntryButtons;
        QVector<EntryPushButton *> buttonsInGrid;
        QTreeWidget treeFileSearchResults;
        QLabel calculationResultLabel;
        QString currentCalculationResult;
        QString queuedFileSearch;
        QString queuedContentSearch;
		void createGui();
        void filterGridFor(QString filter);
        void populateGrid(const QVector<EntryPushButton *> &list);
		void keyReleaseEvent(QKeyEvent *event);
        QVector<EntryPushButton *> generateEntryButtons(const QVector<EntryConfig> &userEntryButtons);
        void keyPressEvent(QKeyEvent *event);
        void buttonClick(const EntryPushButton &config);
		QLineEdit *lineEdit;
		QGridLayout *grid;
        EntryPushButton *createEntryButton(const EntryConfig &config);
		void lineEditTextChanged(QString text);
        void addPATHSuggestion(const QString &text);
        void clearGrid();
        void addCalcResult(const QString & expression);
        void initTreeWidgets();
        QStringList generatePATHSuggestions(const QString &text);
        bool isSearchMode();
    private slots:
        void lineEditReturnPressed();
        void handleSearchResults(const QVector<QString> &results);
        void handleCancelledSearch();
        void treeSearchItemActivated(QTreeWidgetItem *item, int i);
        void showSearchResultsContextMenu(const QPoint &point);
        void showCalculationResultContextMenu(const QPoint &point);
    signals:
        void beginFileSearch(const QString &query);
        void beginContentSearch(const QString &query);
	public:
        Window(const QVector<EntryConfig> &userEntryButtons, const QString &dbpath);
        void setSystemConfig(const QVector<EntryConfig> &config);
        bool eventFilter(QObject *obj, QEvent *event);
        ~Window();

};

#endif
