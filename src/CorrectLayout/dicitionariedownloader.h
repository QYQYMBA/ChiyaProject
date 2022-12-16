#ifndef DICITIONARIEDOWNLOADER_H
#define DICITIONARIEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDebug>

class DicitionarieDownloader : public QObject
{
    Q_OBJECT
public:
    explicit DicitionarieDownloader(QObject *parent = 0);
    void getData(QString url);
    void abort();
    bool isRunning();

private:
    QNetworkAccessManager *_manager;
    QNetworkReply *_reply;
    QString _hkl;

private slots:
    void finished();
    void error(QNetworkReply::NetworkError err);

signals:
    void onReady();
};

#endif // DICITIONARIEDOWNLOADER_H
