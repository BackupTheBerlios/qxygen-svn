/***************************************************************************
 *   Copyright (C) 2004-2005 by Naresh [Kamil Klimek]                      *
 *   naresh@tlen.pl                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QNetworkProxy>
#include <QDir>
#include <QMessageBox>
#include <QTcpServer>
#include <QScrollArea>
#include <QFontDialog>
#include <QColorDialog>
#include <QPixmap>

#include <QDebug>

#include "settingsdialog.h"
#include "settings.h"
#include "roster_view.h"
#include "roster_widget.h"

settingsDialog *settingsDlg=0;


/*******************************************************************************************
Settings dialog
*******************************************************************************************/

settingsDialog::settingsDialog(QWidget *parent): QDialog(parent) {
	settingsDlg=this;
	setWindowTitle(tr("Qxygen: settings"));
	setWindowIcon(QIcon(":logo.png"));
	setWindowFlags(Qt::Window);
	QGridLayout *main = new QGridLayout(this);
	QHBoxLayout *top = new QHBoxLayout();
	QHBoxLayout *bottom = new QHBoxLayout();
//	widgetScroll=new QScrollArea();
//	widgetScroll->setLayout( new QHBoxLayout() );
//	widgetScroll->setWidgetResizable(TRUE);
	settingsStackedWidget=new QStackedWidget(this);

	tabs = new QListWidget();
	tabs->setIconSize(QSize(32,32));
	tabs->setMovement(QListView::Static);
	tabs->setFlow(QListView::TopToBottom);
	tabs->setFixedWidth(120);

	ok=new QPushButton(tr("Ok"));
	apply=new QPushButton(tr("Apply"));
	cancel=new QPushButton(tr("Cancel"));

	bottom->addStretch();
	bottom->addWidget(ok);
	bottom->addWidget(apply);
	bottom->addWidget(cancel);

	new generalSettings();
	new networkSettings();
	new lookSettings();

	top->addWidget(tabs);
	top->addWidget(settingsStackedWidget);

	main->addLayout(top, 0, 0);
	main->addLayout(bottom, 1, 0);

	tabs->setItemSelected(tabs->item(0), TRUE);

	connect( tabs, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*) ), this, SLOT( swapSettingsWidget(QListWidgetItem*,QListWidgetItem*) ) );
	connect( cancel, SIGNAL( clicked() ), this, SLOT( cancelSettings() ) );
	connect( ok, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
	connect( apply, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
}

void settingsDialog::swapSettingsWidget(QListWidgetItem *curr, QListWidgetItem*) {
	settingsStackedWidget->setCurrentIndex( tabs->row(curr) );
}

void settingsDialog::saveSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>( ((QScrollArea*)settingsStackedWidget->widget(i))->widget() )->saveSettings();
	}

	emit updateSettings();

	QPushButton *b=qobject_cast<QPushButton*>(sender());
	if(b==ok)
		close();
}

void settingsDialog::loadSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>( ((QScrollArea*)settingsStackedWidget->widget(i))->widget() )->loadSettings();
	}
}

void settingsDialog::cancelSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>( ((QScrollArea*)settingsStackedWidget->widget(i))->widget() )->cancelSettings();
	}
	close();
}

void settingsDialog::insertSettings(settingsWidget *sw) {
	tabs->addItem( sw->settingsTab() );
	QScrollArea *sa=new QScrollArea();
	sa->setLayout( new QGridLayout() );
	sa->setWidgetResizable(TRUE);
	sa->setWidget( sw->settingsTab()->data(settingsWidget::widgetRole).value<QWidget*>() );
	settingsStackedWidget->addWidget( sa );
}

/*******************************************************************************************
Network settings
*******************************************************************************************/

networkSettings::networkSettings(QWidget *parent): settingsWidget(parent) {
	if( settings->profileValue("files/receivedir").toString().isEmpty() )
		settings->setProfileValue("files/receivedir", QDir::homePath());

	if( settings->profileValue("network/filetransfer/port").toInt()==0 )
		settings->setProfileValue("network/filetransfer/port", 4433);

	ui.setupUi(this);
	cancelSettings();
	item=new QListWidgetItem( QIcon(":network.png"), tr("Network") );
	QVariant V;
	V.setValue<QWidget*>( this );
	item->setData( widgetRole, V );

	connect( ui.testPortPushButton, SIGNAL( clicked() ), this, SLOT( testPort() ) );

	settingsDlg->insertSettings( this );
}

