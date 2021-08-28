#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QProcess>

#include "windows.h"
#include "adminrights.h"
#include "layoutcontrollersettingswindow.h"
#include "mainsettingswindow.h"
#include "aboutwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , layoutController((HWND)MainWindow::winId())
    , _closing(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->lcStateButton, SIGNAL (released()), this, SLOT (handleLcStateButton()));
    connect(ui->lcSettingsButton, SIGNAL (released()), this, SLOT (handleLcSettingsButton()));
    connect(ui->actionMainSettings, SIGNAL (triggered()), this, SLOT (handleActionSettingsTriggered()));
    connect(ui->actionAbout, SIGNAL (triggered()), this, SLOT (handleActionHelpTriggered()));
    connect(ui->actionCheckForUpdates, SIGNAL (triggered()), this, SLOT (handleActionCheckForUpdates()));
    loadSettings();
    setupTrayIco();
}

MainWindow::~MainWindow()
{
    layoutController.stop();

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

    if(_runnedAsAdmin && settings.value("autoUpdate").toBool())
    {
        if(checkUpdates(true))
        {
            updateChiya();
        }
    }

    settings.beginGroup("LayoutController");

    if(settings.value("runOnStart").toBool())
    {
        if(layoutController.start())
            ui->lcStateButton->setText("Stop");
    }

    settings.endGroup();

    if(!settings.value("startInTray").toBool())
    {
        this->show();
    }
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
    if(layoutController.isRunning())
    {
        if(layoutController.stop())
            ui->lcStateButton->setText("Start");
    }
    else
    {
        if(layoutController.start())
            ui->lcStateButton->setText("Stop");
    }
}

void MainWindow::handleLcSettingsButton()
{
    layoutController.stop();
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

bool MainWindow::checkUpdates(bool silent)
{
    QProcess process;
    QString path = "" + QCoreApplication::applicationDirPath() + "\\maintenancetool.exe";
    path.replace("/","\\");
    process.start(path, QStringList() << "ch");

    process.waitForFinished();

    if(process.error() != QProcess::UnknownError)
    {
        if(!silent)
        {
            QMessageBox info;
            info.setText("Can't open maintenancetool. Have you installed Chiya using installer?");
            info.exec();
        }
        return false;
    }

    QByteArray data = process.readAllStandardOutput();

    if(data.contains("no updates available"))
    {
        if(!silent)
        {
            QMessageBox info;
            info.setText("No updates available.");
            info.exec();
        }
        return false;
    }

    return true;
}

void MainWindow::handleActionCheckForUpdates()
{
    if(!checkUpdates(false))
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Updates available!", "Do you want to perform update? Chiya will be restarted!",
                                    QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::No)
        return;

    if(!_runnedAsAdmin)
    {
        AdminRights::ElevateNow(L"--update");
    }
    else
    {
        updateChiya();
    }
}

void MainWindow::updateChiya()
{
    QString path = "" + QCoreApplication::applicationDirPath() + "\\maintenancetool.exe";
    path.replace("/","\\");

    QProcess::startDetached(path, QStringList() << "up" << "--confirm-command");

    qApp->closeAllWindows();
    exit(0);
}
