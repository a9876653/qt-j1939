INCLUDEPATH += $$PWD/rt_list_slist


HEADERS += \
    $$PWD/boot_header.h \
    $$PWD/boot_master.h \
    $$PWD/boot_port.h \
    $$PWD/boot_update_node.h \
    $$PWD/frmbootloader.h \
    $$PWD/middle_signal.h \
    $$PWD/rt_list_slist/rt_list.h \
    $$PWD/rt_list_slist/rt_slist.h \
    $$PWD/rt_list_slist/rt_utils.h


SOURCES += \
    $$PWD/boot_master.cpp \
    $$PWD/boot_port.cpp \
    $$PWD/boot_update_node.cpp \
    $$PWD/frmbootloader.cpp \
    $$PWD/middle_signal.cpp

FORMS += \
    $$PWD/frmbootloader.ui