void networkSettings::saveSettings() {
	settings->setProfileValue( "network/useproxy", QVariant( ui.useProxyCheckBox->isChecked() ) );
	settings->setProfileValue( "network/proxy/host", ui.hostLineEdit->text() );
	settings->setProfileValue( "network/proxy/port", ui.portLineEdit->text() );
	settings->setProfileValue( "network/proxy/username", ui.userLineEdit->text() );
	settings->setProfileValue( "network/proxy/password", ui.passLineEdit->text() );
	settings->setProfileValue( "network/filetransfer/port", ui.sendPortSpinBox->value() );
	settings->setProfileValue( "files/receivedir", ui.downloadDirLineEdit->text() );
	settings->setProfileValue( "network/filetransfer/closeafterfinish", QVariant( ui.closeDialogCheckBox->isChecked() ) );
	loadSettings();
}

void networkSettings::loadSettings() {
	if( settings->profileValue("network/useproxy").toBool() ) {
		QNetworkProxy proxy;
		proxy.setHostName( settings->profileValue("network/proxy/host").toString() );
		proxy.setPort( settings->profileValue("network/proxy/port").toUInt() );
		proxy.setUser( settings->profileValue("network/proxy/username").toString() );
		proxy.setPassword( settings->profileValue("network/proxy/password").toString() );
		QNetworkProxy::setApplicationProxy(proxy);
	} else {
		QNetworkProxy::setApplicationProxy( QNetworkProxy::NoProxy );
	}
}

void networkSettings::cancelSettings() {
	if( settings->profileValue("network/useproxy").toBool() ) {
		ui.useProxyCheckBox->setChecked(TRUE);
		ui.groupBox->setEnabled(TRUE);
	}

	if( settings->profileValue("network/filetransfer/closeafterfinish").toBool() )
		ui.closeDialogCheckBox->setChecked(TRUE);

	ui.hostLineEdit->setText( settings->profileValue("network/proxy/host").toString() );
	ui.portLineEdit->setText( settings->profileValue("network/proxy/port").toString() );
	ui.userLineEdit->setText( settings->profileValue("network/proxy/username").toString() );
	ui.passLineEdit->setText( settings->profileValue("network/proxy/password").toString() );
	ui.downloadDirLineEdit->setText( settings->profileValue("files/receivedir").toString() );
	ui.sendPortSpinBox->setValue( settings->profileValue("network/filetransfer/port").toInt() );
}

void networkSettings::testPort() {
	QTcpServer *server = new QTcpServer(this);

	if( server->listen(QHostAddress::Any, (quint16)ui.sendPortSpinBox->value() ) )
		QMessageBox::information( static_cast<QWidget*>(this), tr("Port testing"), tr("Server can listen at port %1.").arg(ui.sendPortSpinBox->value()), 1);
	else
		QMessageBox::critical( static_cast<QWidget*>(this), tr("Port testing"), tr("Server couldn't listen at port %1. Please change port.").arg(ui.sendPortSpinBox->value()), 1, 0);

	delete server;
}

/*******************************************************************************************
General settings
*******************************************************************************************/

generalSettings::generalSettings(QWidget *parent): settingsWidget(parent) {
	translator=new QTranslator;
	ui.setupUi(this);
	cancelSettings();
	item=new QListWidgetItem( QIcon(":logo.png"), tr("General") );
	QVariant V;
	V.setValue<QWidget*>( this );
	item->setData( widgetRole, V );

	QDir dir(":/translation");
	QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);

	QStringListIterator i(fileNames);

	while(i.hasNext()) {
		QString l=i.next();
		translator->load( ":/translation/"+l );
		ui.langComboBox->addItem( translator->translate("qxygen", "LANG_NAME"), l );
	}

	if ( settings->exists( "window/language", 0 ) ) {
		ui.langComboBox->setCurrentIndex( ui.langComboBox->findData( settings->defaultValue("window/language") ) );
	} else {
		ui.langComboBox->setCurrentIndex( ui.langComboBox->findText("English") );
	}

	settingsDlg->insertSettings( this );
}

void generalSettings::saveSettings() {
	settings->setProfileValue( "roster/showGroups", ui.showGroups->isChecked() );
	settings->setProfileValue( "roster/showSubgroups", ui.showSubgroups->isChecked() );
	settings->setProfileValue( "roster/showDescription", ui.showDescription->isChecked() );

	settings->setDefaultValue( "window/language", ui.langComboBox->itemData( ui.langComboBox->currentIndex() ) );
	loadSettings();
}

void generalSettings::loadSettings() {
	if( settings->profileValue("roster/showSubgroups").toBool() )
		rosterModel->setSubgroups(TRUE);
	else
		rosterModel->setSubgroups(FALSE);

	if( settings->profileValue("roster/showDescription").toBool() )
		rosterModel->setShowDescr(TRUE);
	else
		rosterModel->setShowDescr(FALSE);
}

