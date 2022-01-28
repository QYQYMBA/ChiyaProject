#ifndef UPDATEDOWNLOADER_H
#define UPDATEDOWNLOADER_H


#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QProgressDialog>

class UpdateDownloader : public QObject
{
    Q_OBJECT
public:
    explicit UpdateDownloader(QObject *parent = 0);
    void getData(QString url, bool showProgressBar);
    void abort();
    bool isRunning();

private:
    QNetworkAccessManager *_manager;
    QNetworkReply *_reply;
    QProgressDialog* _progress;

private slots:
    void finished();
    void error(QNetworkReply::NetworkError err);
    void updateProgress(qint64 read, qint64 total);

signals:
    void onReady();
};

#endif // UPDATEDOWNLOADER_H
