QT += core gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    correctlayout.cpp \
    keypress.cpp \
    layoutchecker.cpp

HEADERS += \
    correctlayout.h \
    keypress.h \
    layoutchecker.h

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


# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../WinApiAdapter/release/ -lWinApiAdapter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../WinApiAdapter/debug/ -lWinApiAdapter

INCLUDEPATH += $$PWD/../WinApiAdapter
DEPENDPATH += $$PWD/../WinApiAdapter

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/release/libWinApiAdapter.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/debug/libWinApiAdapter.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/release/WinApiAdapter.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../WinApiAdapter/debug/WinApiAdapter.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtGlobalInput/release/ -lQtGlobalInput
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtGlobalInput/debug/ -lQtGlobalInput

INCLUDEPATH += $$PWD/../QtGlobalInput/src
DEPENDPATH += $$PWD/../QtGlobalInput/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/release/libQtGlobalInput.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/debug/libQtGlobalInput.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/release/QtGlobalInput.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QtGlobalInput/debug/QtGlobalInput.lib