void generalSettings::cancelSettings() {
	ui.showGroups->setChecked( settings->profileValue("roster/showGroups").toBool() );
	ui.showSubgroups->setChecked( settings->profileValue("roster/showSubgroups").toBool() );
	ui.showDescription->setChecked( settings->profileValue("roster/showDescription").toBool() );
}

/*******************************************************************************************
Look settings
*******************************************************************************************/

lookSettings::lookSettings(QWidget *parent): settingsWidget(parent) {
	ui.setupUi(this);

	item=new QListWidgetItem( QIcon(":look.png"), tr("Look") );
	QVariant V;
	V.setValue<QWidget*>( this );
	item->setData( widgetRole, V );

	// setup defaults

	if ( !settings->exists("look/font/nick") ) {
		QFont roster=font();
		settings->setProfileValue("look/font/nick",roster);
		settings->setProfileValue("look/font/yourmsg",roster);
		settings->setProfileValue("look/font/usermsg",roster);
		roster.setItalic(TRUE);
		settings->setProfileValue("look/font/descr",roster);
		roster.setItalic(FALSE);
		roster.setBold(TRUE);
		settings->setProfileValue("look/font/headermsg",roster);
		settings->setProfileValue("look/font/group",roster);
		settings->setProfileValue("look/font/subgroup",roster);
	}

	if( !settings->exists("look/color/rosterbg") ) {
		QPalette p=palette();
		settings->setProfileValue( "look/color/rosterbg",p.color( QPalette::Base ) );
		settings->setProfileValue( "look/color/nickc",p.color( QPalette::Text ) );
		settings->setProfileValue( "look/color/descrc",p.color( QPalette::Text ) );
		settings->setProfileValue( "look/color/teditbg",p.color( QPalette::Base ) );
		settings->setProfileValue( "look/color/teditc",p.color( QPalette::Text ) );
		settings->setProfileValue( "look/color/yourbg",p.color( QPalette::Base ) );
		settings->setProfileValue( "look/color/yourc",p.color( QPalette::Text ) );
		settings->setProfileValue( "look/color/userbg",p.color( QPalette::AlternateBase ) );
		settings->setProfileValue( "look/color/userc",p.color( QPalette::Text ) );
	}

	cancelSettings();

	connect( ui.nickFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.descrFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.groupFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.subgroupFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.headerMsgFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.yourMsgFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );
	connect( ui.userMsgFontPushButton, SIGNAL(clicked()), this, SLOT(pickFont()) );

	connect( ui.rosterBgPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.nickCPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.descrCPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.tEditBgPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.tEditCPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.yourBgPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.yourCPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.userBgPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );
	connect( ui.userCPushButton, SIGNAL(clicked()), this, SLOT(pickColor()) );

	settingsDlg->insertSettings( this );
}

void lookSettings::saveSettings() {
	// FONTS
	settings->setProfileValue( "look/font/nick", ui.nickFontLineEdit->font() );
	settings->setProfileValue( "look/font/descr", ui.descrFontLineEdit->font() );
	settings->setProfileValue( "look/font/group", ui.groupFontLineEdit->font() );
	settings->setProfileValue( "look/font/subgroup", ui.subgroupFontLineEdit->font() );
	settings->setProfileValue( "look/font/headermsg", ui.headerMsgFontLineEdit->font() );
	settings->setProfileValue( "look/font/usermsg", ui.userMsgFontLineEdit->font() );
	settings->setProfileValue( "look/font/yourmsg", ui.yourMsgFontLineEdit->font() );

	// COLORS
	settings->setProfileValue( "look/color/rosterbg", getColor(ui.rosterBgPushButton) );
	settings->setProfileValue( "look/color/nickc", getColor(ui.nickCPushButton) );
	settings->setProfileValue( "look/color/descrc", getColor(ui.descrCPushButton) );
	settings->setProfileValue( "look/color/teditbg", getColor(ui.tEditBgPushButton) );
	settings->setProfileValue( "look/color/teditc", getColor(ui.tEditCPushButton) );
	settings->setProfileValue( "look/color/yourbg", getColor(ui.yourBgPushButton) );
	settings->setProfileValue( "look/color/yourc", getColor(ui.yourCPushButton) );
	settings->setProfileValue( "look/color/userbg", getColor(ui.userBgPushButton) );
	settings->setProfileValue( "look/color/userc", getColor(ui.userCPushButton) );

	// APPLY VISUAL CHANGES
	rosterModel->emitLayoutChanged();
	loadSettings();
}

