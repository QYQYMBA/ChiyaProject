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
    void handleLsAutoCheckBox();
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

    void handleSApplyButton();
    void handleSPauseButton();
    void handleSNextButton();
    void handleSUndoButton();
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

    bool _shortcutPause = false;
    Key* _shortcutPauseKey;

    bool _shortcutNext = false;
    Key* _shortcutNextKey;

    bool _shortcutUndo = false;
    Key* _shortcutUndoKey;

    std::vector<HKL> _layoutsList;
    QModelIndex _index;
    int _tab;

    bool _gChanged = false;

    bool _lsIndexChanged = true;
    bool _lsChanged = false;

    bool _eChanged = false;
    bool _pChanged = false;
    bool _sChanged = false;
};

#endif // CORRECTLAYOUTSETTINGSWINDOW_H
