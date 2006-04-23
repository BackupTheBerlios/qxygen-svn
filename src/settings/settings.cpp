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

#include <QDir>
#include <QSettings>

#include "settings.h"

settingsMenager *settings=new settingsMenager();

settingsMenager::settingsMenager( QObject* parent ):QObject( parent ) {
#ifdef Q_WS_WIN
	dir=new QDir(QDir::homePath()+"/qxygen");
#else
	dir=new QDir(QDir::homePath()+"/.qxygen");
#endif

	if(!dir->exists())
		dir->mkdir(dir->path());

	if(!dir->exists(dir->path()+"/profiles"))
		dir->mkdir(dir->path()+"/profiles");
#ifdef Q_WS_WIN
	qxygen=new QSettings(dir->path()+"/qxygen",QSettings::IniFormat);
#else
	qxygen=new QSettings(dir->path()+"/qxygen",QSettings::NativeFormat);
#endif
}

settingsMenager::~settingsMenager() {
}

void settingsMenager::initModule() {
	if(qxygen->value("profiles/default").toString().isEmpty())
		emit noProfile();
	else
	{
#ifdef Q_WS_WIN
		profile=new QSettings(dir->path()+"/profiles/"+qxygen->value("profiles/default").toString(), QSettings::IniFormat);
#else
		profile=new QSettings(dir->path()+"/profiles/"+qxygen->value("profiles/default").toString(), QSettings::NativeFormat);
#endif
		emit loadProfile();
	}
}

void settingsMenager::addProfile(QString name, QString login, QString pass) {
#ifdef Q_WS_WIN
	profile=new QSettings(dir->path()+"/profiles/"+name, QSettings::IniFormat);
#else
	profile=new QSettings(dir->path()+"/profiles/"+name, QSettings::NativeFormat);
#endif
	qxygen->setValue("profiles/default", name);
	QStringList profilesList=qxygen->value("profiles/list").value<QStringList>();
	profilesList<<name;
	qxygen->setValue("profiles/list", profilesList);
	profile->setValue("user/profile", name);
	profile->setValue("user/login",login);
	profile->setValue("user/pass", pass);
}

void settingsMenager::choseProfile(QString p) {
	qxygen->setValue("profiles/default", p);
#ifdef Q_WS_WIN
	profile=new QSettings(dir->path()+"/profiles/"+qxygen->value("profiles/default").toString(), QSettings::IniFormat);
#else
	profile=new QSettings(dir->path()+"/profiles/"+qxygen->value("profiles/default").toString(), QSettings::NativeFormat);
#endif
}

void settingsMenager::setProfileValue(QString key, QVariant value) {
	profile->setValue(key,value);
}

void settingsMenager::setDefaultValue(QString key, QVariant value) {
	qxygen->setValue(key, value);
}

QVariant settingsMenager::profileValue(QString key) {
	return profile->value(key);
}

QVariant settingsMenager::defaultValue(QString key) {
	return qxygen->value(key);
}
