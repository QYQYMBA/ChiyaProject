#include "dicitionariedownloader.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

DicitionarieDownloader::DicitionarieDownloader(QObject *parent) : QObject(parent)
{
    _manager = new QNetworkAccessManager();
}

void DicitionarieDownloader::getData(QString file)
{
    _hkl = file;
    QNetworkRequest request;
    request.setUrl("https://repository.chiyaproject.com/dictionaries/" + file);
    _reply = _manager->get(request);
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(error(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(finished()),
            this, SLOT(finished()));
}

void DicitionarieDownloader::abort()
{
    _reply->abort();
}

bool DicitionarieDownloader::isRunning()
{
    return _reply->isRunning();
}

void DicitionarieDownloader::finished()
{
    if(_reply->error()){
        qDebug() << "ERROR";
        qDebug() << _reply->errorString();
    } else {
        QDir dir("" + QCoreApplication::applicationDirPath() + "\\" + "Dictionaries/");
        if (!dir.exists())
            dir.mkpath(".");
        QString path = "" + QCoreApplication::applicationDirPath() + "\\" + "Dictionaries/" + _hkl;
        qDebug() << path;
        QFile *file = new QFile(path);
        if(file->open(QFile::WriteOnly)){
            file->write(_reply->readAll());
            file->close();
            qDebug() << "Downloading is completed";
            emit onReady();
        }
    }
}

void DicitionarieDownloader::error(QNetworkReply::NetworkError err)
{
    qDebug() << "ERROR";
    qDebug() << _reply->errorString();
}
