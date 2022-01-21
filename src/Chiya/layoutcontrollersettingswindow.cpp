#include "layoutcontrollersettingswindow.h"
#include "ui_layoutcontrollersettingswindow.h"

#include <windows.h>
#include <sstream>

#include <QStringListModel>
#include <QMessageBox>

#include "layoutcontroller.h"
#include "adminrights.h"
#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

LayoutControllerSettingsWindow::LayoutControllerSettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LayoutControllerSettingsWindow)
{
    _settings.beginGroup("LayoutController");

    _shortcutActivateKey = new Key();
    _shortcutActivateKey->vkCode = 0;

    _shortcutSelectKey = new Key();
    _shortcutSelectKey->vkCode = 0;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged()));

    connect(ui->gApplyButtonLs, SIGNAL (clicked()), this, SLOT (handleGApplyButton()));
    connect(ui->gAutoStartCheckBoxLs, SIGNAL (clicked()), this, SLOT (handleGAutoStart()));
    connect(ui->gChangeRegistryCheckBoxLs, SIGNAL(clicked()), this, SLOT(handleGChangeRegistry()));

    connect(ui->lsApplyButtonLs, SIGNAL (clicked()), this, SLOT (handleLsApplyButton()));
    connect(ui->lsShortcutActivateButtonLs, SIGNAL (clicked()), this, SLOT (handleLsShortcutActivateButton()));
    connect(ui->lsShortcutSelectButtonLs, SIGNAL (clicked()), this, SLOT (handleLsShortcutSelectButton()));
    connect(ui->lsActiveCheckBoxLs, SIGNAL (clicked()), this, SLOT (handleLsActivateCheckBox()));

    connect(ui->eApplyButtonLs, SIGNAL (clicked()), this, SLOT (handleEApplyButton()));
    connect(ui->eWhiteListCheckBoxLs, SIGNAL (clicked()), this, SLOT (handleEWhiteList()));
    connect(ui->eExceptionsPlainTextEditLs, SIGNAL (textChanged()), this, SLOT (handleEExceptionsChanged()));

    setupLayoutsList();

    _tab = ui->mainTabs->currentIndex();

    loadSettings();
}

LayoutControllerSettingsWindow::~LayoutControllerSettingsWindow()
{
    _settings.endGroup();
    delete ui;
}

void LayoutControllerSettingsWindow::setupLayoutsList()
{
    _layoutsList = WinApiAdapter::getLayoutsList();
    QStringListModel *model = new QStringListModel(this);
    QStringList list;

    for (int i = 0; i < _layoutsList.size(); i++)
    {
        if(_layoutsList[i] != 0)
        {
            wchar_t name[MAXNAMELENGTH];
            LANGID language = (LANGID)(((UINT)_layoutsList[i]) & 0x0000FFFF);
            LCID locale = MAKELCID(language, SORT_DEFAULT);

            GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);
            list << QString::fromWCharArray(name);
        }
    }

    model->setStringList(list);
    ui->lsLayoutsListViewLs->setModel(model);
    ui->lsLayoutsListViewLs->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lsLayoutsListViewLs->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(handleLsSelectionChanged()));

    ui->lsLayoutsListViewLs->setCurrentIndex(model->index(0, 0));
}

void LayoutControllerSettingsWindow::loadSettings()
{
    ui->mainTabs->setCurrentIndex(0);

    ui->gAutoStartCheckBoxLs->setChecked(_settings.value("runOnStart").toBool());
    if(AdminRights::IsRunAsAdministrator())
        ui->gChangeRegistryCheckBoxLs->setChecked(_settings.value("changeRegistry").toBool());
    else
        ui->gChangeRegistryCheckBoxLs->setEnabled(false);

    ui->eExceptionsPlainTextEditLs->appendPlainText(_settings.value("exceptions/blacklist").toString());
    ui->eWhiteListCheckBoxLs->setChecked(_settings.value("exceptions/isWhiteList").toBool());

    QSettings systemSettings("HKEY_CURRENT_USER\\Keyboard Layout\\Toggle", QSettings::Registry64Format);
    int toggleValue = systemSettings.value("Language Hotkey").toInt();
    if(toggleValue == 1)
        ui->gShiftAltButtonLs->setChecked(true);
    else if (toggleValue == 2)
        ui->gCtrlShiftButtonLs->setChecked(true);
    else
        ui->gNotSetButtonLs->setChecked(true);
    _gChanged = false;
    _lsChanged = false;
    _eChanged = false;
}

void LayoutControllerSettingsWindow::handleTabChanged()
{
    int oldTab = _tab;
    _tab = ui->mainTabs->currentIndex();
    if(oldTab == 0 && _gChanged)
    {
        if(!unsavedChangesMessage() && !(ui->mainTabs->currentIndex() == 0))
        {
            ui->mainTabs->setCurrentIndex(oldTab);
            return;
        }
    }
    else if(oldTab == 1 && _lsChanged)
    {
        if(!unsavedChangesMessage() && !(ui->mainTabs->currentIndex() == 1))
        {
            ui->mainTabs->setCurrentIndex(oldTab);
            return;
        }
    }
    else if(oldTab == 2 && _eChanged)
    {
        if(!unsavedChangesMessage() && !(ui->mainTabs->currentIndex() == 2))
        {
            ui->mainTabs->setCurrentIndex(oldTab);
            return;
        }
    }
}

