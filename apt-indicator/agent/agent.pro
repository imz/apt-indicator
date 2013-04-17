
TARGET=../apt-indicator
TEMPLATE = app
QT += network
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
QMAKE_CXXFLAGS += -pedantic -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

SOURCES = qtlocalpeer.cpp qtlockedfile.cpp qtlockedfile_unix.cpp qtsingleapplication.cpp
SOURCES += main.cpp
SOURCES += agent.cpp configuration.cpp help_browser.cpp config_dialog.cpp
SOURCES += info_window.cpp utils.cpp


HEADERS = qtlocalpeer.h qtlockedfile.h qtsingleapplication.h
HEADERS += ../config.h
HEADERS += agent.h configuration.h help_browser.h config_dialog.h
HEADERS += info_window.h utils.h

FORMS = config_dialog.ui info_window.ui help_browser.ui

TRANSLATIONS = \
    ../translations/apt_indicator_agent_ru.ts \
    ../translations/apt_indicator_agent_uk.ts \
    ../translations/untranslated_agent.ts

DATA = $$TARGET.pro

RESOURCES = pixmaps.qrc

target.path = /usr/bin/
INSTALLS += target

trans.path = /usr/share/$$TARGET/translations
trans.files = translations/apt_indicator_agent_*.qm
INSTALLS += trans
