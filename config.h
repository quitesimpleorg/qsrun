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
#ifndef CONFIG_H
#define CONFIG_H
#include <exception>
#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QIcon>
#include <QKeySequence>
class EntryConfig
{
	public:
        QString key;
        QString name;
        QString command;
        QStringList arguments;
        QIcon icon;
        int row=0;
        int col=0;


	
};

class ConfigReader
{
	private:
        QStringList configPaths;
		EntryConfig readFromFile(const QString &path);
		EntryConfig readFromDesktopFile(const QString &path);
		QStringList desktopIgnoreArgs;
	public:
        ConfigReader(QStringList paths);
        QVector<EntryConfig> readConfig();
};

class ConfigFormatException : public std::runtime_error
{
public:
    ConfigFormatException() : std::runtime_error("Error in configuration file, misformated line?") {};
    ConfigFormatException(const std::string &str) : std::runtime_error(str) {};
};

#endif
