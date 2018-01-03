#include "searchworker.h"
#include <QDebug>
#include <QSqlError>
//TODO: we have code duplication in the search functions currently.
SearchWorker::SearchWorker(const QString &dbpath)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbpath);
	if(!db.open())
	{
		qDebug() << "failed to open database";
	}
	queryContent = new QSqlQuery(db);
	queryFile = new QSqlQuery(db);
	queryFile->prepare("SELECT path FROM file WHERE path LIKE ? ORDER BY mtime DESC");
	queryContent->prepare("SELECT file.path FROM file INNER JOIN file_fts ON file.id = file_fts.ROWID WHERE file_fts.content MATCH ? ORDER By file.mtime DESC");
}

void SearchWorker::searchForFile(const QString &query)
{
	this->isPending = true;
	this->cancelCurrent = false;
	QVector<QString> results;
	queryFile->addBindValue("%" + query + "%");
	queryFile->exec();
	while(queryFile->next())
	{
		if(cancelCurrent)
		{
			this->isPending = false;
			emit searchCancelled();
			return;
		}
		QString result = queryFile->value(0).toString();
		qDebug() << "result"  << result;
		results.append(queryFile->value(0).toString());
	}
	this->isPending = false;
	emit searchResultsReady(results);
}

void SearchWorker::requestCancellation()
{
	this->cancelCurrent = true;
}

void SearchWorker::searchForContent(const QString &query)
{
	this->isPending = true;
	this->cancelCurrent = false;
	QVector<QString> results;
	queryContent->addBindValue(query);
	queryContent->exec();
	while(queryContent->next())
	{
		if(cancelCurrent)
		{
			this->isPending = false;
			emit searchCancelled();
			return;
		}
		QString result = queryContent->value(0).toString();
		qDebug() << "result"  << result;
		results.append(queryContent->value(0).toString());
	}
	this->isPending = false;
	emit searchResultsReady(results);
}

bool SearchWorker::isOperationPending()
{
	return this->isPending;
}
