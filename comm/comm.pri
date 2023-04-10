include($$PWD/zlgcan/zlgcan.pri)

include($$PWD/controlcan/controlcan.pri)
include($$PWD/j1939/j1939.pri)
include($$PWD/boot_update/boot_update.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/j1939/include
INCLUDEPATH += $$PWD/zlgcan
INCLUDEPATH += $$PWD/controlcan

HEADERS += \
    $$PWD/canbase.h \
    $$PWD/comm_j1939.h \
    $$PWD/comm_j1939_port.h \
    $$PWD/j1939_config.h \
    $$PWD/j1939_modbus.h \
    $$PWD/j1939_modbus_master.h

SOURCES += \
    $$PWD/comm_j1939.cpp \
    $$PWD/comm_j1939_port.cpp \
    $$PWD/j1939_modbus_master.cpp



