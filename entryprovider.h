#ifndef ENTRYPROVIDER_H
#define ENTRYPROVIDER_H
#include <QIcon>
#include <QSettings>
#include <optional>

class ConfigFormatException : public std::runtime_error
{
  public:
	ConfigFormatException() : std::runtime_error("Error in configuration file, misformated line?")
	{
	}
	ConfigFormatException(const std::string &str) : std::runtime_error(str)
	{
	}
};

class EntryConfig
{
  public:
	bool userEntry = false;
	bool hidden = false;
	QString entryPath;
	QString key;
	QString name;
	QString command;
	QStringList arguments;
	QIcon icon;
	int row = 0;
	int col = 0;

	EntryConfig &update(const EntryConfig &o);
};

class EntryProvider
{
  protected:
	QStringList _desktopIgnoreArgs;
	QStringList userEntriesDirsPaths;
	QStringList systemEntriesDirsPaths;
	EntryConfig readqsrunFile(const QString &path);
	EntryConfig readFromDesktopFile(const QString &path);
	std::optional<EntryConfig> readEntryFromPath(const QString &path);
	QVector<EntryConfig> readConfig(QStringList paths, bool userentrymode=false);
	QString resolveEntryPath(QString path);

  public:
	EntryProvider(QStringList userEntriesDirsPaths, QStringList systemEntriesDirsPaths);
	QVector<EntryConfig> getUserEntries();
	QVector<EntryConfig> getSystemEntries();
	void saveUserEntry(const EntryConfig &config);
};

#endif // ENTRYPROVIDER_H
