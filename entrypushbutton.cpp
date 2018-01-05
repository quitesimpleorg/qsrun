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
#include "entrypushbutton.h"

EntryPushButton::EntryPushButton(const EntryConfig &config) : QPushButton()
{
	this->setText(config.name);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setIcon(config.icon);
	this->setIconSize(config.icon.availableSizes().first());
	this->config = config;
	connect(this, SIGNAL(clicked()), this, SLOT(emitOwnClicked()));
}




void EntryPushButton::emitOwnClicked()
{
	emit clicked(this->config);
}

const EntryConfig &EntryPushButton::getEntryConfig()
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

int EntryPushButton::getRow() const { return config.row; }
int EntryPushButton::getCol() const { return config.col; }
QString EntryPushButton::getName() const { return config.name; }
QString EntryPushButton::getShortcutKey() const { return config.key; }
void EntryPushButton::setRow(int row) { this->config.row = row; }
void EntryPushButton::setCol(int col) { this->config.col = col; }
QStringList EntryPushButton::getArguments() const { return this->config.arguments; }
QString EntryPushButton::getCommand() const { return this->config.command; }
