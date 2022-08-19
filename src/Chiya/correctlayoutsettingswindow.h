#ifndef CORRECTLAYOUTSETTINGSWINDOW_H
#define CORRECTLAYOUTSETTINGSWINDOW_H

#include <QDialog>
#include <QItemSelection>
#include <QKeyEvent>
#include <QSettings>

#include "key.h"

namespace Ui {
class CorrectLayoutSettingsWindow;
}

class CorrectLayoutSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CorrectLayoutSettingsWindow(QWidget *parent = nullptr);
    ~CorrectLayoutSettingsWindow();

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

    void handleEApplyButton();
    void handleEWhiteList();
    void handleEExceptionsChanged();

    void handlePApplyButton();
    void handlePCapsLockCheckBox();
    void handlePLayoutCheckBox();
    void handlePLayoutComboBox();
private:
    void setupLayoutsList();
    void loadSettings();

    bool unsavedChangesMessage();

    Ui::CorrectLayoutSettingsWindow *ui;

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

#endif // CORRECTLAYOUTSETTINGSWINDOW_H
