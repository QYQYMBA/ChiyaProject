#ifndef MAINSETTINGSWINDOW_H
#define MAINSETTINGSWINDOW_H

#include <QDialog>

namespace Ui {
class MainSettingsWindow;
}

class MainSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettingsWindow(QWidget *parent = nullptr);
    ~MainSettingsWindow();

private slots:
    void handleCancelButton();
    void handleApplyButton();

private:
    void loadSettings();

    void normalStartup();
    void clearNormalStartup();

    Ui::MainSettingsWindow *ui;
};

#endif // MAINSETTINGSWINDOW_H
