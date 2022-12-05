include($$PWD/zlgcan/zlgcan.pri)
include($$PWD/j1939/j1939.pri)
include($$PWD/boot_update/boot_update.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/j1939/include
INCLUDEPATH += $$PWD/zlgcan
INCLUDEPATH += $$PWD/boot_update

HEADERS += \
    $$PWD/boot_port.h \
    $$PWD/comm_j1939.h \
    $$PWD/j1939_config.h \
    $$PWD/middle_signal.h

SOURCES += \
    $$PWD/boot_port.cpp \
    $$PWD/comm_j1939.cpp \
    $$PWD/middle_signal.cpp



