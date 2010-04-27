
TARGET=../apt-indicator
TEMPLATE = app
QT -= gui core
rpm_opt_flags = $$(RPM_OPT_FLAGS)
CONFIG(release) {
    isEmpty(rpm_opt_flags) {
       QMAKE_CXXFLAGS += -O2
    } else {
       QMAKE_CXXFLAGS += $$rpm_opt_flags
    }
    QMAKE_CXXFLAGS += -DNDEBUG
} else {
    QMAKE_CXXFLAGS += -g -O0
}
QMAKE_CXXFLAGS += -pedantic

OBJECTS_DIR = .obj

SOURCES = main.c

HEADERS = ../config.h

DATA = $$TARGET.pro

target.path = /usr/bin/
INSTALLS += target