void LayoutControllerSettingsWindow::handleLsShortcutActivateButton()
{
    _shortcutSelect = false;
    _shortcutActivate = true;
}

void LayoutControllerSettingsWindow::handleLsShortcutSelectButton()
{
    _shortcutSelect = true;
    _shortcutActivate = false;
}

void LayoutControllerSettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if(_shortcutActivate)
    {
        if(event->key() == Qt::Key_Control || event->key() ==Qt::Key_Shift || event->key() == Qt::Key_Alt)
            return;
        if(event->key() == Qt::Key_Escape)
        {
            _shortcutActivateKey = new Key();
            _shortcutActivateKey->vkCode = 0;
            _shortcutActivate = false;
            ui->lsShortcutActivateLineEditLs->setText("");
            return;
        }
        Key* key = new Key();
        key->vkCode = event->nativeVirtualKey();
        key->scanCode = event->key();
        key->ctrl = GetAsyncKeyState(VK_CONTROL);
        key->shift = GetAsyncKeyState(VK_SHIFT);
        key->alt = GetAsyncKeyState(VK_MENU);

        QString shortcut = "";
        if(key->ctrl)
            shortcut += "Ctrl +";
        if(key->shift)
            shortcut += "Shift +";
        if(key->alt)
            shortcut += "Alt +";
        std::string s = "";
        s += MapVirtualKey(key->vkCode, MAPVK_VK_TO_CHAR);
        shortcut += QString::fromStdString(s);

        _shortcutActivateKey = key;

        ui->lsShortcutActivateLineEditLs->setText(shortcut);

        _shortcutActivate = false;

        _lsChanged = true;
    }

    if(_shortcutSelect)
    {
        if(event->key() == Qt::Key_Control || event->key() ==Qt::Key_Shift || event->key() == Qt::Key_Alt)
            return;
        if(event->key() == Qt::Key_Escape)
        {
            _shortcutSelectKey = new Key();
            _shortcutSelectKey->vkCode = 0;
            _shortcutSelect = false;
            ui->lsShortcutSelectLineEditLs->setText("");
            return;
        }
        Key* key = new Key();
        key->vkCode = event->nativeVirtualKey();
        key->scanCode = event->key();
        key->ctrl = GetAsyncKeyState(VK_CONTROL);
        key->shift = GetAsyncKeyState(VK_SHIFT);
        key->alt = GetAsyncKeyState(VK_MENU);

        QString shortcut = "";
        if(key->ctrl)
            shortcut += "Ctrl +";
        if(key->shift)
            shortcut += "Shift +";
        if(key->alt)
            shortcut += "Alt +";
        std::string s = "";
        s += MapVirtualKey(key->vkCode, MAPVK_VK_TO_CHAR);
        shortcut += QString::fromStdString(s);

        _shortcutSelectKey = key;

        ui->lsShortcutSelectLineEditLs->setText(shortcut);

        _shortcutSelect = false;

        _lsChanged = true;
    }
}

void LayoutControllerSettingsWindow::handleLsActivateCheckBox()
{
    _lsChanged = true;
}

void LayoutControllerSettingsWindow::handleLsApplyButton()
{
    if(!_lsChanged)
        return;

    _lsChanged = false;

    QModelIndex index = ui->lsLayoutsListViewLs->currentIndex();
    QString itemText;
    itemText = itemText.fromStdString(WinApiAdapter::hklToStr(_layoutsList[_index.row()]));

    _settings.setValue("runOnStart", ui->gAutoStartCheckBoxLs->isChecked());

    _settings.setValue("layouts/" + itemText + "/deactivated", ui->lsActiveCheckBoxLs->isChecked());

    _settings.setValue("layouts/" + itemText + "/shortcut/activate/active", _shortcutActivateKey->vkCode != 0);
    if(_shortcutActivateKey->vkCode != 0)
    {
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/ctrl", _shortcutActivateKey->ctrl);
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/shift", _shortcutActivateKey->shift);
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/alt", _shortcutActivateKey->alt);
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/qtCode", _shortcutActivateKey->scanCode);
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/vkCode", _shortcutActivateKey->vkCode);
    }

    _settings.setValue("layouts/" + itemText + "/shortcut/select/active", _shortcutSelectKey->vkCode != 0);
    if(_shortcutSelectKey->vkCode != 0)
    {
        _settings.setValue("layouts/" + itemText + "/shortcut/select/ctrl", _shortcutSelectKey->ctrl);
        _settings.setValue("layouts/" + itemText + "/shortcut/select/shift", _shortcutSelectKey->shift);
        _settings.setValue("layouts/" + itemText + "/shortcut/select/alt", _shortcutSelectKey->alt);
        _settings.setValue("layouts/" + itemText + "/shortcut/select/qtCode", _shortcutSelectKey->scanCode);
        _settings.setValue("layouts/" + itemText + "/shortcut/select/vkCode", _shortcutSelectKey->vkCode);
    }
}

