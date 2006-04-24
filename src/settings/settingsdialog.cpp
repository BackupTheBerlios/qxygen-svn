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

#include <QDebug>

#include "settingsdialog.h"
#include "settings.h"

settingsDialog *settingsDlg=0;

settingsDialog::settingsDialog(QWidget *parent): QDialog(parent) {
	setWindowTitle(tr("Qxygen: settings"));
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

	//CREATE SETTINGS SECTIONS
	QListWidgetItem *networkS = new QListWidgetItem( QIcon(":network.png"), tr("Network") );
	networkSettings *networkWidget = new networkSettings();
	QVariant networkV;
	networkV.setValue<QWidget*>( networkWidget );
	networkS->setData( widgetRole, networkV );
	tabs->addItem(networkS);

	top->addWidget(tabs);
	top->addWidget(widgetScroll);

	main->addLayout(top, 0, 0);
	main->addLayout(bottom, 1, 0);

	QListWidgetItem *firstItem = tabs->item(0);
	tabs->setItemSelected(firstItem, TRUE);
	widgetScroll->setWidget( firstItem->data(widgetRole).value<QWidget*>() );

	connect( tabs, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*) ), this, SLOT( swapSettingsWidget(QListWidgetItem*,QListWidgetItem*) ) );
	connect( cancel, SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( ok, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
	connect( apply, SIGNAL( clicked() ), this, SLOT( saveSettings() ) );
}

void settingsDialog::swapSettingsWidget(QListWidgetItem *curr, QListWidgetItem*) {
	widgetScroll->setWidget( curr->data(widgetRole).value<QWidget*>() );
}

void settingsDialog::saveSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>(tabs->item(i)->data(widgetRole).value<QWidget*>() )->saveSettings();
	}
	QPushButton *b=qobject_cast<QPushButton*>(sender());
	if(b==ok)
		close();
}

void settingsDialog::loadSettings() {
	for(int i=0; i<tabs->count(); ++i) {
		static_cast<settingsWidget*>(tabs->item(i)->data(widgetRole).value<QWidget*>() )->loadSettings();
	}
}

networkSettings::networkSettings(QWidget *parent): settingsWidget(parent) {
	ui.setupUi(this);
	if( settings->profileValue("network/useproxy").toBool() ) {
		ui.useProxyCheckBox->setChecked(TRUE);
		ui.groupBox->setEnabled(TRUE);
	}
	ui.hostLineEdit->setText( settings->profileValue("network/proxy/host").toString() );
	ui.portLineEdit->setText( settings->profileValue("network/proxy/port").toString() );
	ui.userLineEdit->setText( settings->profileValue("network/proxy/username").toString() );
	ui.passLineEdit->setText( settings->profileValue("network/proxy/password").toString() );
}

void networkSettings::saveSettings() {
	settings->setProfileValue( "network/useproxy", QVariant( ui.useProxyCheckBox->isChecked() ) );
	settings->setProfileValue( "network/proxy/host", ui.hostLineEdit->text() );
	settings->setProfileValue( "network/proxy/port", ui.portLineEdit->text() );
	settings->setProfileValue( "network/proxy/username", ui.userLineEdit->text() );
	settings->setProfileValue( "network/proxy/password", ui.passLineEdit->text() );

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
