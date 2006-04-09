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

#include "qxygen.h"
#include "roster_view.h"
#include "roster_delegate.h"
#include "roster_item.h"
#include "trayicon.h"
#include "tlen.h"
#include "useradd.h"
#include "chatwindow.h"

qxygen::qxygen(QWidget *parent, QString title): QMainWindow(parent)
{
	ui.setupUi(this);

	statusBar()->setHidden(TRUE);

	setupMenus();

	setupTray();

	setupModel();

	setWindowIcon(QIcon(":offline.png"));
	setWindowTitle(title);

	ui.statusButton->setIcon(QIcon(":offline.png"));
	ui.menuButton->setIcon(QIcon(":menu.png"));

	Tlen=new tlen(this);

	connect(ui.menuButton, SIGNAL(clicked()), this, SLOT(showMainMenu()));
	connect(ui.statusButton, SIGNAL(clicked()), this, SLOT(showStatusMenu()));

	connect(ui.rosterView, SIGNAL(expanded ( const QModelIndex & )), this, SLOT(expandItem( const QModelIndex & )));
	connect(ui.rosterView, SIGNAL(collapsed ( const QModelIndex & )), this, SLOT(collapseItem( const QModelIndex & )));

	// TLEN PROTOCOL AND GUI
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
	connect(rosterModel, SIGNAL(windowUpdate(QString,QIcon)), this, SLOT(windowUpdate(QString,QIcon)));

	connect(online, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(chatty, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(away, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(unavailable, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(dnd, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(invisible, SIGNAL(triggered()), Tlen, SLOT(setStatus()));
	connect(offline, SIGNAL(triggered()), Tlen, SLOT(setStatus()));

	connect(ui.rosterView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(showChatWindow(const QModelIndex&)));

	// ACTIONS
	connect(exit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(add,SIGNAL(triggered()),this,SLOT(addUser()));
	connect(remove, SIGNAL(triggered()), this, SLOT(parseRemove()));

	connect(rosterModel, SIGNAL(groupAdded(const QModelIndex &)), ui.rosterView, SLOT(expand(const QModelIndex &)));

	connect(mTray, SIGNAL(openMsg(QString)), this, SLOT(openMsg(QString)));
}

qxygen::~qxygen()
{
	if(Tlen)
		delete Tlen;
	if(delegate)
		delete delegate;
	if(rosterModel)
		delete rosterModel;
}

void qxygen::setupTray()
{
	qApp->setApplicationName("Qxygen");
	mTray = new TrayIcon(QPixmap(":offline.png"), "Qxygen", 0, this);
	mTray->setWMDock(false);
	mTray->setPopup(mTrayMenu);

	connect(mTray, SIGNAL(clicked(const QPoint &, int)), SLOT(toggleVisibility()));
	connect(mTray, SIGNAL(closed()), SLOT(slotTrayClosed()));
//	connect(qApp, SIGNAL(newTrayOwner()), mTray, SLOT(newTrayOwner())); // Used on X11
//	connect(qApp, SIGNAL(trayOwnerDied()), mTray, SLOT(hide())); // Used on X11

	mTray->show();
}


void qxygen::slotTrayClosed()
{
	if(isHidden())
		show();
}


void qxygen::toggleVisibility()
{
	if(isHidden())
	{
		show();
		if(isMinimized())
		{
			if(isMaximized())
				showMaximized();
			else
				showNormal();
		}
		raise();
		activateWindow();
	}
	else
	{
		hide();
	}
}


void qxygen::closeEvent(QCloseEvent *e)
{
#ifdef Q_WS_MAC
	hide();
	e->accept();
#else
	if (!mTray)
		qApp->quit();

	toggleVisibility();
	e->ignore();
#endif
}

void qxygen::expandItem( const QModelIndex &index )
{
	delegate->setWidth(ui.rosterView->viewport()->width());
	rosterModel->setData(index, QVariant(TRUE), rosterView::ExpandRole);
}

void qxygen::collapseItem( const QModelIndex &index )
{
	delegate->setWidth(ui.rosterView->viewport()->width());
	rosterModel->setData(index, QVariant(FALSE), rosterView::ExpandRole);
}

void qxygen::setupModel()
{
	rosterModel=new rosterView();
	ui.rosterView->setModel(rosterModel);

	delegate=new rosterDelegate();
	ui.rosterView->setItemDelegate(delegate);

	ui.rosterView->setMenu(rosterMenu);
}

void qxygen::showMainMenu()
{
	mainMenu->popup(QPoint(mapToGlobal(ui.menuButton->pos()).x(),mapToGlobal(ui.menuButton->pos()).y()-mainMenu->sizeHint().height()));
}

void qxygen::showStatusMenu()
{
	statusMenu->popup(QPoint(mapToGlobal(ui.statusButton->pos()).x()+ui.statusButton->width()-statusMenu->sizeHint().width(),mapToGlobal(ui.statusButton->pos()).y()-statusMenu->sizeHint().height()));
}

void qxygen::setupMenus()
{
	mTrayMenu = new QMenu();
	mainMenu=new QMenu(ui.menuButton);
	statusMenu=new QMenu(ui.statusButton);
	rosterMenu=new QMenu(ui.rosterView);

	exit=new QAction(QIcon(":quit.png"), tr("Exit program"), this);

	online=new QAction(QIcon(":online.png"), tr("Available"), this);
	online->setData(qxygen::Available);

	chatty=new QAction(QIcon(":chatty.png"), tr("Free for Chat"), this);
	chatty->setData(qxygen::Chatty);

	away=new QAction(QIcon(":away.png"), tr("Away"), this);
	away->setData(qxygen::Away);

	unavailable=new QAction(QIcon(":unavailable.png"), tr("Not Available"), this);
	unavailable->setData(qxygen::Unavailable);

	dnd=new QAction(QIcon(":dnd.png"), tr("Do not Disturb"), this);
	dnd->setData(qxygen::Dnd);

	invisible=new QAction(QIcon(":invisible.png"), tr("Invisible"), this);
	invisible->setData(qxygen::Invisible);

	offline=new QAction(QIcon(":offline.png"), tr("Unavailable"), this);
	offline->setData(qxygen::Offline);

	add=new QAction(QIcon(":add.png"), tr("Add contact"), this);
	remove=new QAction(QIcon(":remove.png"), tr("Remove contact"), this);

	rosterMenu->addAction(remove);

	mTrayMenu->addAction(online);
	mTrayMenu->addAction(chatty);
	mTrayMenu->addAction(away);
	mTrayMenu->addAction(unavailable);
	mTrayMenu->addAction(dnd);
	mTrayMenu->addAction(invisible);
	mTrayMenu->addAction(offline);
	mTrayMenu->addSeparator();
	mTrayMenu->addAction(exit);

	mainMenu->addAction(add);
	mainMenu->addSeparator();
	mainMenu->addAction(exit);

	statusMenu->addAction(online);
	statusMenu->addAction(chatty);
	statusMenu->addAction(away);
	statusMenu->addAction(unavailable);
	statusMenu->addAction(dnd);
	statusMenu->addAction(invisible);
	statusMenu->addAction(offline);
}

void qxygen::statusChange()
{
	QString status=Tlen->strStatus();
	bool descr=(Tlen->description()).isEmpty();
	QPixmap px;

	if(status=="available")
		px=descr?QPixmap(":online.png"):QPixmap(":onlinei.png");
	else if(status=="chat")
		px=descr?QPixmap(":chatty.png"):QPixmap(":chattyi.png");
	else if(status=="away")
		px=descr?QPixmap(":away.png"):QPixmap(":awayi.png");
	else if(status=="xa")
		px=descr?QPixmap(":unavailable.png"):QPixmap(":unavailablei.png");
	else if(status=="dnd")
		px=descr?QPixmap(":dnd.png"):QPixmap(":dndi.png");
	else if(status=="invisible")
		px=descr?QPixmap(":invisible.png"):QPixmap("invisiblei.png");
	else
		px=descr?QPixmap(":offline.png"):QPixmap(":offlinei.png");

	ui.statusButton->setIcon(QIcon(px));
	setWindowIcon(QIcon(px));
	mTray->setIcon(px);
}

void qxygen::authorizationAsk(QString from)
{
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

void qxygen::addUser()
{
	if(!Tlen->isConnected())
		return;

	useradd *addDlg=new useradd(rosterModel->groupNames(),this);
	connect(addDlg, SIGNAL(addItem(QString,QString,QString,bool)), Tlen, SLOT(addItem(QString,QString,QString,bool)));
	addDlg->exec();
}

void qxygen::parseRemove()
{
	QModelIndex idx=ui.rosterView->currentIndex();

	if(idx.isValid())
		Tlen->remove(idx.model()->data(idx, rosterView::JidRole).toString());
}

void qxygen::chatMsgReceived(QDomNode n)
{
	//<message from='qxygen@tlen.pl' type='chat'><body>Test</body><x xmlns='jabber:x:delay' stamp='20060406T16:07:11'/></message>
	//USE QDateTime to convert UTC stamp to local time
	// QDateTime::fromString(stamp, "yyyyMMddTHHmmss");

	QDomElement msg=n.toElement();
	QString from=msg.attribute("from");
	QString body;
	QString timeStamp="";

	QDomNodeList nl=msg.childNodes();
	for(int i=0; i<nl.count();++i)
	{
		QDomNode tmp=nl.item(i);
		if(tmp.nodeName()=="body")
			body=tmp.firstChild().toText().data();

		if(tmp.nodeName()=="x")
		{
			QDomElement e=tmp.toElement();
			if(e.hasAttribute("xmlns") && e.attribute("xmlns")=="jabber:x:delay")
			{
				QDateTime dt;
				dt.setTimeSpec(Qt::UTC);
				dt=QDateTime::fromString("yyyyMMddTHHmmss");
				timeStamp=dt.toLocalTime().toString("dd.MM.yyyy hh:mm:ss");
			}
		}
	}

	if(timeStamp.isEmpty())
		timeStamp=QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

	//<message type='chat' from='qxygen@tlen.pl'><body>wiadomosc</body></message>

	if(chatWindow *w=chatMap[from])
		if(!w->isHidden())
			w->displayMsg(Tlen->decode(body.toUtf8()),timeStamp);
		else
			queueMsg(body,timeStamp,from);
	else
		queueMsg(body,timeStamp,from);
}

void qxygen::showChatWindow(const QModelIndex &index)
{
	if(index.model()->data(index,rosterView::TypeRole)==0)
	{
		QString jid=index.model()->data(index,rosterView::JidRole).toString();
		if(chatWindow *w=chatMap[jid])
		{
			w->show();
			if(w->isMinimized())
			{
				if(w->isMaximized())
					w->showMaximized();
				else
					w->showNormal();
			}
			w->raise();
			w->activateWindow();
		}
		else
		{
			QString label=index.model()->data(index,Qt::DisplayRole).toString();
			chatWindow *w=new chatWindow(label,jid,0);
			w->setWindowIcon(QIcon(QPixmap::fromImage(index.model()->data(index,Qt::DecorationRole).value<QImage>())));
			chatMap.insert(jid,w);
			connect(w, SIGNAL(writeMsg(QString,QString)), Tlen, SLOT(writeMsg(QString,QString)));
			w->show();
		}
	}
}

void qxygen::queueMsg(QString msg,QString ts,QString from)
{
	msgMap[from].append(msg+"\t"+ts);
	mTray->gotMsg(from);
}

void qxygen::openMsg(QString from)
{
	if(chatWindow *w=chatMap[from])
	{
		w->show();

		QStringListIterator it(msgMap[from]);
		while(it.hasNext())
		{
			QString line=it.next();
			QStringList msg=line.split("\t");
			w->displayMsg(Tlen->decode(msg[0].toUtf8()), msg[1]);
		}
		msgMap[from].clear();

		if(w->isMinimized())
		{
			if(w->isMaximized())
				w->showMaximized();
			else
				w->showNormal();
		}
		w->raise();
		w->activateWindow();
	}
	else
	{
		rosterItem *item=rosterModel->find(from);
		QString label=item->data(0);
		chatWindow *w=new chatWindow(label,from,0);
		chatMap.insert(from,w);
		connect(w, SIGNAL(writeMsg(QString,QString)), Tlen, SLOT(writeMsg(QString,QString)));
		w->setWindowIcon(QIcon(QPixmap::fromImage(item->icon())));
		QStringListIterator it(msgMap[from]);
		while(it.hasNext())
		{
			QString line=it.next();
			QStringList msg=line.split("\t");
			w->displayMsg(Tlen->decode(msg[0].toUtf8()), msg[1]);
		}
		msgMap[from].clear();

		w->show();
	}
}

void qxygen::windowUpdate(QString owner, QIcon ico)
{
	if(chatWindow *w=chatMap[owner])
		w->setWindowIcon(ico);
}
