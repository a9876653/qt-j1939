
unix|win32: LIBS += -L$$PWD/ -lzlgcan

SOURCES += \
    $$PWD/zlgcan_ctrl.cpp

HEADERS += \
    $$PWD/canframe.h \
    $$PWD/config.h \
    $$PWD/typedef.h \
    $$PWD/zlgcan.h  \
    $$PWD/zlgcan_ctrl.h

DISTFILES += \
    $$PWD/zlgcan.dll \
    $$PWD/zlgcan.lib

