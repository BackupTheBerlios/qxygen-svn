CONFIG += qt warn_on x11 release

QT += network xml

RCC_DIR=tmp/rcc
OBJECTS_DIR=tmp/obj
MOC_DIR=tmp/moc
UI_DIR=tmp/ui

DEPENDPATH =	src \
		src/chat \
		src/filetransfer \
		src/protocol \
		src/roster \
		src/settings \
		src/tray

INCLUDEPATH = $$DEPENDPATH

FORMS = src/qxygen.ui \
	src/descrdialog.ui \
	src/roster/useradd.ui \
	src/settings/profile.ui \
	src/settings/networkswidget.ui \
	src/settings/generalswidget.ui \
	src/filetransfer/filetransferdialog.ui \
	src/filetransfer/fileincomingdialog.ui

RESOURCES=qxygen.qrc

HEADERS=src/qxygen.h \
	src/descrdialog.h \
	src/settings/settings.h \
	src/settings/profileform.h \
	src/settings/settingsdialog.h \
	src/settings/settingswidget.h \
	src/roster/roster_view.h \
	src/roster/roster_item.h \
	src/roster/roster_delegate.h \
	src/roster/roster_widget.h \
	src/roster/roster_tiplabel.h \
	src/roster/useradd.h \
	src/protocol/tlen.h \
	src/protocol/auth.h \
	src/tray/trayicon.h \
	src/chat/chatwindow.h \
	src/chat/chattextedit.h \
	src/filetransfer/filetransfer.h \
	src/filetransfer/fileincoming.h

SOURCES=src/main.cpp \
	src/qxygen.cpp \
	src/descrdialog.cpp \
	src/settings/settings.cpp \
	src/settings/profileform.cpp \
	src/settings/settingsdialog.cpp \
	src/settings/settingswidget.h \
	src/roster/roster_view.cpp \
	src/roster/roster_item.cpp \
	src/roster/roster_delegate.cpp \
	src/roster/roster_widget.cpp \
	src/roster/useradd.cpp \
	src/protocol/tlen.cpp \
	src/protocol/auth.cpp \
	src/tray/trayicon.cpp \
	src/chat/chatwindow.cpp \
	src/chat/chattextedit.cpp \
	src/filetransfer/filetransfer.cpp \
	src/filetransfer/fileincoming.cpp

TRANSLATIONS =	translation/qxygen_pl.ts

unix:!mac {
	SOURCES += src/tray/trayicon_x11.cpp
	LIBS += -lXt
}
win32: {
	SOURCES += src/tray/trayicon_win.cpp
	win32-g++: {
	# Probably MinGW
		LIBS += libgdi32 libuser32 libshell32
	}
	else {
	# Assume msvc compiler
		LIBS += Gdi32.lib User32.lib shell32.lib
	}
}
mac: {
	SOURCES += src/tray/trayicon_mac.cpp
}
