#include "correctlayoutsettingswindow.h"
#include "ui_correctlayoutsettingswindow.h"

#include <windows.h>
#include <sstream>

#include <QStringListModel>
#include <QMessageBox>

#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

CorrectLayoutSettingsWindow::CorrectLayoutSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorrectLayoutSettingsWindow)
{
    _settings.beginGroup("CorrectLayout");

    _shortcutActivateKey = new Key();
    _shortcutActivateKey->vkCode = 0;

    _shortcutSelectKey = new Key();
    _shortcutSelectKey->vkCode = 0;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged()));

    connect(ui->gApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleGApplyButton()));
    connect(ui->gAutoStartCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleGAutoStart()));

    connect(ui->lsApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleLsApplyButton()));
    connect(ui->lsShortcutActivateButtonCl, SIGNAL (clicked()), this, SLOT (handleLsShortcutActivateButton()));
    connect(ui->lsShortcutSelectButtonCl, SIGNAL (clicked()), this, SLOT (handleLsShortcutSelectButton()));
    connect(ui->lsActiveCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleLsActivateCheckBox()));

    connect(ui->eApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleEApplyButton()));
    connect(ui->eWhiteListCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleEWhiteList()));
    connect(ui->eExceptionsPlainTextEditCl, SIGNAL (textChanged()), this, SLOT (handleEExceptionsChanged()));

    connect(ui->pApplyButtonCl, SIGNAL (clicked()), this, SLOT (handlePApplyButton()));
    connect(ui->pCapsLockCheckBoxCl, SIGNAL (clicked()), this, SLOT (handlePCapsLockCheckBox()));
    connect(ui->pLayoutCheckBoxCl, SIGNAL (clicked()), this, SLOT (handlePLayoutCheckBox()));
    connect(ui->pLayoutComboBoxCl, SIGNAL (currentIndexChanged()), this, SLOT (handlePLayoutComboBox()));

    setupLayoutsList();

    _tab = ui->mainTabs->currentIndex();

    loadSettings();
}

CorrectLayoutSettingsWindow::~CorrectLayoutSettingsWindow()
{
    _settings.endGroup();
    delete ui;
}

void CorrectLayoutSettingsWindow::setupLayoutsList()
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
    ui->lsLayoutsListViewCl->setModel(model);
    ui->pLayoutComboBoxCl->setModel(model);

    ui->lsLayoutsListViewCl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lsLayoutsListViewCl->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(handleLsSelectionChanged()));

    ui->lsLayoutsListViewCl->setCurrentIndex(model->index(0, 0));

    QString layout = _settings.value("passwords/layout").toString();

    for (int i = 0; i < _layoutsList.size(); i++)
    {
        if(WinApiAdapter::hklToStr(_layoutsList[i]) == layout)
        {
            ui->pLayoutComboBoxCl->setCurrentIndex(i);
        }
    }
}

void CorrectLayoutSettingsWindow::loadSettings()
{
    ui->mainTabs->setCurrentIndex(0);

    ui->gAutoStartCheckBoxCl->setChecked(_settings.value("runOnStart").toBool());

    ui->eExceptionsPlainTextEditCl->appendPlainText(_settings.value("exceptions/blacklist").toString());
    ui->eWhiteListCheckBoxCl->setChecked(_settings.value("exceptions/isWhiteList").toBool());

    ui->pCapsLockCheckBoxCl->setChecked(_settings.value("passwords/caplslock").toBool());
    bool layoutChecked = _settings.value("passwords/layoutChecked").toBool();
    ui->pLayoutCheckBoxCl->setChecked(layoutChecked);
    ui->pLayoutComboBoxCl->setEnabled(layoutChecked);

    _gChanged = false;
    _lsChanged = false;
    _eChanged = false;
}

void CorrectLayoutSettingsWindow::handleTabChanged()
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

void CorrectLayoutSettingsWindow::handleLsShortcutActivateButton()
{
    _shortcutSelect = false;
    _shortcutActivate = true;
}

void CorrectLayoutSettingsWindow::handleLsShortcutSelectButton()
{
    _shortcutSelect = true;
    _shortcutActivate = false;
}

