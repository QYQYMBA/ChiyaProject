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
#include "mainsettingswindow.h"
#include "aboutwindow.h"

UpdateDownloader* MainWindow::updateDownloader = new UpdateDownloader();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _layoutController((HWND)MainWindow::winId())
    , _closing(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->lcStateButton, SIGNAL (released()), this, SLOT (handleLcStateButton()));
    connect(ui->lcSettingsButton, SIGNAL (released()), this, SLOT (handleLcSettingsButton()));

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
        _silentUpdate = true;
        handleActionCheckForUpdates();
    }

    settings.beginGroup("LayoutController");

    if(settings.value("runOnStart").toBool())
    {
        if(_layoutController.start())
            ui->lcStateButton->setText("Stop");
    }

    settings.endGroup();

    if(!settings.value("startInTray").toBool() && !settings.value("forceShow").toBool())
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

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(exitAction);

    _sysTrayIcon = new QSystemTrayIcon(this);
    _sysTrayIcon->setContextMenu(trayIconMenu);
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
            ui->lcStateButton->setText("Start");
    }
    else
    {
        if(_layoutController.start())
            ui->lcStateButton->setText("Stop");
    }
}

void MainWindow::handleLcSettingsButton()
{
    _layoutController.stop();
    ui->lcStateButton->setText("Start");

    LayoutControllerSettingsWindow* layoutControllerSettingsWindow = new LayoutControllerSettingsWindow(this);
    layoutControllerSettingsWindow->setModal(true);
    layoutControllerSettingsWindow->show();
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

void MainWindow::handleActionCheckForUpdates()
{
    _silentUpdate = false;
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
             return;

         component = component.nextSibling().toElement();
         if(component.tagName() != "URL")
             return;

         qDebug() << component.firstChild().toText().data();

         updateDownloader->getData(component.firstChild().toText().data());

         //updateChiya();
    });
}

void MainWindow::updateChiya()
{
    QString path = "" + QCoreApplication::applicationDirPath() + "\\Update.bat";
    QFile file(path);
    QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);

        QString delay = "";
        delay += "ping 127.0.0.1 -n 2 > nul";
        stream << delay << Qt::endl;

        QString mkdir = "mkdir Backup";
        stream << mkdir << Qt::endl;

        QString firstCommand = "";
        firstCommand += "move /Y \"";
        firstCommand += QCoreApplication::applicationDirPath() + "\\"+ exeName + "\" ";
        firstCommand += QCoreApplication::applicationDirPath() + "\\Backup\\" + exeName + "\"";
        stream << firstCommand << Qt::endl;

        QString secondCommand = "";
        secondCommand += "ren \"";
        secondCommand += QCoreApplication::applicationDirPath() + "\\" + "New" + exeName + "\" ";
        secondCommand += "\"" + exeName + "\"";
        stream << secondCommand << Qt::endl;

        QString thirdCommand = "start " + exeName;
        if(!_silentUpdate)
            thirdCommand += " --forceShow";
        stream << thirdCommand << Qt::endl;

        QString fourthCommand = "DEL \"%~f0\"";
        stream << fourthCommand << Qt::endl;

        QProcess process;
        process.setProgram( "cmd.exe" );
        process.setArguments( { "/C", path } );
        process.setWorkingDirectory( QCoreApplication::applicationDirPath() );
        process.setStandardOutputFile( QProcess::nullDevice() );
        process.setStandardErrorFile( QProcess::nullDevice() );
        process.startDetached();
    }

    qApp->closeAllWindows();
    exit(0);
}