void LayoutControllerSettingsWindow::handleLsSelectionChanged(){
    if(!_lsIndexChanged)
    {
        _lsIndexChanged = true;
        return;
    }

    if(_lsChanged)
    {   
        if (!!unsavedChangesMessage()) {
            _lsIndexChanged = false;
            ui->lsLayoutsListViewLs->setCurrentIndex(_index);
            return;
        }
    }

    _index = ui->lsLayoutsListViewLs->currentIndex();
    QString itemText;
    itemText = itemText.fromStdString(WinApiAdapter::hklToStr(_layoutsList[_index.row()]));

    ui->lsActiveCheckBoxLs->setChecked(_settings.value("layouts/" + itemText + "/deactivated").toBool());

    WinApiAdapter::SetKeyboardLayout(_layoutsList[_index.row()]);

    if(_settings.value("layouts/" + itemText + "/shortcut/activate/active").toBool())
    {
        QString shortcut = "";
        if(_settings.value("layouts/" + itemText + "/shortcut/activate/ctrl").toBool())
           shortcut += "Ctrl +";
        if(_settings.value("layouts/" + itemText + "/shortcut/activate/shift").toBool())
           shortcut += "Shift +";
        if(_settings.value("layouts/" + itemText + "/shortcut/activate/alt").toBool())
            shortcut += "Alt +";
        std::string s = "";
        s += MapVirtualKey(_settings.value("layouts/" + itemText + "/shortcut/activate/vkCode").toInt(), MAPVK_VK_TO_CHAR);
        shortcut += QString::fromStdString(s);

        ui->lsShortcutActivateLineEditLs->setText(shortcut);
   }
   else
   {
       ui->lsShortcutActivateLineEditLs->setText("");
   }

    if(_settings.value("layouts/" + itemText + "/shortcut/select/active").toBool())
    {
        QString shortcut = "";
        if(_settings.value("layouts/" + itemText + "/shortcut/select/ctrl").toBool())
           shortcut += "Ctrl +";
        if(_settings.value("layouts/" + itemText + "/shortcut/select/shift").toBool())
           shortcut += "Shift +";
        if(_settings.value("layouts/" + itemText + "/shortcut/select/alt").toBool())
            shortcut += "Alt +";
        std::string s = "";
        s += MapVirtualKey(_settings.value("layouts/" + itemText + "/shortcut/select/vkCode").toInt(), MAPVK_VK_TO_CHAR);
        shortcut += QString::fromStdString(s);

        ui->lsShortcutSelectLineEditLs->setText(shortcut);
   }
   else
   {
       ui->lsShortcutSelectLineEditLs->setText("");
   }

    _lsChanged = false;
}

void LayoutControllerSettingsWindow::closeEvent(QCloseEvent *event) {
    if(_lsChanged)
    {
        if (!unsavedChangesMessage()) {
            _lsIndexChanged = false;
            ui->lsLayoutsListViewLs->setCurrentIndex(_index);
            event->ignore();
            return;
        }

        _lsChanged = false;
        event->accept();
    }
}

void LayoutControllerSettingsWindow::handleGApplyButton()
{
    _settings.setValue("runOnStart", ui->gAutoStartCheckBoxLs->isChecked());

    _settings.setValue("changeRegistry", ui->gChangeRegistryCheckBoxLs->isChecked());

    QString toggleValue;
    if(ui->gShiftAltButtonLs->isChecked())
        toggleValue = "1";
    else if(ui->gCtrlShiftButtonLs->isChecked())
        toggleValue = "2";
    else
        toggleValue = "3";

    QSettings systemSettings("HKEY_CURRENT_USER\\Keyboard Layout\\Toggle", QSettings::Registry64Format);
    systemSettings.setValue("Language Hotkey", toggleValue);

    _gChanged = false;
}

void LayoutControllerSettingsWindow::handleGAutoStart()
{
    _gChanged = true;
}

void LayoutControllerSettingsWindow::handleGChangeRegistry()
{
    _gChanged = true;
}

bool LayoutControllerSettingsWindow::unsavedChangesMessage()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Unsaved changes!", "You have unsaved changes! Are you sure you want to continue?",
                                    QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        _gChanged = false;
        _lsChanged = false;
        _eChanged = false;
    }
    return (reply == QMessageBox::Yes);
}

void LayoutControllerSettingsWindow::handleEApplyButton()
{
    _settings.setValue("exceptions/blacklist", ui->eExceptionsPlainTextEditLs->toPlainText());
    _settings.setValue("exceptions/isWhiteList", ui->eWhiteListCheckBoxLs->isChecked());

    _eChanged = false;
}

void LayoutControllerSettingsWindow::handleEWhiteList()
{
    _eChanged = true;
}

void LayoutControllerSettingsWindow::handleEExceptionsChanged()
{
    _eChanged = true;
}
