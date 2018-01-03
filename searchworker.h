#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVector>

class SearchWorker : public QObject
{
    Q_OBJECT
private:
    QSqlQuery *queryFile;
    QSqlQuery *queryContent;
    bool isPending = false;
    bool cancelCurrent = false;
public:
    SearchWorker(const QString &dbpath);
    bool isOperationPending();
    void requestCancellation();


public slots:
    void searchForFile(const QString &query);
    void searchForContent(const QString &query);
signals:
    void searchResultsReady(const QVector<QString> &results);
    void searchCancelled();

};

#endif // SEARCHWORKER_H
