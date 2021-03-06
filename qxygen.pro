CONFIG += qt thread warn_on x11 release

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
		src/tray \

INCLUDEPATH = $$DEPENDPATH

FORMS = src/qxygen.ui \
	src/descrdialog.ui \
	src/roster/useradd.ui \
	src/settings/profile.ui \
	src/settings/networkswidget.ui \
	src/settings/generalswidget.ui \
	src/settings/lookswidget.ui \
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
	src/chat/chatwindow.h \
	src/chat/chattextedit.h \
	src/filetransfer/filetransfer.h \
	src/filetransfer/fileincoming.h \
	src/filetransfer/sha1.h

SOURCES=src/main.cpp \
	src/qxygen.cpp \
	src/descrdialog.cpp \
	src/settings/settings.cpp \
	src/settings/profileform.cpp \
	src/settings/settingsdialog.cpp \
	src/roster/roster_view.cpp \
	src/roster/roster_item.cpp \
	src/roster/roster_delegate.cpp \
	src/roster/roster_widget.cpp \
	src/roster/useradd.cpp \
	src/protocol/tlen.cpp \
	src/protocol/auth.cpp \
	src/chat/chatwindow.cpp \
	src/chat/chattextedit.cpp \
	src/filetransfer/filetransfer.cpp \
	src/filetransfer/fileincoming.cpp \
	src/filetransfer/sha1.cpp

TRANSLATIONS =	translation/qxygen_pl.ts \
		translation/qxygen_en.ts