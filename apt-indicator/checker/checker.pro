
TARGET=../apt-indicator-checker
TEMPLATE = app
QT -= gui
QMAKE_CXXFLAGS += -pedantic $(RPM_OPT_FLAGS)
LIBS += -lapt-pkg

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

SOURCES = main.cpp
SOURCES += checker.cpp dist_upgrade.cpp

HEADERS = ../indicator/config.h
HEADERS += checker.h dist_upgrade.h

DATA = $$TARGET.pro

target.path = /usr/bin/
INSTALLS += target

#TRANSLATIONS = \
#    ../translations/apt_indicator_checker_ru.ts \
#    ../translations/apt_indicator_checker_uk.ts \
#    ../translations/untranslated_checker.ts
