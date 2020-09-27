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
#ifndef ENTRYPUSHBUTTON_H
#define ENTRYPUSHBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QMenu>

#include "entryprovider.h"

#define ENTRYBUTTON_MIME_TYPE_STR "application/x-qsrun-entrypushbutton"

class EntryPushButton : public QPushButton
{
	Q_OBJECT
  private:
	QMenu systemEntryMenu;
	QMenu userEntryMenu;
	EntryConfig config;
	QPoint dragStartPosition;

  private slots:
	void emitOwnClicked();

  signals:
	void clicked(const EntryConfig &config);
	void deleteRequested(const EntryConfig &config);
	void addToFavourites(const EntryConfig &config);

  protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

  public:
	EntryPushButton(const EntryConfig &config);
	const EntryConfig &getEntryConfig() const;
	void setEntryConfig(const EntryConfig &config);
	void showShortcut();
	void showName();
	int getRow() const;
	int getCol() const;
	QString getName() const;
	QString getShortcutKey() const;
	QStringList getArguments() const;
	QString getCommand() const;
	void setRow(int row);
	void setCol(int col);
	void setShortcutKey(QString key);
};

#endif // ENTRYPUSHBUTTON_H
