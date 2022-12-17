#include "correctlayoutsettingswindow.h"
#include "ui_correctlayoutsettingswindow.h"

#include <windows.h>
#include <sstream>

#include <QStringListModel>
#include <QMessageBox>
#include <QFile>

#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

CorrectLayoutSettingsWindow::CorrectLayoutSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorrectLayoutSettingsWindow)
{
    _settings.beginGroup("CorrectLayout");

    _shortcutActivateKey = new Key();
    _shortcutActivateKey->vkCode = -1;

    _shortcutSelectKey = new Key();
    _shortcutSelectKey->vkCode = -1;

    _shortcutPauseKey = new Key();
    _shortcutPauseKey->vkCode = -1;

    _shortcutNextKey = new Key();
    _shortcutNextKey->vkCode = -1;

    _shortcutUndoKey = new Key();
    _shortcutUndoKey->vkCode = -1;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged()));

    connect(ui->gApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleGApplyButton()));
    connect(ui->gAutoStartCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleGAutoStart()));

    connect(ui->lsApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleLsApplyButton()));
    connect(ui->lsShortcutActivateButtonCl, SIGNAL (clicked()), this, SLOT (handleLsShortcutActivateButton()));
    connect(ui->lsShortcutSelectButtonCl, SIGNAL (clicked()), this, SLOT (handleLsShortcutSelectButton()));
    connect(ui->lsActiveCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleLsActivateCheckBox()));
    connect(ui->lsAutoCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleLsAutoCheckBox()));
    connect(ui->lsAddButtonCl, SIGNAL (clicked()), this, SLOT (handleLsAddButton()));
    connect(ui->lsRemoveButtonCl, SIGNAL (clicked()), this, SLOT (handleLsRemoveButton()));

    connect(ui->eApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleEApplyButton()));
    connect(ui->eWhiteListCheckBoxCl, SIGNAL (clicked()), this, SLOT (handleEWhiteList()));
    connect(ui->eExceptionsPlainTextEditCl, SIGNAL (textChanged()), this, SLOT (handleEExceptionsChanged()));

    connect(ui->pApplyButtonCl, SIGNAL (clicked()), this, SLOT (handlePApplyButton()));
    connect(ui->pCapsLockCheckBoxCl, SIGNAL (clicked()), this, SLOT (handlePCapsLockCheckBox()));
    connect(ui->pLayoutCheckBoxCl, SIGNAL (clicked()), this, SLOT (handlePLayoutCheckBox()));
    connect(ui->pLayoutComboBoxCl, SIGNAL (currentIndexChanged()), this, SLOT (handlePLayoutComboBox()));

    connect(ui->sApplyButtonCl, SIGNAL (clicked()), this, SLOT (handleSApplyButton()));
    connect(ui->sPauseButtonCl, SIGNAL (clicked()), this, SLOT (handleSPauseButton()));
    connect(ui->sNextButtonCl, SIGNAL (clicked()), this, SLOT (handleSNextButton()));
    connect(ui->sUndoButtonCl, SIGNAL (clicked()), this, SLOT (handleSUndoButton()));

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

    if(_settings.value("shortcuts/shortcut/pause/active").toBool())
    {
        QString shortcut = "";
        if(_settings.value("shortcuts/shortcut/pause/ctrl").toBool())
            shortcut += "Ctrl + ";
        if(_settings.value("shortcuts/shortcut/pause/shift").toBool())
            shortcut += "Shift + ";
        if(_settings.value("shortcuts/shortcut/pause/alt").toBool())
            shortcut += "Alt + ";
        shortcut += WinApiAdapter::vkToString(_settings.value("shortcuts/shortcut/pause/vkCode").toUInt());

        ui->sPauseLineEditCl->setText(shortcut);
    }
    else
    {
        ui->sPauseLineEditCl->setText("");
    }

    if(_settings.value("shortcuts/shortcut/next/active").toBool())
    {
        QString shortcut = "";
        if(_settings.value("shortcuts/shortcut/next/ctrl").toBool())
            shortcut += "Ctrl + ";
        if(_settings.value("shortcuts/shortcut/next/shift").toBool())
            shortcut += "Shift + ";
        if(_settings.value("shortcuts/shortcut/next/alt").toBool())
            shortcut += "Alt + ";
        shortcut += WinApiAdapter::vkToString(_settings.value("shortcuts/shortcut/next/vkCode").toUInt());

        ui->sNextLineEditCl->setText(shortcut);
    }
    else
    {
        ui->sNextLineEditCl->setText("");
    }

    if(_settings.value("shortcuts/shortcut/undo/active").toBool())
    {
        QString shortcut = "";
        if(_settings.value("shortcuts/shortcut/undo/ctrl").toBool())
            shortcut += "Ctrl + ";
        if(_settings.value("shortcuts/shortcut/undo/shift").toBool())
            shortcut += "Shift + ";
        if(_settings.value("shortcuts/shortcut/undo/alt").toBool())
            shortcut += "Alt + ";
        shortcut += WinApiAdapter::vkToString(_settings.value("shortcuts/shortcut/undo/vkCode").toUInt());

        ui->sUndoLineEditCl->setText(shortcut);
    }
    else
    {
        ui->sUndoLineEditCl->setText("");
    }

    _gChanged = false;
    _lsChanged = false;
    _eChanged = false;
    _pChanged = false;
    _sChanged = false;
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
    else if(oldTab == 3 && _pChanged)
    {
        if(!unsavedChangesMessage() && !(ui->mainTabs->currentIndex() == 2))
        {
            ui->mainTabs->setCurrentIndex(oldTab);
            return;
        }
    }
    else if(oldTab == 4 && _sChanged)
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
    _shortcutActivate = true;
    _shortcutSelect = false;
    _shortcutPause = false;
    _shortcutNext = false;
    _shortcutUndo = false;
}

