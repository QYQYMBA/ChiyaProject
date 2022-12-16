#ifndef LAYOUTCONTROLLERSETTINGSWINDOW_H
#define LAYOUTCONTROLLERSETTINGSWINDOW_H

#include <QDialog>
#include <QItemSelection>
#include <QKeyEvent>
#include <QSettings>

#include "key.h"

namespace Ui {
class LayoutControllerSettingsWindow;
}

class LayoutControllerSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LayoutControllerSettingsWindow(QWidget *parent = nullptr);
    ~LayoutControllerSettingsWindow();

private slots:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    void handleTabChanged();

    void handleLsApplyButton();
    void handleLsShortcutActivateButton();
    void handleLsShortcutSelectButton();
    void handleLsActivateCheckBox();
    void handleLsSelectionChanged();

    void handleGApplyButton();
    void handleGAutoStart();
    void handleGChangeRegistry();
    void handleGEnableQt();

    void handleEApplyButton();
    void handleEWhiteList();
    void handleEExceptionsChanged();
private:
    void setupLayoutsList();
    void loadSettings();

    bool unsavedChangesMessage();

    Ui::LayoutControllerSettingsWindow *ui;

    QSettings _settings;

    bool _shortcutActivate = false;
    Key* _shortcutActivateKey;

    bool _shortcutSelect = false;
    Key* _shortcutSelectKey;

    std::vector<HKL> _layoutsList;
    QModelIndex _index;
    int _tab;

    bool _gChanged = false;

    bool _lsIndexChanged = true;
    bool _lsChanged = false;

    bool _eChanged = false;
};

#endif // LAYOUTCONTROLLERSETTINGSWINDOW_H
