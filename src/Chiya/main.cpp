#include "mainwindow.h"

#include <string>

#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QFileInfo>
#include <QCommandLineOption>
#include <QCommandLineParser>

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

    QCommandLineOption autostart("a");
    QCommandLineOption updated("u");
    QCommandLineOption forceShow("f");

    QCommandLineParser parser;
    parser.addOption(updated);
    parser.addOption(forceShow);
    parser.addOption(autostart);
    parser.process(a);

    if(parser.isSet(forceShow) && !settings.value("forceShow").toBool())
    {
        settings.setValue("forceShow", true);
    }
    if(parser.isSet(updated))
    {
        QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        QFile::remove(QCoreApplication::applicationDirPath() + "\\" + "Old" + exeName);
    }

    MainWindow w;
    return a.exec();
}