void CorrectLayoutSettingsWindow::handleLsShortcutSelectButton()
{
    _shortcutActivate = false;
    _shortcutSelect = true;
    _shortcutPause = false;
    _shortcutNext = false;
    _shortcutUndo = false;
}

void CorrectLayoutSettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if(_shortcutActivate || _shortcutSelect || _shortcutPause || _shortcutNext || _shortcutUndo)
    {
        if(event->key() == Qt::Key_Control || event->key() ==Qt::Key_Shift || event->key() == Qt::Key_Alt)
            return;
        if(event->key() == Qt::Key_Escape)
        {
            _shortcutActivateKey = new Key();
            _shortcutActivateKey->vkCode = 0;
            _shortcutActivate = false;
            ui->lsShortcutActivateLineEditCl->setText("");
            _lsChanged = true;
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
            shortcut += "Ctrl + ";
        if(key->shift)
            shortcut += "Shift + ";
        if(key->alt)
            shortcut += "Alt + ";
        shortcut += WinApiAdapter::vkToString(key->vkCode);
        if(_shortcutActivate)
        {
            _shortcutActivateKey = key;

            ui->lsShortcutActivateLineEditCl->setText(shortcut);

            _shortcutActivate = false;

            _lsChanged = true;
        }
        else if(_shortcutSelect)
        {
            _shortcutSelectKey = key;

            ui->lsShortcutSelectLineEditCl->setText(shortcut);

            _shortcutSelect = false;

            _lsChanged = true;
        }
        else if(_shortcutPause)
        {
            _shortcutPauseKey = key;

            ui->sPauseLineEditCl->setText(shortcut);

            _shortcutPause = false;

            _sChanged = true;
        }
        else if(_shortcutNext)
        {
            _shortcutNextKey = key;

            ui->sNextLineEditCl->setText(shortcut);

            _shortcutNext = false;

            _sChanged = true;
        }
        else if(_shortcutUndo)
        {
            _shortcutUndoKey = key;

            ui->sUndoLineEditCl->setText(shortcut);

            _shortcutUndo = false;

            _sChanged = true;
        }
    }
}

void CorrectLayoutSettingsWindow::handleLsActivateCheckBox()
{
    _lsChanged = true;
}

