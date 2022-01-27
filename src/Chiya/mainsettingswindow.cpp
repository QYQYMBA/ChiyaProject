#include "mainsettingswindow.h"
#include "ui_mainsettingswindow.h"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

MainSettingsWindow::MainSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainSettingsWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint );

    connect(ui->cancelButton, SIGNAL (clicked()), this, SLOT (handleCancelButton()));
    connect(ui->applyButton, SIGNAL (clicked()), this, SLOT (handleApplyButton()));

    connect(ui->asAdminCheckBox, SIGNAL (clicked()), this, SLOT (handleAsAdminCheckBox()));

    loadSettings();
}

MainSettingsWindow::~MainSettingsWindow()
{
    delete ui;
}

void MainSettingsWindow::loadSettings()
{
    QSettings settings;

    ui->runOnStartupCheckBox->setChecked(settings.value("runOnStartup").toBool());

    ui->asAdminCheckBox->setChecked(settings.value("runAsAdmin").toBool());

    ui->autoUpdateCheckBox->setChecked(settings.value("autoUpdate").toBool());

    ui->startInTrayCheckBox->setChecked(settings.value("startInTray").toBool());
}

void MainSettingsWindow::handleCancelButton()
{
    close();
}

void MainSettingsWindow::handleApplyButton()
{
    QSettings settings;

    settings.setValue("startInTray", ui->startInTrayCheckBox->isChecked());
    settings.setValue("runAsAdmin", ui->asAdminCheckBox->isChecked());
    settings.setValue("autoUpdate", ui->autoUpdateCheckBox->isChecked());

    if((settings.value("runOnStartup").toBool() != ui->runOnStartupCheckBox->isChecked()))
    {
        if(ui->runOnStartupCheckBox->isChecked())
        {
            normalStartup();
        }
        else
        {
            clearNormalStartup();
        }
    }

    close();
}

void MainSettingsWindow::handleAsAdminCheckBox()
{
    if(ui->asAdminCheckBox->isChecked())
    {
        ui->autoUpdateCheckBox->setEnabled(true);
    }
    else
    {
        ui->autoUpdateCheckBox->setChecked(false);
        ui->autoUpdateCheckBox->setEnabled(false);
    }
}

void MainSettingsWindow::normalStartup()
{
    QSettings startupSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    QString value = QCoreApplication::applicationFilePath();
    QString apostroph = "\"";
    value.replace("/","\\");
    value = apostroph + value + apostroph + " --autostart";

    startupSettings.setValue(QCoreApplication::applicationName(), value);

    QSettings settings;
    settings.setValue("runOnStartup", true);
}

void MainSettingsWindow::clearNormalStartup()
{
    QSettings startupSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    startupSettings.remove(QCoreApplication::applicationName());

    QSettings settings;
    settings.setValue("runOnStartup", false);
}
