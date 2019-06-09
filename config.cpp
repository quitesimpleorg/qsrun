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
#include "config.h"
#include <QDirIterator>
#include <QDebug>
#include <QTextStream>


ConfigReader::ConfigReader(QStringList paths)
{
	this->configPaths = paths;
	desktopIgnoreArgs << "%F" << "%f" << "%U" << "%u";
}


EntryConfig ConfigReader::readFromDesktopFile(const QString &path)
{
	EntryConfig result;
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		//TODO: better exception class
		throw new std::runtime_error("Failed to open file");
	}
	QTextStream stream(&file);
	QString firstline = stream.readLine();
	
	//There should be nothing preceding this group in the desktop entry file but possibly one or more comments. 
	//https://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html#group-header
	while(firstline[0] == '#' && !stream.atEnd())
	{
		firstline = stream.readLine();
	}
	if(firstline != "[Desktop Entry]")
	{
		throw ConfigFormatException(".desktop file does not start with [Desktop Entry]");
	}

	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		//new group, so we are finished with [Desktop Entry]
		if(line.startsWith("[") && line.endsWith("]"))
		{
			return result;
		}
		QStringList splitted = line.split("=");
		if(splitted.length() >= 2)
		{
			QString key = splitted[0].toLower();
			if(key == "name")
			{
				if(result.name.length() == 0)
				{
					result.name = splitted[1];
				}
			}
			if(key == "icon")
			{
				result.icon = QIcon::fromTheme(splitted[1]);
			}
			if(key == "exec")
			{
				QStringList arguments = splitted[1].split(" ");

				result.command = arguments[0];
				arguments = arguments.mid(1);
				if(arguments.length() > 1)
				{
					for(QString &arg : arguments)
					{
						if(!desktopIgnoreArgs.contains(arg))
						{
							result.arguments.append(arg);
						}
					}
				}
			}
	
		}
		
	}

	return result;
}

/* qsrun own's config file */
EntryConfig ConfigReader::readFromFile(const QString &path)
{
	EntryConfig result;
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		//TODO: better exception class
		throw new std::runtime_error("Failed to open file");
	}
	QTextStream stream(&file);
	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		QStringList splitted = line.split(" ");
		if(splitted.length() < 2)
		{
			throw new ConfigFormatException("misformated line in .qsrun config file " + path.toStdString());
		}
		QString key = splitted[0];
		if(key == "arguments")
		{
			auto args = splitted.mid(1);
			QString merged;
			for(QString &str : args)
			{
				if(str.startsWith('"') && !str.endsWith("'"))
				{
					merged += str.mid(1) + " ";
				}
				else if(str.endsWith('"'))
				{
					str.chop(1);
					merged += str;
					result.arguments.append(merged);
					merged = "";
				}
				else if(merged != "")
				{
					merged += str + " ";
				}
				else
				{
					result.arguments.append(str);
				}
			}
			if(merged != "")
			{
				throw ConfigFormatException("non-closed \" in config file " + path.toStdString());
			}
		}
		if(key == "name")
		{
			result.name = splitted[1];
		}
		if(key == "icon")
		{
			result.icon = QIcon(splitted[1]);
		}
		if(key == "row")
		{
			result.row = splitted[1].toInt(); 
		}
		if(key == "col")
		{
			result.col = splitted[1].toInt();
		}
		if(key == "command")
		{
			result.command = splitted[1];
		}
		if(key == "key")
		{
			//QKeySequence sequence(splitted[1]);
			//result.keySequence = sequence;
			result.key = splitted[1].toLower();
		}
	}
	return result;
}

QVector<EntryConfig> ConfigReader::readConfig()
{
	QVector<EntryConfig> result;
	for(QString &configPath : configPaths)
	{
		QDirIterator it(configPath);
		while(it.hasNext())
		{
			QString path = it.next();
			QFileInfo info(path);
			if(info.isFile())
			{
				QString suffix = info.completeSuffix();
				if(suffix == "desktop")
				{
					result.append(readFromDesktopFile(path));

				}
				if(suffix == "qsrun")
				{
					result.append(readFromFile(path));
				}
			}
		}

	}
	
	return result;
}
