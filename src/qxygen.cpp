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

#include <QCoreApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QHttp>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QUrl>
#include <QNetworkProxy>

#include <QDebug>

#include "qxygen.h"
#include "roster_item.h"
#include "roster_widget.h"
#include "useradd.h"
#include "tlen.h"
#include "profileform.h"
#include "settingsdialog.h"
#include "filetransfer.h"

qxygen *Qxygen=0;
rosterWidget *rosterW=0;

qxygen::qxygen( QWidget* parent): QMainWindow( parent ) {
	Qxygen=this;
	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		setupTray();

	setupGui();
	setupRoster();
	setupProtocol();
	setupMenus();
	setupSettings();
}

qxygen::~qxygen() {
}

void qxygen::setupTray() {
	mTrayMenu = new QMenu();
	mTray=new QSystemTrayIcon(QIcon(":offline.png"),this);

	connect(mTray, SIGNAL(activated(int)), this, SLOT(activated(int)));

	mTray->setContextMenu(mTrayMenu);
	mTray->show();

	msgTimer=new QTimer;
	msg=TRUE;

	connect(msgTimer, SIGNAL(timeout()), this, SLOT(swapMsgIcon()));
}

void qxygen::setupGui() {
	ui.setupUi(this);
	statusBar()->setHidden(TRUE);
	setWindowIcon(QIcon(":offline.png"));
	ui.statusButton->setIcon(QIcon(":offline.png"));
	connect(ui.statusButton, SIGNAL(clicked()), this, SLOT(showStatusMenu()));
	ui.menuButton->setIcon(QIcon(":menu.png"));
	ui.menuButton->setFixedSize(ui.menuButton->sizeHint());
	ui.menuButton->setIconSize(ui.menuButton->sizeHint());
	connect(ui.menuButton, SIGNAL(clicked()), this, SLOT(showMainMenu()));
	resize(settings->defaultValue("window/size").value<QSize>());
	move(settings->defaultValue("window/position").value<QPoint>());
}

