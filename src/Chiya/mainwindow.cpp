#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QProcess>
#include <QtNetwork>
#include <QtXml>

#include "windows.h"
#include "adminrights.h"
#include "layoutcontrollersettingswindow.h"
#include "correctlayoutsettingswindow.h"
#include "mainsettingswindow.h"
#include "aboutwindow.h"

UpdateDownloader* MainWindow::updateDownloader;
bool MainWindow::_silentUpdate = false;
bool MainWindow::_showNoUpdate = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _layoutController((HWND)MainWindow::winId())
    , _correctLayout((HWND)MainWindow::winId(), &_layoutController)
    , _closing(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    updateDownloader = new UpdateDownloader();

    connect(ui->lcStateButton, SIGNAL (released()), this, SLOT (handleLcStateButton()));
    connect(ui->lcSettingsButton, SIGNAL (released()), this, SLOT (handleLcSettingsButton()));

    connect(ui->clStateButton, SIGNAL (released()), this, SLOT (handleClStateButton()));
    connect(ui->clSettingsButton, SIGNAL (released()), this, SLOT (handleClSettingsButton()));

    connect(ui->actionMainSettings, SIGNAL (triggered()), this, SLOT (handleActionSettingsTriggered()));
    connect(ui->actionAbout, SIGNAL (triggered()), this, SLOT (handleActionHelpTriggered()));
    connect(ui->actionCheckForUpdates, SIGNAL (triggered()), this, SLOT (handleActionCheckForUpdates()));

    connect(updateDownloader, &UpdateDownloader::onReady, this, &MainWindow::updateChiya);

    loadSettings();
    setupTrayIco();

    QtGlobalInput::init((HWND)MainWindow::winId());
}

MainWindow::~MainWindow()
{
    _layoutController.stop();

    delete _sysTrayIcon;
    delete _trayIconMenu;
    delete ui;
}

void MainWindow::loadSettings()
{
    QSettings settings;

    _runnedAsAdmin = AdminRights::IsRunAsAdministrator();

    if(settings.value("runAsAdmin").toBool())
        if(!_runnedAsAdmin)
        {
#ifndef QT_DEBUG
            AdminRights::ElevateNow(L"rasadmin");
#endif
        }

    if(settings.value("autoUpdate").toBool())
    {
        _showNoUpdate = true;
        if(!settings.value("startInTray").toBool())
        {
            _silentUpdate = false;
            checkUpdate();
        }
        else
        {
            _silentUpdate = true;
            checkUpdate();
        }
    }

    settings.beginGroup("LayoutController");

    if(settings.value("runOnStart").toBool())
    {
        if(_layoutController.start())
        {
            ui->lcStateButton->setText("Stop");
            ui->clStateButton->setEnabled(true);
        }
    }

    settings.endGroup();

    settings.beginGroup("CorrectLayout");

    if(settings.value("runOnStart").toBool())
    {
        if(_correctLayout.startCl())
            ui->clStateButton->setText("Stop");
    }

    settings.endGroup();

    if(!settings.value("startInTray").toBool() || settings.value("forceShow").toBool())
    {
        this->show();
    }

    settings.setValue("forceShow", false);
}

void MainWindow::setupTrayIco()
{
    QAction* exitAction = new QAction(tr("&Exit"), this);
    connect(exitAction, &QAction::triggered, [this]()
    {
        _closing = true;
        close();
    });

    _trayIconMenu = new QMenu(this);
    _trayIconMenu->addAction(exitAction);

    _sysTrayIcon = new QSystemTrayIcon(this);
    _sysTrayIcon->setContextMenu(_trayIconMenu);
    _sysTrayIcon->setIcon(QIcon(":/icons/NormalIco.ico"));
    _sysTrayIcon->show();

    connect(_sysTrayIcon, &QSystemTrayIcon::activated, [this](auto reason)
    {
        if(reason == QSystemTrayIcon::Trigger)
        {
            if(isVisible())
            {
                hide();
            }
            else
            {
                show();
                activateWindow();
            }
        }
    });
}

void MainWindow::handleLcStateButton()
{
    if(_layoutController.isRunning())
    {
        if(_layoutController.stop())
        {
            _correctLayout.stopCl();
            ui->lcStateButton->setText("Start");
            ui->clStateButton->setEnabled(false);
            ui->clStateButton->setText("Start");
        }
    }
    else
    {
        if(_layoutController.start())
        {
            ui->lcStateButton->setText("Stop");
            ui->clStateButton->setEnabled(true);
        }
    }
}

void MainWindow::handleLcSettingsButton()
{
    _layoutController.stop();
    _correctLayout.stopCl();
    ui->clStateButton->setEnabled(false);
    ui->clStateButton->setText("Start");
    ui->lcStateButton->setText("Start");

    LayoutControllerSettingsWindow* layoutControllerSettingsWindow = new LayoutControllerSettingsWindow(this);
    layoutControllerSettingsWindow->setModal(true);
    layoutControllerSettingsWindow->show();
}

