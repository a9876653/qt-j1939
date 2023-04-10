
LIBS += -L$$PWD -lControlCAN
LIBS += $$PWD/ControlCAN.dll

HEADERS += \
    $$PWD/ControlCAN.h \
    $$PWD/ctrlcan.h

SOURCES += \
    $$PWD/ctrlcan.cpp

DISTFILES += \
    $$PWD/ControlCAN.dll \
    $$PWD/ControlCAN.lib