void CorrectLayoutSettingsWindow::handleLsAutoCheckBox()
{
    _lsChanged = true;
}

void CorrectLayoutSettingsWindow::handleLsAddButton()
{
    QString newFile;

    QString word = ui->lsWordLineEditCl->text().toLower();

    wchar_t name[MAXNAMELENGTH];
    LANGID language = (LANGID)(((UINT)_layoutsList[_index.row()]) & 0x0000FFFF);
    LCID locale = MAKELCID(language, SORT_DEFAULT);

    GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

    qDebug() << "Start editing " << QString::fromWCharArray(name) << "(" << WinApiAdapter::decToHex(_layoutsList[_index.row()]) << ") dictionary";

    QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(_layoutsList[_index.row()]) + ".txt";

    QFile inputFile(path);

    if (!inputFile.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Can't open dictionarie file";
        return;
    }
    QTextStream in(&inputFile);

    QString noShift = in.readLine().toUtf8();

    newFile += noShift + "\n";

    QString shift = in.readLine();
    newFile += shift + "\n";

    for(QChar c : word)
    {
        if(!noShift.contains(c) && !shift.contains(c))
        {
            QMessageBox::information(this, "Wrong format!",
                                            "Alphabet for this language doesn't have all the symbols");
            qDebug() << "Alphabet for this language doesn't have all the symbols";
            return;
        }
    }

    QString oldLine = "";

    bool placed = false;

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if(word == line)
        {
            QMessageBox::information(this, "Word exists!",
                                            "This word is already exists in dicionarie");
            qDebug() << "This word already exists";
            return;
        }

        if(word > oldLine && word < line  && !placed)
        {
            newFile += word + "\n";
            placed = true;
        }
        newFile += line + "\n";
        oldLine = line;
    }

    inputFile.close();

    QFile outputFile(path);

    if (!outputFile.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Can't open dictionarie file";
        return;
    }

    QTextStream out(&outputFile);
    out.setEncoding(QStringConverter::Encoding::Utf8);

    out << newFile;
}

void CorrectLayoutSettingsWindow::handleLsRemoveButton()
{
    QString newFile;

    QString word = ui->lsWordLineEditCl->text().toLower();

    wchar_t name[MAXNAMELENGTH];
    LANGID language = (LANGID)(((UINT)_layoutsList[_index.row()]) & 0x0000FFFF);
    LCID locale = MAKELCID(language, SORT_DEFAULT);

    GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

    qDebug() << "Start editing " << QString::fromWCharArray(name) << "(" << WinApiAdapter::decToHex(_layoutsList[_index.row()]) << ") dictionary";

    QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(_layoutsList[_index.row()]) + ".txt";

    QFile inputFile(path);

    if (!inputFile.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Can't open dictionarie file";
        return;
    }
    QTextStream in(&inputFile);

    QString noShift = in.readLine().toUtf8();

    newFile += noShift + "\n";

    QString shift = in.readLine();
    newFile += shift + "\n";

    for(QChar c : word)
    {
        if(!noShift.contains(c) && !shift.contains(c))
        {
            QMessageBox::information(this, "Wrong format!",
                                            "Alphabet for this language doesn't have all the symbols");
            qDebug() << "Alphabet for this language doesn't have all the symbols";
            return;
        }
    }

    bool exists = false;

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if(word != line)
        {
            newFile += line + "\n";
        }
        else
        {
            exists = true;
        }
    }

    if(!exists)
    {
        QMessageBox::information(this, "No word!",
                                        "There is no such word in dictionarie!");
    }

    inputFile.close();

    QFile outputFile(path);

    if (!outputFile.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Can't open dictionarie file";
        return;
    }

    QTextStream out(&outputFile);
    out.setEncoding(QStringConverter::Encoding::Utf8);

    out << newFile;

    outputFile.close();
}

