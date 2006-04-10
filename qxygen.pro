CONFIG += qt warn_on x11 debug

QT += network xml

RCC_DIR=tmp/rcc
OBJECTS_DIR=tmp/obj
MOC_DIR=tmp/moc
UI_DIR=tmp/ui

DEPENDPATH=src/roster src/settings

INCLUDEPATH=src src/roster src/protocol src/chat src/tray src/settings

FORMS = src/qxygen.ui \
	src/roster/useradd.ui

RESOURCES=qxygen.qrc

HEADERS=src/qxygen.h \
	src/settings/settings.h \
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
	src/chat/chattextedit.h

SOURCES=src/main.cpp \
	src/qxygen.cpp \
	src/settings/settings.cpp \
	src/roster/roster_view.cpp \
	src/roster/roster_item.cpp \
	src/roster/roster_delegate.cpp \
	src/roster/roster_widget.cpp \
	src/roster/useradd.cpp \
	src/protocol/tlen.cpp \
	src/protocol/auth.cpp \
	src/tray/trayicon.cpp \
	src/chat/chatwindow.cpp \
	src/chat/chattextedit.cpp

TRANSLATIONS =	translation/qtlen_pl.ts

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
