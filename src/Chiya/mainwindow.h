#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include "layoutcontroller.h"
#include "updatedownloader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateChiya();
    static UpdateDownloader *updateDownloader;
private slots:
    void closeEvent(QCloseEvent *event) override;

    void handleLcStateButton();
    void handleLcSettingsButton();

    void handleActionSettingsTriggered();

    void handleActionHelpTriggered();
    void handleActionCheckForUpdates();
private:
    void setupTrayIco();
    void loadSettings();

    Ui::MainWindow *ui;
    QSystemTrayIcon *_sysTrayIcon;

    LayoutController _layoutController;

    bool _closing;

    bool _runnedAsAdmin;

    bool _silentUpdate;
};
#endif // MAINWINDOW_H
