
TARGET=../apt-indicator
TEMPLATE = app
QT += network
QMAKE_CXXFLAGS += -pedantic $(RPM_OPT_FLAGS)
#LIBS += -lapt-pkg

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

SOURCES = main.cpp
SOURCES += agent.cpp configuration.cpp help_browser.cpp config_dialog.cpp
SOURCES += info_window.cpp

HEADERS = ../config.h
HEADERS += agent.h configuration.h help_browser.h config_dialog.h
HEADERS += info_window.h

FORMS = config_dialog.ui info_window.ui help_browser.ui

TRANSLATIONS = \
    ../translations/apt_indicator_ru.ts \
    ../translations/apt_indicator_uk.ts \
    ../translations/untranslated.ts

DATA = $$TARGET.pro

RESOURCES = pixmaps.qrc

target.path = /usr/bin/
INSTALLS += target

trans.path = /usr/share/$$TARGET/translations
trans.files = translations/apt_indicator_*.qm
INSTALLS += trans