void qxygen::setupMenus() {
	mainMenu=new QMenu(ui.menuButton);
	statusMenu=new QMenu(ui.statusButton);
	profilesMenu=new QMenu(tr("Profiles"));
	profiles=new QActionGroup(this);
	connect(profiles, SIGNAL(triggered(QAction*)), this, SLOT(choseProfile(QAction*)));
	createProfileAction=new QAction(tr("Add profile"), this);
	connect(createProfileAction, SIGNAL(triggered()), this, SLOT(createProfile()));
	exit=new QAction(QIcon(":quit.png"), tr("Exit program"), this);
	connect(exit, SIGNAL(triggered()), qApp, SLOT(quit()));
	online=new QAction(QIcon(":online.png"), tr("Available"), this);
	online->setData(qxygen::Available);
	connect(online, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	chatty=new QAction(QIcon(":chatty.png"), tr("Free for Chat"), this);
	chatty->setData(qxygen::Chatty);
	connect(chatty, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	away=new QAction(QIcon(":away.png"), tr("Away"), this);
	away->setData(qxygen::Away);
	connect(away, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	unavailable=new QAction(QIcon(":unavailable.png"), tr("Not Available"), this);
	unavailable->setData(qxygen::Unavailable);
	connect(unavailable, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	dnd=new QAction(QIcon(":dnd.png"), tr("Do not Disturb"), this);
	dnd->setData(qxygen::Dnd);
	connect(dnd, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	invisible=new QAction(QIcon(":invisible.png"), tr("Invisible"), this);
	invisible->setData(qxygen::Invisible);
	connect(invisible, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	offline=new QAction(QIcon(":offline.png"), tr("Unavailable"), this);
	offline->setData(qxygen::Offline);
	connect(offline, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	withDescr=new QAction(QIcon(":offlinei.png"), tr("With description"), this);
	connect(withDescr, SIGNAL(triggered()), this, SLOT(setDescrDialog()));
	add=new QAction(QIcon(":add.png"), tr("Add contact"), this);
	connect(add,SIGNAL(triggered()),this,SLOT(addUser()));
	remove=new QAction(QIcon(":remove.png"), tr("Remove contact"), this);
	connect(remove, SIGNAL(triggered()), this, SLOT(parseRemove()));
	sendFile=new QAction(tr("Send file"), this);
	connect( sendFile, SIGNAL( triggered() ), this, SLOT( fileSendDialog() ) );
	settingsDialogA = new QAction(tr("Settings"), this);
	connect(settingsDialogA, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
//	rosterMenu->addAction(sendFile);
	rosterMenu->addAction(remove);
	mTrayMenu->addAction(online);
	mTrayMenu->addAction(chatty);
	mTrayMenu->addAction(away);
	mTrayMenu->addAction(unavailable);
	mTrayMenu->addAction(dnd);
	mTrayMenu->addAction(invisible);
	mTrayMenu->addAction(offline);
	mTrayMenu->addAction(withDescr);
	mTrayMenu->addSeparator();
	mTrayMenu->addAction(exit);
	mainMenu->addMenu(profilesMenu);
	mainMenu->addAction(add);
	mainMenu->addSeparator();
	mainMenu->addAction(settingsDialogA);
	mainMenu->addSeparator();
	mainMenu->addAction(exit);
	statusMenu->addAction(online);
	statusMenu->addAction(chatty);
	statusMenu->addAction(away);
	statusMenu->addAction(unavailable);
	statusMenu->addAction(dnd);
	statusMenu->addAction(invisible);
	statusMenu->addAction(offline);
	statusMenu->addSeparator();
	statusMenu->addAction(withDescr);
}

void qxygen::setupRoster() {
	rosterMenu=new QMenu(ui.rosterView);
	rosterModel=new rosterView();
	rosterW=ui.rosterView;
	ui.rosterView->setModel(rosterModel);
	delegate=new rosterDelegate();
	ui.rosterView->setItemDelegate(delegate);
	ui.rosterView->setMenu(rosterMenu);
	connect(ui.rosterView, SIGNAL(expanded ( const QModelIndex & )), this, SLOT(expandItem( const QModelIndex & )));
	connect(ui.rosterView, SIGNAL(collapsed ( const QModelIndex & )), this, SLOT(collapseItem( const QModelIndex & )));
	connect(rosterModel, SIGNAL(windowUpdate(QString,QIcon)), this, SLOT(windowUpdate(QString,QIcon)));
	connect(rosterModel, SIGNAL(groupAdded(const QModelIndex &)), ui.rosterView, SLOT(expand(const QModelIndex &)));
	connect(ui.rosterView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(showChatWindow(const QModelIndex&)));
}

void qxygen::setupProtocol() {
	Tlen = new tlen(this);
	connect(Tlen, SIGNAL(statusChanged()), this, SLOT(statusChange()));
	connect(Tlen, SIGNAL(clearRosterView()), rosterModel, SLOT(clearRoster()));
	connect(Tlen, SIGNAL(itemReceived(QString, QString, QString, QString,bool)), rosterModel, SLOT(addItem(QString, QString, QString, QString,bool)));
	connect(Tlen, SIGNAL(presenceChanged(QString, QString, QString)), rosterModel, SLOT(presenceUpdate(QString, QString, QString)));
	connect(Tlen, SIGNAL(presenceDisconnected()), rosterModel, SLOT(presenceDisconnected()));
	connect(Tlen, SIGNAL(sortRoster()), rosterModel, SLOT(sortRoster()));
	connect(Tlen, SIGNAL(authorizationAsk(QString)), this, SLOT(authorizationAsk(QString)));
	connect(this, SIGNAL(authorize(QString,bool)), Tlen, SLOT(authorize(QString,bool)));
	connect(Tlen, SIGNAL(removeItem(QString)), rosterModel, SLOT(removeItem(QString)));
	connect(Tlen, SIGNAL(chatMsgReceived(QDomNode)), this, SLOT(chatMsgReceived(QDomNode)));
	connect(Tlen, SIGNAL(chatNotify(QString,QString)), this, SLOT(chatNotify(QString,QString)));
}

void qxygen::setupSettings() {
	connect(settings, SIGNAL(noProfile()), this, SLOT(createProfile()));
	connect(settings, SIGNAL(loadProfile()), this, SLOT(loadProfile()));
	settings->initModule();
	updateProfilesMenu();
}

void qxygen::toggleVisibility() {
	if(isHidden()) {
		show();
		if(isMinimized()) {
			if(isMaximized())
				showMaximized();
			else
				showNormal();
		}
		resize(settings->defaultValue("window/size").value<QSize>());
		move(settings->defaultValue("window/position").value<QPoint>());
		raise();
		activateWindow();
	}
	else {
		settings->setDefaultValue("window/size", QVariant(size()));
		settings->setDefaultValue("window/position", QVariant(pos()));
		hide();
	}
}

void qxygen::expandItem( const QModelIndex &index ) {
	delegate->setWidth(ui.rosterView->viewport()->width());
	rosterModel->setData(index, QVariant(TRUE), rosterView::ExpandRole);
}

void qxygen::collapseItem( const QModelIndex &index ) {
	delegate->setWidth(ui.rosterView->viewport()->width());
	rosterModel->setData(index, QVariant(FALSE), rosterView::ExpandRole);
}

void qxygen::showMainMenu() {
	mainMenu->popup(QPoint(mapToGlobal(ui.menuButton->pos()).x(),mapToGlobal(ui.menuButton->pos()).y()-mainMenu->sizeHint().height()));
}

void qxygen::showStatusMenu() {
	statusMenu->popup(QPoint(mapToGlobal(ui.statusButton->pos()).x()+ui.statusButton->width()-statusMenu->sizeHint().width(),mapToGlobal(ui.statusButton->pos()).y()-statusMenu->sizeHint().height()));
}

void qxygen::statusChange() {
	QString status=Tlen->strStatus();
	bool descr=(Tlen->description()).isEmpty();
	QIcon px;

	if(status=="available")
		px=descr?QIcon(":online.png"):QIcon(":onlinei.png");
	else if(status=="chat" || status=="chatty")
		px=descr?QIcon(":chatty.png"):QIcon(":chattyi.png");
	else if(status=="away")
		px=descr?QIcon(":away.png"):QIcon(":awayi.png");
	else if(status=="xa")
		px=descr?QIcon(":unavailable.png"):QIcon(":unavailablei.png");
	else if(status=="dnd")
		px=descr?QIcon(":dnd.png"):QIcon(":dndi.png");
	else if(status=="invisible")
		px=descr?QIcon(":invisible.png"):QIcon(":invisiblei.png");
	else
		px=descr?QIcon(":offline.png"):QIcon(":offlinei.png");

	ui.statusButton->setIcon( px );
	setWindowIcon( px );
	mTray->setIcon( px );
}

void qxygen::authorizationAsk( QString from ) {
	if( rosterItem *item = rosterModel->find(from) ) {
		if( item->data(3)=="subscribed" || item->data(3)=="both" )
			return;
	}

	if(QMessageBox::question(this,
				tr("User authorization"),
				tr("User %1 requested authorization from You."
				"Do you want to authorize him?")
				.arg(from),
				tr("&Yes"), tr("&No"),
				QString(), 2, 1))
		emit authorize(from,FALSE);
	else
		emit authorize(from,TRUE);
}

void qxygen::addUser() {
	if(!Tlen->isConnected())
		return;

	useradd *addDlg=new useradd(rosterModel->groupNames(),this);
	connect(addDlg, SIGNAL(addItem(QString,QString,QString,bool)), Tlen, SLOT(addItem(QString,QString,QString,bool)));
	addDlg->exec();
}

void qxygen::parseRemove() {
	QModelIndex idx=ui.rosterView->currentIndex();

	if(idx.isValid())
		Tlen->remove(idx.model()->data(idx, rosterView::JidRole).toString());
}

void qxygen::chatMsgReceived( QDomNode n ) {
	QDomElement msg=n.toElement();
	QString from=msg.attribute("from");
	QString body;
	QString timeStamp;

	QDomNodeList nl=msg.childNodes();
	for(int i=0; i<nl.count();++i) {
		QDomNode tmp=nl.item(i);
		if(tmp.nodeName()=="body")
			body=tmp.firstChild().toText().data();

		if(tmp.nodeName()=="x") {
			QDomElement e=tmp.toElement();
			if(e.hasAttribute("xmlns") && e.attribute("xmlns")=="jabber:x:delay") {
				QDateTime dt(QDateTime::fromString(e.attribute("stamp"), "yyyyMMdd'T'hh:mm:ss"));
				dt.setTimeSpec(Qt::UTC);
				timeStamp=dt.toLocalTime().toString("dd.MM.yyyy hh:mm:ss");
			}
		}
	}

	if(timeStamp.isEmpty())
		timeStamp=QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

	if(chatWindow *w=chatMap[from]) {
		if(!w->isHidden()) {
			w->displayMsg(Tlen->decode(body.toUtf8()),timeStamp);
			return;
		}
	}

	jidMsgOrderList.removeAll(from);
	jidMsgOrderList.prepend(from);

	msgMap[from].append(body+'\t'+timeStamp);
	msgTimer->start(500);
}

void qxygen::showChatWindow(const QModelIndex &index)
{
	if(index.model()->data(index,rosterView::TypeRole)==0) {
		QString jid=index.model()->data(index,rosterView::JidRole).toString();
		if(chatWindow *w=chatMap[jid]) {
			w->show();
			if(w->isMinimized()) {
				if(w->isMaximized())
					w->showMaximized();
				else
					w->showNormal();
			}
			w->raise();
			w->activateWindow();
		}
		else {
			createChatWindow( index.model()->data(index,Qt::DisplayRole).toString(),jid, QIcon(QPixmap::fromImage(index.model()->data(index,Qt::DecorationRole).value<QImage>())) );
		}
	}
}

void qxygen::openMsg(QString from)
{
	if(chatWindow *w=chatMap[from]) {
		w->show();

		QStringListIterator it(msgMap[from]);
		while(it.hasNext()) {
			QString line=it.next();
			QStringList msg=line.split("\t");
			w->displayMsg(Tlen->decode(msg[0].toUtf8()), msg[1]);
		}
		msgMap[from].clear();

		if(w->isMinimized()) {
			if(w->isMaximized())
				w->showMaximized();
			else
				w->showNormal();
		}
		w->raise();
		w->activateWindow();
	}
	else {
		rosterItem *item=rosterModel->find(from);
		createChatWindow(item->data(0),from,QIcon(QPixmap::fromImage(item->icon())));
	}
}

void qxygen::createChatWindow(const QString& label, const QString& jid, const QIcon& icon) {
	chatWindow *w=new chatWindow(label,jid,0);
	w->setWindowIcon( icon );
	chatMap.insert(jid,w);
	QStringListIterator it(msgMap[jid]);
	while(it.hasNext()) {
		QString line=it.next();
		QStringList msg=line.split("\t");
		w->displayMsg(Tlen->decode(msg[0].toUtf8()), msg[1]);
	}
	msgMap[jid].clear();
	connect(w, SIGNAL(writeMsg(QString,QString)), Tlen, SLOT(writeMsg(QString,QString)));
	connect(w, SIGNAL(chatNotify(QString, bool)), Tlen, SLOT(chatNotify(QString,bool)));
	connect(settingsDlg, SIGNAL(updateSettings()), w, SLOT(updateSettings()));
}

void qxygen::windowUpdate(QString owner, QIcon ico)
{
	if(chatWindow *w=chatMap[owner])
		w->setWindowIcon(ico);
}

void qxygen::createProfile()
{
	profileForm *form=new profileForm(this);
	connect(form, SIGNAL(addProfile(QString,QString,QString)), this, SLOT(addProfile(QString,QString,QString)));
	form->exec();
}

void qxygen::addProfile(QString profile,QString login,QString pass)
{
	settings->addProfile(profile,login,pass);
	updateProfilesMenu();
	loadProfile();
}

void qxygen::updateProfilesMenu()
{
	profilesMenu->clear();
	QStringList profileNames=settings->defaultValue("profiles/list").value<QStringList>();

	profiles->setExclusive(TRUE);

	QStringListIterator it(profileNames);
	while(it.hasNext()) {
		QString name=it.next();
		QAction *a=new QAction(name,this);
		a->setCheckable(TRUE);
		if(name==settings->profileValue("user/profile").toString())
			a->setChecked(TRUE);
		profiles->addAction(a);
		profilesMenu->addAction(a);
	}
	if(profileNames.count())
		profilesMenu->addSeparator();
	profilesMenu->addAction(createProfileAction);
}

void qxygen::loadProfile() {
	if( Tlen->isConnected() )
		Tlen->closeConn();

	Tlen->setPass( settings->profileValue("user/pass").toString() );
	Tlen->setUname( settings->profileValue("user/login").toString() );
	setWindowTitle( "Qxygen: "+settings->profileValue("user/profile").toString() );
	rosterModel->clearRoster();
	if(settingsDlg) {
		delete settingsDlg;
	}

	new settingsDialog(this);
	settingsDlg->loadSettings();
	
	chatMap.clear();
	msgMap.clear();
	jidMsgOrderList.clear();
	msgTimer->stop();
	if(!msg)
		swapMsgIcon();

	descrDlg=new descrDialog(Tlen->strStatus(), this);
	connect(descrDlg, SIGNAL(statusChanged(QString, QString)), Tlen, SLOT(setStatusDescr(QString, QString)));
}

void qxygen::choseProfile ( QAction* a ) {
	settings->choseProfile( a->text() );
	loadProfile();
}

void qxygen::chatNotify(QString to, QString type) {
	if(chatWindow *w=chatMap[to]) {
		if(type=="t")
			w->typingNotify(TRUE);
		else if(type=="u")
			w->typingNotify(FALSE);
	}
	//<m tp='a' f='qxygen@tlen.pl'/> - user qxygen@tlen.pl sent sound alarm
}

void qxygen::setDescrDialog() {
	descrDlg->show();
}

void qxygen::showSettingsDialog() {
	settingsDlg->show();
}

void qxygen::fileSendDialog() {
	QModelIndex idx=ui.rosterView->currentIndex();

	if(idx.isValid()) {
		QString jid = idx.model()->data(idx, rosterView::JidRole).toString();
		jid.remove("@tlen.pl");
		int id=rand();
		while( Tlen->fTransferMap.contains( QString("%1-%2").arg(jid).arg(id) ) )
			++id;

		fileTransferDialog *dlg = new fileTransferDialog( id, jid );
		Tlen->fTransferMap.insert( QString("%1-%2").arg(jid).arg(id), dlg );
		dlg->show();
	}
}
void qxygen::activated(int r) {
	if ( jidMsgOrderList.count() ) {
		openMsg( jidMsgOrderList[0] );
		jidMsgOrderList.removeAll( jidMsgOrderList[0] );
		if(!jidMsgOrderList.count()) {
			msgTimer->stop();
			if(!msg)
				swapMsgIcon();
		}
	}
	else if ( r == QSystemTrayIcon::DoubleClick || r == QSystemTrayIcon::Trigger )
		toggleVisibility();
}

void qxygen::swapMsgIcon() {
	msg?mTray->setIcon(QPixmap(":msg.png")):mTray->setIcon(windowIcon());
	msg=!msg;
}

rosterWidget* qxygen::roster() {
	return ui.rosterView;
}
