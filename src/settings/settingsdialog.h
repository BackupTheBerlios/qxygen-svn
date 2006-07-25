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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>

#include "settingswidget.h"
#include "ui_networkswidget.h"
#include "ui_generalswidget.h"

class settingsDialog: public QDialog
{
Q_OBJECT
public:
	settingsDialog(QWidget *parent=0);
	
public slots:
	void loadSettings();
	void saveSettings();
	void cancelSettings();

private slots:
	void swapSettingsWidget(QListWidgetItem*,QListWidgetItem*);

private:
	QListWidget *tabs;
	QScrollArea *widgetScroll;
	QPushButton *ok, *apply, *cancel;
};

class generalSettings: public settingsWidget
{
Q_OBJECT
public:
	generalSettings(QWidget *parent=0);
	void saveSettings();
	void loadSettings();
	void cancelSettings();
	QListWidgetItem *settingsTab(){return item;}
private:
	QListWidgetItem *item;
	Ui::generalSWidget ui;
};

class networkSettings: public settingsWidget
{
Q_OBJECT
public:
	networkSettings(QWidget *parent=0);
	void saveSettings();
	void loadSettings();
	void cancelSettings();
	QListWidgetItem *settingsTab(){return item;}

public slots:
	void testPort();

private:
	QListWidgetItem *item;
	Ui::networkSWidget ui;
};

extern settingsDialog *settingsDlg;
#endif
