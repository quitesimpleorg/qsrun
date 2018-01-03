#include "config.h"
#include <QDirIterator>
#include <QDebug>
#include <QTextStream>

ConfigReader::ConfigReader(QString directory)
{
	this->configDirectory = directory;
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
	while(firstline[0] == '#')
	{
		firstline = stream.readLine();
	}
	if(firstline != "[Desktop Entry]")
	{
		throw new ConfigFormatException(".desktop file does not start with [Desktop Entry]");
	}

	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		QStringList splitted = line.split("=");
		if(splitted.length() >= 2)
		{
			qDebug() << splitted [0] + " " + splitted[1];
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
				//TODO: the other arguments may also be relevant... except for %f and so

				QStringList arguments = splitted[1].split(" ");

				result.command = arguments[0];
				if(arguments.length() > 1)
				{
					arguments = arguments.mid(1);
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
			// throw new ConfigFormatException("Format must be [key] [value] for every line");
			
		}
		qDebug() << splitted [0] + " " + splitted[1];
		QString key = splitted[0];
		if(key == "arguments")
		{
			result.arguments = splitted.mid(1);
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
	QDirIterator it(this->configDirectory);
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
	
	return result;
}
