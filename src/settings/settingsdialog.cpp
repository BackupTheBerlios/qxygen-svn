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
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QNetworkProxy>
#include <QDir>
#include <QMessageBox>
#include <QTcpServer>

#include <QDebug>

#include "settingsdialog.h"
#include "settings.h"
#include "roster_view.h"

settingsDialog *settingsDlg=0;

settingsDialog::settingsDialog(QWidget *parent): QDialog(parent) {
	setWindowTitle(tr("Qxygen: settings"));
	setWindowIcon(QIcon(":logo.png"));
	QGridLayout *main = new QGridLayout(this);
	QHBoxLayout *top = new QHBoxLayout();
	QHBoxLayout *bottom = new QHBoxLayout();
	widgetScroll=new QScrollArea();
	widgetScroll->setLayout( new QHBoxLayout() );
	widgetScroll->setWidgetResizable(TRUE);
	tabs = new QListWidget();
	tabs->setIconSize(QSize(32,32));
	tabs->setMovement(QListView::Static);
	tabs->setFlow(QListView::TopToBottom);
	tabs->setFixedWidth(150);

	ok=new QPushButton(tr("Ok"));
	apply=new QPushButton(tr("Apply"));
	cancel=new QPushButton(tr("Cancel"));

	bottom->addStretch();
	bottom->addWidget(ok);
	bottom->addWidget(apply);
	bottom->addWidget(cancel);

	generalSettings *generalS=new generalSettings();
	tabs->addItem( generalS->settingsTab() );
	networkSettings *networkS=new networkSettings();
	tabs->addItem( networkS->settingsTab() );

	top->addWidget(tabs);
	top->addWidget(widgetScroll);

	main->addLayout(top, 0, 0);
	main->addLayout(bottom, 1, 0);

//	QListWidgetItem *firstItem = tabs->item(0);
//	tabs->setItemSelected(firstItem, TRUE);
//	widgetScroll->setWidget( firstItem->data(settingsWidget::widgetRole).value<QWidget*>() );

	connect( tabs, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*) ), this, SLOT( swapSettingsWidget(QListWidgetItem*,QListWidgetItem*) ) );
	connect( cancel, SIGNAL( clicked() ), this, SLOT( cancelSettings() ) );
	connect( ok, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
	connect( apply, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
}

void settingsDialog::swapSettingsWidget(QListWidgetItem *curr, QListWidgetItem *prev) {
	if(prev)
		widgetScroll->takeWidget();

	widgetScroll->setWidget( curr->data(settingsWidget::widgetRole).value<QWidget*>() );
	curr->data(settingsWidget::widgetRole).value<QWidget*>()->show();
}

void settingsDialog::saveSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>(tabs->item(i)->data(settingsWidget::widgetRole).value<QWidget*>() )->saveSettings();
	}
	QPushButton *b=qobject_cast<QPushButton*>(sender());
	if(b==ok)
		close();
}

void settingsDialog::loadSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>(tabs->item(i)->data(settingsWidget::widgetRole).value<QWidget*>() )->loadSettings();
	}
}

void settingsDialog::cancelSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>(tabs->item(i)->data(settingsWidget::widgetRole).value<QWidget*>() )->cancelSettings();
	}
	close();
}

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

generalSettings::generalSettings(QWidget *parent): settingsWidget(parent) {
	ui.setupUi(this);
	cancelSettings();
	item=new QListWidgetItem( QIcon(":logo.png"), tr("General") );
	QVariant V;
	V.setValue<QWidget*>( this );
	item->setData( widgetRole, V );
}

void generalSettings::saveSettings() {
	settings->setProfileValue( "roster/showGroups", ui.showGroups->isChecked() );
	settings->setProfileValue( "roster/showSubgroups", ui.showSubgroups->isChecked() );
	settings->setProfileValue( "roster/showDescription", ui.showDescription->isChecked() );
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