void CorrectLayoutSettingsWindow::keyPressEvent(QKeyEvent *event)
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
            ui->lsShortcutActivateLineEditCl->setText("");
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

        ui->lsShortcutActivateLineEditCl->setText(shortcut);

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
            ui->lsShortcutSelectLineEditCl->setText("");
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

        ui->lsShortcutSelectLineEditCl->setText(shortcut);

        _shortcutSelect = false;

        _lsChanged = true;
    }
}

void CorrectLayoutSettingsWindow::handleLsActivateCheckBox()
{
    _lsChanged = true;
}

void CorrectLayoutSettingsWindow::handleLsApplyButton()
{
    if(!_lsChanged)
        return;

    _lsChanged = false;

    QModelIndex index = ui->lsLayoutsListViewCl->currentIndex();
    QString itemText;
    itemText = WinApiAdapter::hklToStr(_layoutsList[_index.row()]);

    _settings.setValue("runOnStart", ui->gAutoStartCheckBoxCl->isChecked());

    _settings.setValue("layouts/" + itemText + "/deactivated", ui->lsActiveCheckBoxCl->isChecked());

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

void CorrectLayoutSettingsWindow::handleLsSelectionChanged(){
    if(!_lsIndexChanged)
    {
        _lsIndexChanged = true;
        return;
    }

    if(_lsChanged)
    {
        if (!!unsavedChangesMessage()) {
            _lsIndexChanged = false;
            ui->lsLayoutsListViewCl->setCurrentIndex(_index);
            return;
        }
    }

    _index = ui->lsLayoutsListViewCl->currentIndex();
    QString itemText;
    itemText = WinApiAdapter::hklToStr(_layoutsList[_index.row()]);

    ui->lsActiveCheckBoxCl->setChecked(_settings.value("layouts/" + itemText + "/deactivated").toBool());

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

        ui->lsShortcutActivateLineEditCl->setText(shortcut);
   }
   else
   {
       ui->lsShortcutActivateLineEditCl->setText("");
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

        ui->lsShortcutSelectLineEditCl->setText(shortcut);
   }
   else
   {
       ui->lsShortcutSelectLineEditCl->setText("");
   }

    _lsChanged = false;
}

void CorrectLayoutSettingsWindow::closeEvent(QCloseEvent *event) {
    if(_lsChanged)
    {
        if (!unsavedChangesMessage()) {
            _lsIndexChanged = false;
            ui->lsLayoutsListViewCl->setCurrentIndex(_index);
            event->ignore();
            return;
        }

        _lsChanged = false;
        event->accept();
    }
}

void CorrectLayoutSettingsWindow::handleGApplyButton()
{
    _settings.setValue("runOnStart", ui->gAutoStartCheckBoxCl->isChecked());

    _gChanged = false;
}

void CorrectLayoutSettingsWindow::handleGAutoStart()
{
    _gChanged = true;
}

bool CorrectLayoutSettingsWindow::unsavedChangesMessage()
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

void CorrectLayoutSettingsWindow::handleEApplyButton()
{
    _settings.setValue("exceptions/blacklist", ui->eExceptionsPlainTextEditCl->toPlainText());
    _settings.setValue("exceptions/isWhiteList", ui->eWhiteListCheckBoxCl->isChecked());

    _eChanged = false;
}

void CorrectLayoutSettingsWindow::handleEWhiteList()
{
    _eChanged = true;
}

void CorrectLayoutSettingsWindow::handleEExceptionsChanged()
{
    _eChanged = true;
}

void CorrectLayoutSettingsWindow::handlePApplyButton()
{
    _settings.setValue("passwords/caplslock", ui->pCapsLockCheckBoxCl->isChecked());
    bool layoutChecked = ui->pLayoutCheckBoxCl->isChecked();
    _settings.setValue("passwords/layoutChecked", layoutChecked);
    if(layoutChecked)
    {
        int index = ui->pLayoutComboBoxCl->currentIndex();
        _settings.setValue("passwords/layout", WinApiAdapter::hklToStr(_layoutsList[index]));
    }
}

void CorrectLayoutSettingsWindow::handlePCapsLockCheckBox()
{
    _eChanged = true;
}

void CorrectLayoutSettingsWindow::handlePLayoutCheckBox()
{
    _eChanged = true;
    ui->pLayoutComboBoxCl->setEnabled(!ui->pLayoutComboBoxCl->isEnabled());
}

void CorrectLayoutSettingsWindow::handlePLayoutComboBox()
{
    _eChanged = true;
}
