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

#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include "entrypushbutton.h"

EntryPushButton::EntryPushButton(const EntryConfig &config) : QPushButton()
{
	this->setText(config.name);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QIcon icon = resolveIcon(config.iconPath);
	this->setIcon(icon);
	if(!icon.availableSizes().isEmpty())
	{
		auto sizes = icon.availableSizes();
		QSize maxSize = sizes.first();
		for(QSize &current : sizes)
		{
			if(current.width() > maxSize.width())
			{
				maxSize = current;
			}
		}
		this->setIconSize(maxSize);
	}
	this->config = config;
	connect(this, SIGNAL(clicked()), this, SLOT(emitOwnClicked()));

	systemEntryMenu.addAction("Add to favorites", [&] { emit addToFavourites(this->config); });
	userEntryMenu.addAction("Delete", [&] { emit deleteRequested(this->config); });
}

QIcon EntryPushButton::resolveIcon(QString path)
{
	if(!path.isEmpty())
	{
		if(path[0] == '/')
		{
			return QIcon(path);
		}
		else
		{
			return QIcon::fromTheme(path);
		}
	}
	return QIcon();
}

void EntryPushButton::emitOwnClicked()
{
	emit clicked(this->config);
}

const EntryConfig &EntryPushButton::getEntryConfig() const
{
	return this->config;
}

void EntryPushButton::setEntryConfig(const EntryConfig &config)
{
	this->config = config;
}

void EntryPushButton::showShortcut()
{
	this->setText(this->config.key);
}

void EntryPushButton::showName()
{
	this->setText(this->config.name);
}

void EntryPushButton::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		dragStartPosition = event->pos();
	}
	if(event->button() == Qt::RightButton)
	{
		if(this->config.userEntry)
		{
			this->userEntryMenu.exec(QCursor::pos());
		}
		else
		{
			this->systemEntryMenu.exec(QCursor::pos());
		}
	}
	return QPushButton::mousePressEvent(event);
}

void EntryPushButton::mouseMoveEvent(QMouseEvent *event)
{
	if(!this->config.userEntry)
	{
		return;
	}
	if(!(event->buttons() & Qt::LeftButton))
	{
		return;
	}
	if((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
	{
		return;
	}

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData();
	QByteArray data;
	mimeData->setData(ENTRYBUTTON_MIME_TYPE_STR, data);
	drag->setMimeData(mimeData);
	Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

int EntryPushButton::getRow() const
{
	return config.row;
}
int EntryPushButton::getCol() const
{
	return config.col;
}
QString EntryPushButton::getName() const
{
	return config.name;
}
QString EntryPushButton::getShortcutKey() const
{
	return config.key;
}
void EntryPushButton::setShortcutKey(QString key)
{
	this->config.key = key;
}
void EntryPushButton::setRow(int row)
{
	this->config.row = row;
}
void EntryPushButton::setCol(int col)
{
	this->config.col = col;
}
QStringList EntryPushButton::getArguments() const
{
	return this->config.arguments;
}
QString EntryPushButton::getCommand() const
{
	return this->config.command;
}
