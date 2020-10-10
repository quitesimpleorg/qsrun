#ifndef SETTINGSPROVIDER_H
#define SETTINGSPROVIDER_H

#include <QSettings>
#include <stdexcept>

class SettingsProvider
{
  private:
	QSettings *settings;

  public:
	SettingsProvider(QSettings &settings);
	virtual QStringList userEntriesPaths() const;
	virtual QStringList systemApplicationsEntriesPaths() const;
	virtual int getMaxCols() const;
	virtual bool singleInstanceMode() const;
	QString getTerminalCommand() const;
	QString socketPath() const;
};

#endif // SETTINGSPROVIDER_H
