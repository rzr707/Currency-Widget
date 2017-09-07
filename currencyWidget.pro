QT += widgets xml network

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    src/main.cpp

HEADERS += \
    src/chart.h \
    src/current.h \
    src/tray.h


RESOURCES += \
    res.qrc

RC_FILE = icons\icon.rc

DISTFILES += \
    readme.txt \
    todo.txt
