#include "updatedownloader.h"
#include <QCoreApplication>
#include <QFileInfo>

UpdateDownloader::UpdateDownloader(QObject *parent) : QObject(parent)
{
    _manager = new QNetworkAccessManager();
}

void UpdateDownloader::getData(QString url, bool showProgressBar)
{
    QNetworkRequest request;
    request.setUrl(url);
    _reply = _manager->get(request);
    if(showProgressBar)
    {
        _progress = new QProgressDialog("Downloading files...", "Abort", 0, _reply->size(), 0);
        _progress->setWindowModality(Qt::WindowModal);
        connect(_progress, &QProgressDialog::canceled, this, &UpdateDownloader::abort);
        connect(_reply, SIGNAL(downloadProgress(qint64, qint64)),
                this, SLOT(updateProgress(qint64, qint64)));
    }
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(error(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(finished()),
                this, SLOT(finished()));

}

void UpdateDownloader::abort()
{
    _reply->abort();
    _progress->close();
}

bool UpdateDownloader::isRunning()
{
    return _reply->isRunning();
}

void UpdateDownloader::finished()
{
    if(_reply->error()){
        qDebug() << "ERROR";
        qDebug() << _reply->errorString();
    } else {
        QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        QString path = "" + QCoreApplication::applicationDirPath() + "\\" + "New" + exeName;
        QFile *file = new QFile(path);
        if(file->open(QFile::WriteOnly)){
            file->write(_reply->readAll());
            file->close();
            qDebug() << "Downloading is completed";
            emit onReady();
        }
    }
}

void UpdateDownloader::error(QNetworkReply::NetworkError err)
{
    qDebug() << "ERROR";
    qDebug() << _reply->errorString();
}

void UpdateDownloader::updateProgress(qint64 read, qint64 total)
{
    _progress->setMaximum(total);
    _progress->setValue(read);
}
