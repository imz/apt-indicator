
TARGET=../apt-indicator-checker
TEMPLATE = app
QT -= gui
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
QMAKE_CXXFLAGS += -pedantic -D_FILE_OFFSET_BITS=64 -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_QSTRINGBUILDER
LIBS += -lapt-pkg
CONFIG += c++11

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

SOURCES = main.cpp
SOURCES += checker.cpp dist_upgrade.cpp

HEADERS = ../config.h
HEADERS += checker.h dist_upgrade.h

DATA = $$TARGET.pro

target.path = /usr/bin/
INSTALLS += target

TRANSLATIONS = \
    ../translations/apt_indicator_checker_ru.ts \
    ../translations/apt_indicator_checker_uk.ts \
    ../translations/untranslated_checker.ts
