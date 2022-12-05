include($$PWD/zlgcan/zlgcan.pri)
include($$PWD/j1939/j1939.pri)
include($$PWD/boot_update/boot_update.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/j1939/include
INCLUDEPATH += $$PWD/zlgcan

HEADERS += \
    $$PWD/comm_j1939.h \
    $$PWD/j1939_config.h

SOURCES += \
    $$PWD/comm_j1939.cpp



