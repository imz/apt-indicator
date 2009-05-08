
TARGET=../apt-indicator
TEMPLATE = app
QT -= gui core
QMAKE_CFLAGS += -pedantic $(RPM_OPT_FLAGS)

OBJECTS_DIR = .obj

SOURCES = main.c

HEADERS = ../config.h

DATA = $$TARGET.pro

target.path = /usr/bin/
INSTALLS += target