void MainWindow::handleClStateButton()
{
    if(_correctLayout.isRunning())
    {
        if(_correctLayout.stopCl())
            ui->clStateButton->setText("Start");
    }
    else
    {
        if(_correctLayout.startCl())
            ui->clStateButton->setText("Stop");
    }
}

void MainWindow::handleClSettingsButton()
{
    _correctLayout.stopCl();
    ui->clStateButton->setText("Start");

    CorrectLayoutSettingsWindow* correctLayoutSettingsWindow = new CorrectLayoutSettingsWindow(this);
    correctLayoutSettingsWindow->setModal(true);
    correctLayoutSettingsWindow->show();
}

void MainWindow::handleActionSettingsTriggered()
{
    MainSettingsWindow* mainSettingsWindows = new MainSettingsWindow(this);
    mainSettingsWindows->setModal(true);
    mainSettingsWindows->show();
}

void MainWindow::handleActionHelpTriggered()
{
    AboutWindow* aboutWindow = new AboutWindow(this);
    aboutWindow->setModal(true);
    aboutWindow->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(_closing)
    {
        _sysTrayIcon->hide();
        qApp->closeAllWindows();
        event->accept();
    }
    else
    {
        this->hide();
        event->ignore();
    }
}

void htmlGet(const QUrl &url, const std::function<void(const QString&)> &fun) {
   QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
   QNetworkReply *response = manager->get(QNetworkRequest(QUrl(url)));
   QObject::connect(response, &QNetworkReply::finished, [response, fun]{
      response->deleteLater();
      response->manager()->deleteLater();
      if (response->error() != QNetworkReply::NoError) return;
      auto const contentType =
            response->header(QNetworkRequest::ContentTypeHeader).toString();
      if (contentType != "application/xml") {
         qWarning() << "Wrong file type!" << contentType;
         return;
      }
      auto const html = QString::fromUtf8(response->readAll());
      fun(html);
   }) && manager.take();
}

void MainWindow::checkUpdate()
{
    htmlGet({"http://repository.chiyaproject.com/Updates.xml"}, [](const QString &body){
        QDomDocument xmlUpdates;
        xmlUpdates.setContent(body);
        QDomElement root = xmlUpdates.documentElement();
        if(root.tagName() != "Updates")
            return;
        QDomElement component  =root.firstChild().toElement();
        if(component.tagName() != "Version")
            return;
         QVersionNumber newVersion = QVersionNumber::fromString(component.firstChild().toText().data());
         QVersionNumber currentVersion = QVersionNumber::fromString(QApplication::applicationVersion());
         if(currentVersion >= newVersion)
         {
             if(!MainWindow::isShowNoUpdate())
             {
                 QMessageBox::question(0, "No updates available!", "There is no available updates!",
                                       QMessageBox::Ok);
             }
             return;
         }

         component = component.nextSibling().toElement();
         if(component.tagName() != "URL")
             return;

         QString url = component.firstChild().toText().data();
         qDebug() << url;

         if(!MainWindow::isUpdateSilent())
         {
             QMessageBox::StandardButton reply;
             reply = QMessageBox::question(0, "Updates available!", "Do you want to perform update? Chiya will be restarted!",
                                           QMessageBox::Yes|QMessageBox::No);
             if(reply == QMessageBox::No)
                 return;
         }

         MainWindow::updateDownloader->getData(url, !MainWindow::isUpdateSilent());
    });
}

void MainWindow::handleActionCheckForUpdates()
{
    _showNoUpdate = false;
    _silentUpdate = false;
    checkUpdate();
}

void MainWindow::updateChiya()
{
    if(updateDownloader->isRunning())
        return;

    QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    QString oldPath = "" + QCoreApplication::applicationDirPath() + "\\" + exeName;
    QString newPath = "" + QCoreApplication::applicationDirPath() + "\\" + "Old" + exeName;
    QFile::rename(oldPath, newPath);

    oldPath = "" + QCoreApplication::applicationDirPath() + "\\" + "New" +exeName;
    newPath = "" + QCoreApplication::applicationDirPath() + "\\" + exeName;
    QFile::rename(oldPath, newPath);

    if(!_silentUpdate)
        QProcess::startDetached(exeName, QStringList() << "-uf");
    else
        QProcess::startDetached(exeName, QStringList() << "-u");

    qApp->closeAllWindows();
    exit(0);
}

bool MainWindow::isUpdateSilent()
{
    return _silentUpdate;
}

bool MainWindow::isShowNoUpdate()
{
    return _showNoUpdate;
}

