#include "entryprovider.h"
#include <QDebug>
#include <QDirIterator>
#include <QTextStream>

EntryProvider::EntryProvider(QStringList userEntriesDirsPaths, QStringList systemEntriesDirsPaths)
{
	this->userEntriesDirsPaths = userEntriesDirsPaths;
	this->systemEntriesDirsPaths = systemEntriesDirsPaths;
	_desktopIgnoreArgs << "%F"
					   << "%f"
					   << "%U"
					   << "%u";
}

EntryConfig EntryProvider::readFromDesktopFile(const QString &path)
{
	EntryConfig result;
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		// TODO: better exception class
		throw new std::runtime_error("Failed to open file");
	}
	QTextStream stream(&file);
	// There should be nothing preceding this group in the desktop entry file but possibly one or more comments.
	// https://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html#group-header
	QString firstLine;
	do
	{
		firstLine = stream.readLine().trimmed();
	} while(!stream.atEnd() && (firstLine.isEmpty() || firstLine[0] == '#'));

	if(firstLine != "[Desktop Entry]")
	{
		throw ConfigFormatException(".desktop file does not start with [Desktop Entry]: " + path.toStdString());
	}

	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		// new group, so we are finished with [Desktop Entry]
		if(line.startsWith("[") && line.endsWith("]"))
		{
			return result;
		}

		QString key = line.section('=', 0, 0).toLower();
		QString args = line.section('=', 1);
		if(key == "name")
		{
			if(result.name.length() == 0)
			{
				result.name = args;
			}
		}
		if(key == "icon")
		{
			result.icon = QIcon::fromTheme(args);
		}
		if(key == "exec")
		{
			QStringList arguments = args.split(" ");

			result.command = arguments[0];
			arguments = arguments.mid(1);
			if(arguments.length() > 1)
			{
				for(QString &arg : arguments)
				{
					if(!_desktopIgnoreArgs.contains(arg))
					{
						result.arguments.append(arg);
					}
				}
			}
		}
		if(key == "nodisplay")
		{
			result.hidden = args == "true";
		}
	}
	return result;
}

std::optional<EntryConfig> EntryProvider::readEntryFromPath(const QString &path)
{
	QFileInfo info(path);
	if(info.isFile())
	{
		QString suffix = info.suffix();
		if(suffix == "desktop")
		{
			return readFromDesktopFile(path);
		}
		if(suffix == "qsrun")
		{
			return readqsrunFile(path);
		}
	}
	return {};
}

/* qsrun's own format */
EntryConfig EntryProvider::readqsrunFile(const QString &path)
{
	EntryConfig result;
	EntryConfig inheritedConfig;
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		// TODO: better exception class
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
			result.name = splitted.mid(1).join(' ');
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
			// QKeySequence sequence(splitted[1]);
			// result.keySequence = sequence;
			result.key = splitted[1].toLower();
		}
		if(key == "inherit")
		{
			result.inherit = splitted[1];
			auto entry = readEntryFromPath(resolveEntryPath(result.inherit));
			if(entry)
			{
				inheritedConfig = *entry;
			}
		}
	}
	return result.update(inheritedConfig);
}

QString EntryProvider::resolveEntryPath(QString path)
{
	if(path.trimmed().isEmpty())
	{
		return {};
	}
	if(path[0] == '/')
	{
		return path;
	}
	QStringList paths = this->userEntriesDirsPaths + this->systemEntriesDirsPaths;
	for(QString &configPath : paths)
	{
		QDir dir(configPath);
		QString desktopFilePath = dir.absoluteFilePath(path);
		if(QFileInfo::exists(desktopFilePath))
		{
			return desktopFilePath;
		}
	}
	return {};
}

QVector<EntryConfig> EntryProvider::readConfig(QStringList paths, bool userentrymode)
{
	QVector<EntryConfig> result;
	for(QString &configPath : paths)
	{
		QDirIterator it(configPath, QDirIterator::Subdirectories);
		while(it.hasNext())
		{
			QString path = it.next();
			std::optional<EntryConfig> entry = readEntryFromPath(path);
			if(entry)
			{
				if(!entry->hidden)
				{
					if(userentrymode)
					{
						entry->userEntry = true;
					}
					entry->entryPath = path;
					result.append(*entry);
				}
			}
		}
	}
	return result;
}

QVector<EntryConfig> EntryProvider::getUserEntries()
{
	return readConfig(this->userEntriesDirsPaths, true);
}

QVector<EntryConfig> EntryProvider::getSystemEntries()
{
	return readConfig(this->systemEntriesDirsPaths);
}

void EntryProvider::saveUserEntry(const EntryConfig &config)
{
	if(!config.userEntry || config.entryPath.isEmpty())
	{
		throw std::runtime_error("Only user entries can be saved");
	}
	QString transitPath = config.entryPath + ".transit";
	QFile file{transitPath};
	if(!file.open(QIODevice::WriteOnly))
	{
		throw std::runtime_error("Error: Can not open file for writing");
	}
	QTextStream outStream(&file);
	if(!config.inherit.isEmpty())
	{
		outStream << "inherit" << " " << config.inherit << endl;
	}
	if(!config.name.isEmpty())
	{
		outStream << "name" << " " << config.name << endl;
	}
	if(!config.command.isEmpty())
	{
		outStream << "command" << " " << config.command << endl;
	}
	if(!config.icon.isNull())
	{
		outStream << "icon" << " " << config.icon.name() << endl;
	}
	outStream << "row" << " " << config.row << endl;
	outStream << "col" << " " << config.col << endl;
	outStream.flush();
	file.close();

	// Qts don't work if file already exists and c++17... don't want to pull in the fs lib yet
	int ret = rename(transitPath.toStdString().c_str(), config.entryPath.toStdString().c_str());
	if(ret != 0)
	{
		qDebug() << strerror(errno);
		throw std::runtime_error("Failed to save entry file: Error during rename");
	}
}

template <class T> void assignIfDestDefault(T &dest, const T &source)
{
	if(dest == T())
	{
		dest = source;
	}
}

EntryConfig &EntryConfig::update(const EntryConfig &o)
{
	assignIfDestDefault(this->arguments, o.arguments);
	assignIfDestDefault(this->col, o.col);
	assignIfDestDefault(this->command, o.command);
	if(this->icon.isNull())
	{
		this->icon = o.icon;
	}
	assignIfDestDefault(this->key, o.key);
	assignIfDestDefault(this->name, o.name);
	assignIfDestDefault(this->row, o.row);
	return *this;
}
