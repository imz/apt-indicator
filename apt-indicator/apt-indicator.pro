
TEMPLATE = app
QT += network
QMAKE_CXXFLAGS += -pedantic $(RPM_OPT_FLAGS)
LIBS += -lapt-pkg

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

SOURCES = main.cpp
SOURCES += agent.cpp configuration.cpp help_browser.cpp config_dialog.cpp
SOURCES += dist_upgrade.cpp info_window.cpp run_program.cpp

HEADERS = config.h
HEADERS += config.h agent.h assert.h configuration.h dist_upgrade.h run_program.h
HEADERS += config_dialog.h info_window.h help_browser.h

FORMS = config_dialog.ui info_window.ui help_browser.ui

DATA = *.pro $$TARGET.desktop

RESOURCES = pixmaps.qrc

TRANSLATIONS = \
    translations/apt_indicator_ru.ts \
    translations/apt_indicator_uk.ts \
    translations/untranslated.ts

target.path = /usr/bin/
INSTALLS += target

trans.path = /usr/share/$$TARGET/translations
trans.files = translations/apt_indicator_*.qm
INSTALLS += trans