void lookSettings::loadSettings() {
	QPalette tmp=rosterW->palette();
	tmp.setColor( QPalette::Base, settings->profileValue("look/color/rosterbg").value<QColor>() );
	tmp.setColor( QPalette::Text, settings->profileValue("look/color/nickc").value<QColor>() );
	tmp.setColor( QPalette::LinkVisited, settings->profileValue("look/color/descrc").value<QColor>() );
	rosterW->setPalette(tmp);
}

void lookSettings::cancelSettings() {
	QFont tmp;
	// FONTS
	tmp=settings->profileValue("look/font/nick").value<QFont>();
	ui.nickFontLineEdit->setFont( tmp );
	ui.nickFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/descr").value<QFont>();
	ui.descrFontLineEdit->setFont( tmp );
	ui.descrFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/group").value<QFont>();
	ui.groupFontLineEdit->setFont( tmp );
	ui.groupFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/subgroup").value<QFont>();
	ui.subgroupFontLineEdit->setFont( tmp );
	ui.subgroupFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/headermsg").value<QFont>();
	ui.headerMsgFontLineEdit->setFont( tmp );
	ui.headerMsgFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/yourmsg").value<QFont>();
	ui.yourMsgFontLineEdit->setFont( tmp );
	ui.yourMsgFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );

	tmp=settings->profileValue("look/font/usermsg").value<QFont>();
	ui.userMsgFontLineEdit->setFont( tmp );
	ui.userMsgFontLineEdit->setText( QString( "%1, %2" ).arg( tmp.family() ).arg( tmp.pointSize() ) );


	// COLORS
	ui.rosterBgPushButton->setIcon( generateIcon( settings->profileValue("look/color/rosterbg").value<QColor>() ) );
	ui.nickCPushButton->setIcon( generateIcon( settings->profileValue("look/color/nickc").value<QColor>() ) );
	ui.descrCPushButton->setIcon( generateIcon( settings->profileValue("look/color/descrc").value<QColor>() ) );
	ui.tEditBgPushButton->setIcon( generateIcon( settings->profileValue("look/color/teditbg").value<QColor>() ) );
	ui.tEditCPushButton->setIcon( generateIcon( settings->profileValue("look/color/teditc").value<QColor>() ) );
	ui.yourBgPushButton->setIcon( generateIcon( settings->profileValue("look/color/yourbg").value<QColor>() ) );
	ui.yourCPushButton->setIcon( generateIcon( settings->profileValue("look/color/yourc").value<QColor>() ) );
	ui.userBgPushButton->setIcon( generateIcon( settings->profileValue("look/color/userbg").value<QColor>() ) );
	ui.userCPushButton->setIcon( generateIcon( settings->profileValue("look/color/userc").value<QColor>() ) );
}

void lookSettings::pickColor() {
	QPushButton *s=qobject_cast<QPushButton*>(sender());

	QColor newColor=QColorDialog::getColor( getColor(s), this );

	if( newColor.isValid() ) {
		s->setIcon( generateIcon( newColor ) );
	}
}

void lookSettings::pickFont() {
	QPushButton *s=qobject_cast<QPushButton*>(sender());
	QLineEdit *f;
	QString n;
	bool ok;

	if ( s == ui.nickFontPushButton ) {
		f=ui.nickFontLineEdit;
		n="look/font/nick";
	} else if ( s == ui.descrFontPushButton ) {
		f=ui.descrFontLineEdit;
		n="look/font/descr";
	} else if ( s == ui.groupFontPushButton ) {
		f=ui.groupFontLineEdit;
		n="look/font/group";
	} else if ( s == ui.subgroupFontPushButton ) {
		f=ui.subgroupFontLineEdit;
		n="look/font/subgroup";
	} else if ( s == ui.headerMsgFontPushButton ) {
		f=ui.headerMsgFontLineEdit;
		n="look/font/headermsg";
	} else if ( s == ui.yourMsgFontPushButton ) {
		f=ui.yourMsgFontLineEdit;
		n="look/font/yourmsg";
	} else if ( s == ui.userMsgFontPushButton ) {
		f=ui.userMsgFontLineEdit;
		n="look/font/usermsg";
	} else {
		return;
	}

	QFont newFont=QFontDialog::getFont(&ok,settings->profileValue(n).value<QFont>(),settingsDlg);

	if(ok) {
		f->setFont( newFont );
		f->setText( QString( "%1, %2" ).arg( newFont.family() ).arg( newFont.pointSize() ) );
	}
}

QIcon lookSettings::generateIcon( const QColor& color ) {
	QPixmap tmp(50,15);
	tmp.fill(color);
	return QIcon ( tmp );
}

QColor lookSettings::getColor( QPushButton* b ) {
	QIcon ic=b->icon();
	QImage tmp=ic.pixmap(50,15).toImage();
	return QColor( tmp.pixel( 0,1 ) );
}
