#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include "layoutcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void updateChiya();
private slots:
    void closeEvent(QCloseEvent *event) override;

    void handleLcStateButton();
    void handleLcSettingsButton();

    void handleActionSettingsTriggered();

    void handleActionHelpTriggered();
    void handleActionCheckForUpdates();
private:
    bool checkUpdates(bool silent);
    void setupTrayIco();
    void loadSettings();

    Ui::MainWindow *ui;
    QSystemTrayIcon *_sysTrayIcon;

    LayoutController layoutController;

    bool _closing;

    bool _runnedAsAdmin;
};
#endif // MAINWINDOW_H
