#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QProcess>
#include <QtNetwork>
#include <QFuture>
#include <QtXml>

#include "windows.h"
#include "adminrights.h"
#include "layoutcontrollersettingswindow.h"
#include "mainsettingswindow.h"
#include "aboutwindow.h"

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

    settings.beginGroup("LayoutController");

    if(settings.value("runOnStart").toBool())
    {
        if(_layoutController.start())
        {
            ui->lcStateButton->setText("Stop");
        }
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
            ui->lcStateButton->setText("Start");
        }
    }
    else
    {
        if(_layoutController.start())
        {
            ui->lcStateButton->setText("Stop");
        }
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
