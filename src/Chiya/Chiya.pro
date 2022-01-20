QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

VERSION = 1.1.0

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutwindow.cpp \
    layoutcontrollersettingswindow.cpp \
    main.cpp \
    mainsettingswindow.cpp \
    mainwindow.cpp

HEADERS += \
    aboutwindow.h \
    layoutcontrollersettingswindow.h \
    mainsettingswindow.h \
    mainwindow.h

FORMS += \
    aboutwindow.ui \
    layoutcontrollersettingswindow.ui \
    mainsettingswindow.ui \
    mainwindow.ui

LIBS += \
    -luser32 \
    -lKernel32 \
    -lgdi32 \
    -lwinspool \
    -lcomdlg32 \
    -ladvapi32 \
    -lshell32 \
    -lole32 \
    -loleaut32 \
    -luuid \
    -lodbc32 \
    -lodbccp32

RESOURCES += \
  resources.qrc

RC_ICONS = icons/IcoBackground.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../RunGuard/release/ -lRunGuard
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../RunGuard/debug/ -lRunGuard

INCLUDEPATH += $$PWD/../RunGuard
DEPENDPATH += $$PWD/../RunGuard

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../RunGuard/release/libRunGuard.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../RunGuard/debug/libRunGuard.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../RunGuard/release/RunGuard.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../RunGuard/debug/RunGuard.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../LayoutController/release/ -lLayoutController
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../LayoutController/debug/ -lLayoutController

INCLUDEPATH += $$PWD/../LayoutController
DEPENDPATH += $$PWD/../LayoutController

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LayoutController/release/libLayoutController.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LayoutController/debug/libLayoutController.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LayoutController/release/LayoutController.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LayoutController/debug/LayoutController.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../AdminRights/release/ -lAdminRights
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../AdminRights/debug/ -lAdminRights

INCLUDEPATH += $$PWD/../AdminRights
DEPENDPATH += $$PWD/../AdminRights

DISTFILES += \
  icons/IcoBackground.ico \
  icons/NormalIco.ico

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtGlobalInput/release/ -lQtGlobalInput
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtGlobalInput/debug/ -lQtGlobalInput

INCLUDEPATH += $$PWD/../QtGlobalInput/src
DEPENDPATH += $$PWD/../QtGlobalInput/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/release/libQtGlobalInput.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/debug/libQtGlobalInput.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/release/QtGlobalInput.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/debug/QtGlobalInput.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../WinApiAdapter/release/ -lWinApiAdapter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../WinApiAdapter/debug/ -lWinApiAdapter

INCLUDEPATH += $$PWD/../WinApiAdapter
DEPENDPATH += $$PWD/../WinApiAdapter

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/release/libWinApiAdapter.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/debug/libWinApiAdapter.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/release/WinApiAdapter.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/debug/WinApiAdapter.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CorrectLayout/release/ -lCorrectLayout
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CorrectLayout/debug/ -lCorrectLayout

INCLUDEPATH += $$PWD/../CorrectLayout
DEPENDPATH += $$PWD/../CorrectLayout

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CorrectLayout/release/libCorrectLayout.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CorrectLayout/debug/libCorrectLayout.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CorrectLayout/release/CorrectLayout.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CorrectLayout/debug/CorrectLayout.lib
