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
		QString configDirectory;
		EntryConfig readFromFile(const QString &path);
		EntryConfig readFromDesktopFile(const QString &path);
	public:
		ConfigReader(QString path);
        QVector<EntryConfig> readConfig();
};

class ConfigFormatException : public std::runtime_error
{
public:
    ConfigFormatException() : std::runtime_error("Error in configuration file, misformated line?") {};
    ConfigFormatException(const std::string &str) : std::runtime_error(str) {};
};

#endif
