#include "aboutwindow.h"
#include "ui_aboutwindow.h"

#include <QCoreApplication>

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowCloseButtonHint );
    ui->versionLabel->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
