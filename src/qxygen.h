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

#ifndef QXYGEN_H
#define QXYGEN_H

#define VERSION 0.0.0_20060407

#include <QDomNode>

#include "ui_qxygen.h"

class QMenu;

class TrayIcon;
class rosterView;
class rosterDelegate;
class tlen;
class chatWindow;
class settingsMenager;

class qxygen: public QMainWindow
{
Q_OBJECT

public:
	enum {Available=0, Chatty=1, Away=2, Unavailable=3, Dnd=4, Invisible=5, Offline=6};
	qxygen(QWidget *parent=0, QString title="Qxygen");
	~qxygen();

public slots:
	void parseRemove();
	void collapseItem(const QModelIndex &index);
	void expandItem(const QModelIndex &index);
	void toggleVisibility();
	void slotTrayClosed();
	void showMainMenu();
	void showStatusMenu();
	void statusChange();
	void authorizationAsk(QString);
	void addUser();

	void chatMsgReceived(QDomNode);

	void windowUpdate(QString,QIcon);

	void showChatWindow(const QModelIndex&);
	void openMsg(QString);

	void createProfile();
	void addProfile(QString,QString,QString);
	void updateProfilesMenu();
	void loadProfile();
	void choseProfile(QAction*);
signals:
	void authorize(QString, bool);
	void windowOpened(QString);

protected:
	void queueMsg(QString,QString,QString);
	void setupModel();
	void setupTray();
	void setupMenus();
	void closeEvent(QCloseEvent *e);

private:
	QMap<QString /*jid*/, chatWindow*> chatMap;
	QMap<QString /*jid*/, QStringList /*msg queue*/> msgMap;

	settingsMenager *settings;

	QMenu	*mTrayMenu,
		*mainMenu,
		*statusMenu,
		*rosterMenu,
		*profilesMenu;

	QActionGroup *profiles;

	QAction *exit,
		*online,
		*chatty,
		*away,
		*unavailable,
		*dnd,
		*invisible,
		*offline,
		*add,
		*remove,
		*createProfileAction;

	tlen *Tlen;
	rosterView *rosterModel;
	rosterDelegate *delegate;
	TrayIcon *mTray;
	Ui::Qxygen ui;
};
#endif
