#include "updatedownloader.h"
#include <QCoreApplication>
#include <QFileInfo>

UpdateDownloader::UpdateDownloader(QObject *parent) : QObject(parent)
{
    // Initialize manager ...
    manager = new QNetworkAccessManager();
    // ... and connect the signal to the handler
    connect(manager, &QNetworkAccessManager::finished, this, &UpdateDownloader::onResult);
}

void UpdateDownloader::getData(QString url)
{
    QNetworkRequest request;
    request.setUrl(url);
    manager->get(request);
}

void UpdateDownloader::onResult(QNetworkReply *reply)
{
    if(reply->error()){
        qDebug() << "ERROR";
        qDebug() << reply->errorString();
    } else {
        QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        QString path = "" + QCoreApplication::applicationDirPath() + "\\" + "New" + exeName;
        QFile *file = new QFile(path);
        if(file->open(QFile::WriteOnly)){
            file->write(reply->readAll());
            file->close();
        qDebug() << "Downloading is completed";
        emit onReady();
        }
    }
}
