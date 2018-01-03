#ifndef ENTRYPUSHBUTTON_H
#define ENTRYPUSHBUTTON_H

#include <QWidget>
#include <QPushButton>
#include "config.h"
class EntryPushButton : public QPushButton
{
    Q_OBJECT
private:
    EntryConfig config;
private slots:
   void emitOwnClicked();

signals:
    void clicked(const EntryConfig &config);
public:
    EntryPushButton(const EntryConfig &config);
    const EntryConfig &getEntryConfig();
    void setEntryConfig(const EntryConfig &config);
    void showShortcut();
    void showName();
    int getRow() const;
    int getCol() const;
    QString getName() const;
    QString getShortcutKey() const;
    QStringList getArguments() const;
    QString getCommand() const;
    void setRow(int row);
    void setCol(int col);


};

#endif // ENTRYPUSHBUTTON_H