void CorrectLayoutSettingsWindow::handleLsApplyButton()
{
    if(!_lsChanged)
        return;

    _lsChanged = false;

    QString itemText;
    itemText = WinApiAdapter::hklToStr(_layoutsList[_index.row()]);

    _settings.setValue("runOnStart", ui->gAutoStartCheckBoxCl->isChecked());

    _settings.setValue("layouts/" + itemText + "/deactivated", ui->lsActiveCheckBoxCl->isChecked());
    _settings.setValue("layouts/" + itemText + "/auto", ui->lsAutoCheckBoxCl->isChecked());

    if(_shortcutActivateKey->vkCode != -1)
    {
        _settings.setValue("layouts/" + itemText + "/shortcut/activate/active", _shortcutActivateKey->vkCode != 0);
        if(_shortcutActivateKey->vkCode != 0)
        {
            _settings.setValue("layouts/" + itemText + "/shortcut/activate/ctrl", _shortcutActivateKey->ctrl);
            _settings.setValue("layouts/" + itemText + "/shortcut/activate/shift", _shortcutActivateKey->shift);
            _settings.setValue("layouts/" + itemText + "/shortcut/activate/alt", _shortcutActivateKey->alt);
            _settings.setValue("layouts/" + itemText + "/shortcut/activate/qtCode", _shortcutActivateKey->scanCode);
            _settings.setValue("layouts/" + itemText + "/shortcut/activate/vkCode", _shortcutActivateKey->vkCode);
        }
        else
        {
            _settings.remove("layouts/" + itemText + "/shortcut/activate/ctrl");
            _settings.remove("layouts/" + itemText + "/shortcut/activate/shift");
            _settings.remove("layouts/" + itemText + "/shortcut/activate/alt");
            _settings.remove("layouts/" + itemText + "/shortcut/activate/qtCode");
            _settings.remove("layouts/" + itemText + "/shortcut/activate/vkCode");
        }
    }

    if(_shortcutSelectKey->vkCode != -1)
    {
        _settings.setValue("layouts/" + itemText + "/shortcut/select/active", _shortcutSelectKey->vkCode != 0);
        if(_shortcutSelectKey->vkCode != 0)
        {
            _settings.setValue("layouts/" + itemText + "/shortcut/select/ctrl", _shortcutSelectKey->ctrl);
            _settings.setValue("layouts/" + itemText + "/shortcut/select/shift", _shortcutSelectKey->shift);
            _settings.setValue("layouts/" + itemText + "/shortcut/select/alt", _shortcutSelectKey->alt);
            _settings.setValue("layouts/" + itemText + "/shortcut/select/qtCode", _shortcutSelectKey->scanCode);
            _settings.setValue("layouts/" + itemText + "/shortcut/select/vkCode", _shortcutSelectKey->vkCode);
        }
        else
        {
            _settings.remove("layouts/" + itemText + "/shortcut/select/ctrl");
            _settings.remove("layouts/" + itemText + "/shortcut/select/shift");
            _settings.remove("layouts/" + itemText + "/shortcut/select/alt");
            _settings.remove("layouts/" + itemText + "/shortcut/select/qtCode");
            _settings.remove("layouts/" + itemText + "/shortcut/select/vkCode");
        }
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
    ui->lsAutoCheckBoxCl->setChecked(_settings.value("layouts/" + itemText + "/auto", true).toBool());

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
        _pChanged = false;
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

    _pChanged = false;
}

void CorrectLayoutSettingsWindow::handlePCapsLockCheckBox()
{
    _pChanged = true;
}

void CorrectLayoutSettingsWindow::handlePLayoutCheckBox()
{
    _pChanged = true;
    ui->pLayoutComboBoxCl->setEnabled(!ui->pLayoutComboBoxCl->isEnabled());
}

void CorrectLayoutSettingsWindow::handlePLayoutComboBox()
{
    _pChanged = true;
}

void CorrectLayoutSettingsWindow::handleSApplyButton()
{
    if(!_sChanged)
        return;

    _sChanged = false;

    if(_shortcutPauseKey->vkCode != -1)
    {
        _settings.setValue("shortcuts/shortcut/pause/active", _shortcutPauseKey->vkCode != 0);
        if(_shortcutPauseKey->vkCode != 0)
        {
            _settings.setValue("shortcuts/shortcut/pause/ctrl", _shortcutPauseKey->ctrl);
            _settings.setValue("shortcuts/shortcut/pause/shift", _shortcutPauseKey->shift);
            _settings.setValue("shortcuts/shortcut/pause/alt", _shortcutPauseKey->alt);
            _settings.setValue("shortcuts/shortcut/pause/qtCode", _shortcutPauseKey->scanCode);
            _settings.setValue("shortcuts/shortcut/pause/vkCode", _shortcutPauseKey->vkCode);
        }
        else
        {
            _settings.remove("shortcuts/shortcut/pause/ctrl");
            _settings.remove("shortcuts/shortcut/pause/shift");
            _settings.remove("shortcuts/shortcut/pause/alt");
            _settings.remove("shortcuts/shortcut/pause/qtCode");
            _settings.remove("shortcuts/shortcut/pause/vkCode");
        }
    }

    if(_shortcutNextKey->vkCode != -1)
    {
        _settings.setValue("shortcuts/shortcut/next/active", _shortcutNextKey->vkCode != 0);
        if(_shortcutNextKey->vkCode != 0)
        {
            _settings.setValue("shortcuts/shortcut/next/ctrl", _shortcutNextKey->ctrl);
            _settings.setValue("shortcuts/shortcut/next/shift", _shortcutNextKey->shift);
            _settings.setValue("shortcuts/shortcut/next/alt", _shortcutNextKey->alt);
            _settings.setValue("shortcuts/shortcut/next/qtCode", _shortcutNextKey->scanCode);
            _settings.setValue("shortcuts/shortcut/next/vkCode", _shortcutNextKey->vkCode);
        }
        else
        {
            _settings.remove("shortcuts/shortcut/next/ctrl");
            _settings.remove("shortcuts/shortcut/next/shift");
            _settings.remove("shortcuts/shortcut/next/alt");
            _settings.remove("shortcuts/shortcut/next/qtCode");
            _settings.remove("shortcuts/shortcut/next/vkCode");
        }
    }

    if(_shortcutUndoKey->vkCode != -1)
    {
        _settings.setValue("shortcuts/shortcut/undo/active", _shortcutUndoKey->vkCode != 0);
        if(_shortcutUndoKey->vkCode != 0)
        {
            _settings.setValue("shortcuts/shortcut/undo/ctrl", _shortcutUndoKey->ctrl);
            _settings.setValue("shortcuts/shortcut/undo/shift", _shortcutUndoKey->shift);
            _settings.setValue("shortcuts/shortcut/undo/alt", _shortcutUndoKey->alt);
            _settings.setValue("shortcuts/shortcut/undo/qtCode", _shortcutUndoKey->scanCode);
            _settings.setValue("shortcuts/shortcut/undo/vkCode", _shortcutUndoKey->vkCode);
        }
        else
        {
            _settings.remove("shortcuts/shortcut/undo/ctrl");
            _settings.remove("shortcuts/shortcut/undo/shift");
            _settings.remove("shortcuts/shortcut/undo/alt");
            _settings.remove("shortcuts/shortcut/undo/qtCode");
            _settings.remove("shortcuts/shortcut/undo/vkCode");
        }
    }
}

void CorrectLayoutSettingsWindow::handleSPauseButton()
{
    _shortcutActivate = false;
    _shortcutSelect = false;
    _shortcutPause = true;
    _shortcutNext = false;
    _shortcutUndo = false;
}

void CorrectLayoutSettingsWindow::handleSNextButton()
{
    _shortcutActivate = false;
    _shortcutSelect = false;
    _shortcutPause = false;
    _shortcutNext = true;
    _shortcutUndo = false;
}

void CorrectLayoutSettingsWindow::handleSUndoButton()
{
    _shortcutActivate = false;
    _shortcutSelect = false;
    _shortcutPause = false;
    _shortcutNext = false;
    _shortcutUndo = true;
}
