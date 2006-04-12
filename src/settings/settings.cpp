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

settingsMenager::settingsMenager(QObject *parent):QObject(parent)
{
}

settingsMenager::~settingsMenager()
{
}

void settingsMenager::initModule()
{
#ifdef Q_WS_WIN
	dir=new QDir(QDir::homePath()+"\\qxygen");
#else
	dir=new QDir(QDir::homePath()+"/.qxygen");
#endif

	if(!dir->exists())
		dir->mkdir(dir->path());

	qxygen=new QSettings(dir->path()+"/qxygen",QSettings::NativeFormat);
	if(qxygen->value("profiles/default").toString().isEmpty())
		emit noProfile();
	else
		profile=new QSettings(dir->path()+"/"+qxygen->value("profiles/default").toString(), QSettings::NativeFormat);
}

void settingsMenager::addProfile(QString name, QString login, QString pass)
{
	profile=new QSettings(dir->path()+"/"+name, QSettings::NativeFormat);
	qxygen->setValue("profiles/default", name);
	QStringList profilesList=qxygen->value("profiles/list").value<QStringList>();
	profilesList<<name;
	qxygen->setValue("profiles/list", profilesList);
	profile->setValue("user/profile", name);
	profile->setValue("user/login",login);
	profile->setValue("user/pass", pass);
}

QStringList settingsMenager::profilesList()
{
	if(qxygen->contains("profiles/list"))
		return qxygen->value("profiles/list").value<QStringList>();
	else
		return QStringList();
}

QString settingsMenager::user()
{
	return profile->value("user/login").toString();
}

QString settingsMenager::pass()
{
	return profile->value("user/pass").toString();
}

QString settingsMenager::profileName()
{
	return profile->value("user/profile").toString();
}

void settingsMenager::choseProfile(QString p)
{
	qxygen->setValue("profiles/default", p);
	profile=new QSettings(dir->path()+"/"+qxygen->value("profiles/default").toString(), QSettings::NativeFormat);
}
