#ifndef ENTRYPROVIDER_H
#define ENTRYPROVIDER_H
#include <QIcon>
#include <QSettings>

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
	EntryConfig readFromFile(const QString &path);
	EntryConfig readFromDesktopFile(const QString &path);
	QVector<EntryConfig> readConfig(QStringList paths);
	QString resolveEntryPath(QString path);

  public:
	EntryProvider(QStringList userEntriesDirsPaths, QStringList systemEntriesDirsPaths);
	QVector<EntryConfig> getUserEntries();
	QVector<EntryConfig> getSystemEntries();
};

#endif // ENTRYPROVIDER_H
