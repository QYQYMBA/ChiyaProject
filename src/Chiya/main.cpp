#include "mainwindow.h"

#include <string>

#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QMessageBox>

#include "runguard.h"

int main(int argc, char *argv[])
{
    RunGuard guard("7TPn9D9Km2FVMjSTSQ8x");
    if ( !guard.tryToRun() )
    {
        Sleep(1500);
        if ( !guard.tryToRun() )
        {
            return 0;
        }
    }

    QCoreApplication::setOrganizationName("TeTo");
    QCoreApplication::setApplicationName("Chiya");

#ifdef QT_DEBUG
    QCoreApplication::setApplicationName("Chiya_Debug");
#endif

    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication a(argc, argv);

    QSettings settings;

    for(int i = 0; i < argc; i++)
    {
        std::string arg(argv[i]);
        if(arg == "--update" && !settings.value("updateAttempt").toBool())
        {
            settings.setValue("updateAttempt", true);
            MainWindow::updateChiya();
        }
    }

    settings.setValue("updateAttempt", false);

    MainWindow w;
    return a.exec();
}
