#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include "layoutcontroller.h"
#include "correctlayout.h"

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

    void handleClStateButton();
    void handleClSettingsButton();

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
    CorrectLayout correctLayout;

    bool _closing;

    bool _runnedAsAdmin;
};
#endif // MAINWINDOW_H
