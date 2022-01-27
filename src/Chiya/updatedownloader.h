#ifndef UPDATEDOWNLOADER_H
#define UPDATEDOWNLOADER_H


#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDebug>

class UpdateDownloader : public QObject
{
    Q_OBJECT
public:
    explicit UpdateDownloader(QObject *parent = 0);
    void getData(QString url);
    void onResult(QNetworkReply *reply);

signals:
    void onReady();

private:
    QNetworkAccessManager *manager;
};

#endif // UPDATEDOWNLOADER_H
