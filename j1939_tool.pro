QT       += core gui

TARGET = ../bin/j1939_tool

RC_ICONS = battery.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    system.cpp

HEADERS += \
    mainwindow.h \ \
    system.h

FORMS += \
    mainwindow.ui

include($$PWD/comm/comm.pri)
include($$PWD/bytes/bytes.pri)
include($$PWD/page/page.pri)
include($$PWD/widget/widget.pri)
include($$PWD/msg_signals/msg_signals.pri)
include($$PWD/boot_update/boot_update.pri)

INCLUDEPATH += $$PWD/bytes
INCLUDEPATH += $$PWD/widget
INCLUDEPATH += $$PWD/msg_signals
INCLUDEPATH += $$PWD/comm
INCLUDEPATH += $$PWD/page
INCLUDEPATH += $$PWD/boot_update


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource/res.qrc




