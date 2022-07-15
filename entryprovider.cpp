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

bool EntryProvider::isSavable(const EntryConfig &config) const
{
	return ! config.entryPath.isEmpty() && (config.type == EntryType::USER || config.type == EntryType::INHERIT);
}

EntryConfig EntryProvider::readFromDesktopFile(const QString &path)
{
	EntryConfig result;
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		// TODO: better exception class
		throw std::runtime_error("Failed to open file");
	}
	QTextStream stream(&file);
	// There should be nothing preceding this group in the desktop entry file but possibly one or more comments.
	// https://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html#group-header
	// Ignore that as there some that violate that in the wild
	const QString startSection = "[Desktop Entry]";
	QString line;
	do
	{
		line = stream.readLine().trimmed();
	} while(!stream.atEnd() && line != startSection);

	if(line != startSection)
	{
		throw ConfigFormatException(".desktop file does not contain [Desktop Entry] section: " + path.toStdString());
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
			result.iconPath = args;
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
		if(key == "terminal")
		{
			result.isTerminalCommand = args == "true";
		}
	}
	result.type = EntryType::SYSTEM;
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
		throw std::runtime_error("Failed to open file");
	}
	QHash<QString, QString> map;
	QTextStream stream(&file);
	while(!stream.atEnd())
	{
		QString line = stream.readLine();

		int spacePos = line.indexOf(' ');
		if(spacePos == -1)
		{
			throw ConfigFormatException("misformated line in .qsrun config file " + path.toStdString());
		}

		QString key = line.mid(0, spacePos);
		QString value = line.mid(spacePos + 1);

		if(key == "" || value == "")
		{
			throw ConfigFormatException("empty key or value in .qsrun config file " + path.toStdString());
		}
		map[key] = value;
	}

	if(map.contains("inherit"))
	{
		auto entry = readEntryFromPath(map["inherit"]);
		if(entry)
		{
			result = entry.value();
			result.inherit = map["inherit"];
		}
		else
		{
			throw ConfigFormatException("Error attempting to read inherited entry");
		}
	}
	QString type = map["type"];
	if(!type.isEmpty())
	{
		if(type == "system")
		{
			throw ConfigFormatException(".qsrun files cannot be designated as system entries " +
											path.toStdString());
		}
		else if(type == "inherit")
		{
			result.type = EntryType::INHERIT;
		}
		else if(type == "user")
		{
			result.type = EntryType::USER;
		}
		else
		{
			throw ConfigFormatException("Invalid value for type provided in file: " + path.toStdString());
		}
	}
	else
	{
		result.type = EntryType::USER;
	}
	if(result.type != EntryType::INHERIT)
	{
		if(map.contains("arguments"))
		{
			auto args = map["arguments"].split(' ');
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
		auto assignIfSourceNotEmpty = [](QString source, QString &val) {
			if(!source.isEmpty())
			{
				val = source;
			}
		};
		assignIfSourceNotEmpty(map["key"].toLower(), result.key);
		assignIfSourceNotEmpty(map["command"], result.command);
		assignIfSourceNotEmpty(map["icon"], result.iconPath);
		assignIfSourceNotEmpty(map["name"], result.name);
	}
	result.col = map["col"].toInt();
	result.row = map["row"].toInt();
	result.isTerminalCommand = map["terminal"] == "true";
	return result;
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

QVector<EntryConfig> EntryProvider::readConfig(QStringList paths)
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
	return readConfig(this->userEntriesDirsPaths);
}

QVector<EntryConfig> EntryProvider::getSystemEntries()
{
	return readConfig(this->systemEntriesDirsPaths);
}

void EntryProvider::saveUserEntry(const EntryConfig &config)
{
	if(!isSavable(config))
	{
		throw std::runtime_error("Only user/inherited entries can be saved");
	}
	QString transitPath = config.entryPath + ".transit";
	QFile file{transitPath};
	if(!file.open(QIODevice::WriteOnly))
	{
		throw std::runtime_error("Error: Can not open file for writing");
	}
	QTextStream outStream(&file);
	outStream << "type" << " " << ((config.type == EntryType::USER) ? "user" : "inherit") << Qt::endl;
	if(!config.inherit.isEmpty())
	{
		outStream << "inherit" << " " << config.inherit << Qt::endl;
	}
	outStream << "row" << " " << config.row << Qt::endl;
	outStream << "col" << " " << config.col << Qt::endl;
	outStream << "hidden" << " " << config.hidden << Qt::endl;
	if(!config.key.isEmpty())
	{
		outStream << "key" << " " << config.key << Qt::endl;
	}
	if(config.type == EntryType::USER)
	{
		if(!config.name.isEmpty())
		{
			outStream << "name" << " " << config.name << Qt::endl;
		}
		if(!config.command.isEmpty())
		{
			outStream << "command" << " " << config.command << Qt::endl;
		}
		if(!config.iconPath.isEmpty())
		{
			outStream << "icon" << " " << config.iconPath << Qt::endl;
		}
		if(!config.arguments.empty())
		{
			outStream << "arguments" << " " << config.arguments.join(' ') << Qt::endl;
		}
	}

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

bool EntryProvider::deleteUserEntry(const EntryConfig &config)
{
	if(!isSavable(config))
	{
		throw std::runtime_error("Only user/inherited entries can be deleted");
	}
	QFile file{config.entryPath};
	return file.remove();
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
	if(this->iconPath.isEmpty())
	{
		this->iconPath = o.iconPath;
	}
	assignIfDestDefault(this->key, o.key);
	assignIfDestDefault(this->name, o.name);
	assignIfDestDefault(this->row, o.row);
	assignIfDestDefault(this->hidden, o.hidden);
	assignIfDestDefault(this->inherit, o.inherit);
	assignIfDestDefault(this->entryPath, o.entryPath);
	assignIfDestDefault(this->type, o.type);
	return *this;
}
