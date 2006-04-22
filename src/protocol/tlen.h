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

#ifndef TLEN_H
#define TLEN_H

#include <QObject>
#include <QDomNode>

class QTcpSocket;
class QDomDocument;
class QTimer;

class tlen: public QObject
{
Q_OBJECT

public:
	enum ConnectionState{	ConnectingToHub = 0,
				Connecting = 1,
				Connected = 2,
				ErrorDisconnected = 3,
				Disconnected = 4 };

	tlen(QObject *parent=0);

	bool isConnected();

	int intStatus();

	QString strStatus(){return status;}
	QString description(){return descr;}

	QString uname(){return u;}
	void setUname(QString uname){u=uname;}

	QString pass(){return p;}
	void setPass(QString pass){p=pass;}

	void remove(QString);

	QString decode(const QByteArray&);
	QString decode(const QString&);
	QByteArray encode(const QString&);

public slots:
	void openConn();
	void closeConn();
	void writeMsg(QString,QString);
	void chatNotify(QString,bool);
	void rosterRequest();
	void setStatus();
	void setStatus(QString s);
	void setStatusDescr(QString,QString);
	void addItem(QString jid, QString name, QString group, bool subscribe);

private slots:
	bool write(const QDomDocument &d);
	void writeStatus();
	void socketConnected();
	void socketReadyRead();
	void socketDisconnected();

	void authorize(QString, bool);

	void event(QDomNode n);

	void sendPing();

signals:
	void presenceDisconnected();
	void itemReceived(QString, QString, QString, QString,bool);
	void presenceChanged(QString, QString, QString);
	void authorizationAsk(QString);
	void removeItem(QString);

	void sortRoster();

	void chatMsgReceived(QDomNode);

	void clearRosterView();
	void tlenLoggedIn();
	void statusChanged(); // FOR GUI TO UPDATE ICONS
	void statusUpdate();	// FOR TLEN TO WRITE STATUS
	void eventReceived(QDomNode);

	void fileIncoming(QDomNode);

	void chatNotify(QString,QString);
private:
	bool tlenLogin();

	QTimer *ping;

	QDomDocument *tmpDoc;

	QByteArray stream;

	int state;

	QString	u,
		p,
		sid,
		hostname,
		status,
		descr;

	quint16 hostport;
	QTcpSocket *socket;
};
#endif
