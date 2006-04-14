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

#include <QTcpSocket>
#include <QDomDocument>
#include <QTimer>
#include <QTextCodec>
#include <QRegExp>
#include <QAction>
#include <QSettings>
#include <QDebug>

#include "tlen.h"
#include "auth.h"

tlen::tlen( QObject* parent ): QObject( parent ) {
	state = tlen::Disconnected;
	hostname = "s1.tlen.pl";
	hostport = 443;

	status="unavailable";
	descr="";

	socket=new QTcpSocket();
	ping=new QTimer();

	connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

	connect(this, SIGNAL(tlenLoggedIn()), this, SLOT(writeStatus()));
	connect(this, SIGNAL(statusUpdate()), this, SLOT(writeStatus()));

	connect(this, SIGNAL(eventReceived(QDomNode)), this, SLOT(event(QDomNode)));

	connect(ping, SIGNAL(timeout()), this, SLOT(sendPing()));
}

void tlen::openConn() {
	state=tlen::Connecting;
	socket->connectToHost(hostname, hostport);
}

void tlen::closeConn() {
	socket->close();
	state=tlen::Disconnected;
}

bool tlen::isConnected() {
	switch(state) {
	case tlen::Connected:
	case tlen::Connecting:
		return true;
	break;

	default:
		return false;
	break;
	}
	return false;
}

void tlen::socketReadyRead() {
	stream="<s>";
	stream+=socket->readAll();
	stream+="</s>";

	qDebug()<<"Read:"<<stream;

	QDomDocument d;
	d.setContent(stream);
	QDomNode root=d.firstChild();

	if(root.hasChildNodes()) {
		QDomNodeList sl=root.childNodes();
		for(int i=0; i<sl.count(); i++)
			emit eventReceived(sl.item(i));
	}
}

void tlen::socketConnected() {
	state = tlen::Connecting;
	socket->write(QString("<s v=\"7\" t=\"05170402\">").toUtf8());
}


bool sort=TRUE;

void tlen::event(QDomNode n) {
	QString nodeName=n.nodeName();
	QDomElement element = n.toElement();
	if(nodeName=="s" && element.hasAttribute("i")) {
		ping->start( 60000 );
		sid = element.attribute("i");
		if(tlenLogin())
 			state = tlen::Connected;
		else
			socket->close();
	}
	else if(nodeName=="iq") {
		if(element.hasAttribute( "type" ) && element.attribute("type") == "result") {
			if(element.hasAttribute("id") && element.attribute("id")==sid) {
				rosterRequest();
				emit tlenLoggedIn();
			}

			if(element.hasAttribute("id") && element.attribute("id")=="GetRoster") {
				emit clearRosterView();
				sort=FALSE;
			}

			if(n.hasChildNodes()) {
				QDomNodeList el=n.childNodes();
				for(int i=0;i<el.count();++i)
					emit eventReceived(el.item(i));
			}

			if(!sort) {
				emit sortRoster();
				sort=TRUE;
			}

		}
		else if(element.hasAttribute("type") && element.attribute("type") == "set") {
			if(n.hasChildNodes()) {
				QDomNodeList el=n.childNodes();
				for(int i=0;i<el.count();++i)
					emit eventReceived(el.item(i));
			}
		}
	}
	else if(nodeName=="query") {
		QDomElement e=n.toElement();
		QDomNodeList nl=n.childNodes();
		for(int i=0;i<nl.count();++i)
			event(nl.item(i));
	}
	else if(nodeName=="item") {
		QDomElement e=n.toElement();
		QString jid=e.attribute("jid");
		QString subscription=e.attribute("subscription");
		QString name=NULL, group=NULL;

		if(subscription=="remove") {
			emit removeItem(jid);
			return;
		}

		if(e.hasAttribute("ask"))
			subscription=e.attribute("ask");

		if(e.hasAttribute("name"))
			name=e.attribute("name");

		if( n.hasChildNodes() ) {
			QDomNodeList nl=n.childNodes();
			for(int i=0;i<nl.count();++i) {
				group=nl.item(i).firstChild().toText().data();
			}
		}

		if(group.isEmpty())
			group=tr("General");
		if(name.isEmpty())
			name=jid;

		group=decode(group.toUtf8());
		name=decode(name.toUtf8());

		emit itemReceived(jid, name, subscription, group, sort);

	}
	else if(nodeName=="presence") {
		QDomElement e=n.toElement();
		QString from=e.attribute("from");

		if(e.hasAttribute("type") && e.attribute("type")=="subscribe") {
			emit authorizationAsk(from);
		}
		else if(e.hasAttribute("type") && (e.attribute("type")=="unsubscribe" || e.attribute("type")=="subscribed" || e.attribute("type")=="unsubscribed"))
			return;
		else {
			QString status="none";
			QString descr="";

			if(e.hasAttribute("type"))
				status=e.attribute("type");

			QDomNodeList l=n.childNodes();

			for(int i=0; i<l.count(); ++i) {
				if(l.item(i).nodeName()=="show" && status=="none")
					status=l.item(i).firstChild().toText().data();
				if(l.item(i).nodeName()=="status")
					descr=l.item(i).firstChild().toText().data();
			}

			descr=decode(descr.toUtf8());

			emit presenceChanged(from,status,descr);
		}
	}
	else if(nodeName=="message") {
		QDomElement e=n.toElement();
		if(e.hasAttribute("type") && e.attribute("type")=="chat")
			emit chatMsgReceived(n);
	}
	else if(nodeName=="m") {
		QDomElement e=n.toElement();
		if(e.hasAttribute("tp")) {
			chatNotify(e.attribute("f"), e.attribute("tp"));
		}
	}
	else if(nodeName=="n") {
		//<n f='Rainer+Wiesenfarth+%3CRainer.Wiesenfarth@inpho.de%3E' s='Re%3A+qt+and+mysql,+odbc'/> - new mail
	}
}

void tlen::socketDisconnected() {
	state=tlen::Disconnected;
	ping->stop();
	status="unavailable";
	emit statusChanged();
	emit presenceDisconnected();
}

void tlen::sendPing() {
	socket->write(QString("  \t  ").toUtf8());
}

bool tlen::tlenLogin() {
	if( !isConnected() )
		return false;

	QDomDocument doc;
	
	QDomElement iq = doc.createElement( "iq" );
	iq.setAttribute( "type", "set" );
	iq.setAttribute( "id", sid );
	doc.appendChild( iq );
	
	QDomElement query = doc.createElement( "query" );
	query.setAttribute( "xmlns", "jabber:iq:auth" );
	iq.appendChild( query );
	
	QDomElement username_node = doc.createElement( "username" );
	query.appendChild( username_node );
	
	QDomText text = doc.createTextNode( u );
	username_node.appendChild( text );
	
	QDomElement digest = doc.createElement( "digest" );
	query.appendChild( digest );
	
	text = doc.createTextNode( tlen_hash( p.toAscii().data(), sid.toAscii().data() ) );
	digest.appendChild( text );
	
	QDomElement resource = doc.createElement( "resource" );
	query.appendChild( resource );
	text = doc.createTextNode( "t" );
	resource.appendChild( text );
	return write(doc);
}

bool tlen::write( const QDomDocument &d ) {
	if( !isConnected() ) {
		openConn();
		return FALSE;
	}

	qDebug()<<"Write:"<<d.toByteArray();

	return (socket->write(d.toByteArray()) == (qint64)d.toByteArray().size());
}

void tlen::rosterRequest() {
	QDomDocument doc;
	QDomElement iq = doc.createElement( "iq" );
	iq.setAttribute( "type", "get" );
	iq.setAttribute( "id", "GetRoster" );
	doc.appendChild( iq );
	QDomElement query = doc.createElement( "query" );
	query.setAttribute( "xmlns", "jabber:iq:roster" );
	iq.appendChild( query );
	write(doc);
}

QString tlen::decode( const QByteArray &in ) {
	QByteArray o;
	QString out;

	QRegExp ex("\\%[0-9A-Fa-f][0-9A-Fa-f]");

	for(int i = 0; i < in.length(); ++i) {
		if( in.mid(i,1) == "+" )
			o += " ";
		else if( ex.exactMatch(in.mid(i,3)) ) {
			bool ok;
			int code = QString(in.mid(i+1,2)).toInt( &ok, 16 );

			if(ok)
				o += (char)code;
			i+=2;
		}
		else
			o += in.mid(i,1);
	}

	QTextCodec *codec=QTextCodec::codecForName("ISO 8859-2");
	out = codec->toUnicode( o );

	return out;
}

QByteArray tlen::encode( const QString &in ) {
	QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");
	QByteArray o = codec->fromUnicode( in );

	QByteArray out;

	QRegExp ex("[^A-Za-z0-9\\.\\-\\_]");

	for(int i = 0; i < o.length(); ++i) {
		if( o.mid(i,1) == " " )
			out += "+";
		else if( ex.exactMatch( o.mid(i,1) ) ) {
			uchar inCh = (uchar)o[ i ];

			out += "%";

			ushort c = inCh / 16;
			c += c > 9 ? 'A' - 10 : '0';
			out += c;

			c = inCh % 16;
			c += c > 9 ? 'A' - 10 : '0';
			out += c;
		}
		else
			out += o.mid(i,1);
	}

	return out;
}

void tlen::writeStatus() {
	QDomDocument doc;
	QDomElement p = doc.createElement("presence");
	QDomElement s = doc.createElement("show");
	QDomElement d = doc.createElement("status");

	if(status=="unavailable" || status=="invisible")
		p.setAttribute("type", status);
	else
		s.appendChild(doc.createTextNode(status));

	if(!descr.isEmpty())
		d.appendChild(doc.createTextNode(QString(encode(descr))));

	p.appendChild(s);
	p.appendChild(d);
	doc.appendChild(p);

	if(write(doc))
		emit statusChanged();
}

void tlen::setStatus() {
	QAction *a=qobject_cast<QAction*>(sender());
	if(intStatus()==a->data().toInt())
		return;
	switch(a->data().toInt()) {
	case 0: status="available"; break;
	case 1: status="chat"; break;
	case 2: status="away"; break;
	case 3: status="xa"; break;
	case 4: status="dnd"; break;
	case 5: status="invisible"; break;
	case 6: status="unavailable"; break;
	}
	emit statusUpdate();
}

void tlen::setStatus( QString s ) {
	if(status==s)
		return;

	status=s;
	emit statusUpdate();
}

int tlen::intStatus() {
	if(status=="available") return 0;
	else if(status=="chat") return 1;
	else if(status=="away") return 2;
	else if(status=="xa") return 3;
	else if(status=="dnd") return 4;
	else if(status=="invisible") return 5;

	return 6;
}

void tlen::authorize( QString to, bool subscribe ) {
	QDomDocument doc;
	QDomElement p=doc.createElement("presence");
	p.setAttribute("to", to);

	if(subscribe) {
		p.setAttribute("type", "subscribe");
		doc.appendChild(p);
		write(doc);
		doc.clear();
		p=doc.createElement("presence");
		p.setAttribute("to", to);
		p.setAttribute("type", "subscribed");
		doc.appendChild(p);
		write(doc);
	}
	else {
		p.setAttribute("type", "unsubscribed");
		doc.appendChild(p);
		write(doc);
	}
}

void tlen::addItem( QString jid, QString name, QString g, bool subscribe ) {
	QDomDocument doc;
	QDomElement iq=doc.createElement("iq");
	iq.setAttribute("type", "set");
	iq.setAttribute("id", sid);

	QDomElement query=doc.createElement("query");
	query.setAttribute("xmlns","jabber:iq:roster");

	QDomElement item=doc.createElement("item");

	int atPos=jid.indexOf("@");
	if(atPos!=-1)
		jid.remove(atPos, jid.length()-atPos);

	jid+="@tlen.pl";

	item.setAttribute("jid", jid.toLower());

	if(!name.isEmpty())
		item.setAttribute("name", name);

	if(!g.isEmpty()) {
		QDomElement group=doc.createElement("group");
		QDomText t=doc.createTextNode(g);
		group.appendChild(t);
		item.appendChild(group);
	}

	query.appendChild(item);
	iq.appendChild(query);
	doc.appendChild(iq);
	write(doc);

	if(subscribe) {
		doc.clear();
		QDomElement p=doc.createElement("presence");
		p.setAttribute("type","subscribe");
		p.setAttribute("to", jid.toLower());
		doc.appendChild(p);
		write(doc);
	}
}

void tlen::remove(QString jid) {
	QDomDocument doc;
	QDomElement iq=doc.createElement("iq");
	iq.setAttribute("type", "set");
	iq.setAttribute("id", sid);

	QDomElement query=doc.createElement("query");
	query.setAttribute("xmlns", "jabber:iq:roster");

	QDomElement item=doc.createElement("item");
	item.setAttribute("subscription","remove");
	item.setAttribute("jid", jid);

	query.appendChild(item);
	iq.appendChild(query);
	doc.appendChild(iq);
	write(doc);
}

void tlen::writeMsg( QString msg, QString to ) {
	QDomDocument doc;
	QDomElement message=doc.createElement("message");
	message.setAttribute("type", "chat");
	message.setAttribute("to", to);

	QDomElement body=doc.createElement("body");
	QDomText text=doc.createTextNode(QString(encode(msg)));
	body.appendChild(text);
	message.appendChild(body);
	doc.appendChild(message);
	write(doc);
}

void tlen::chatNotify( QString to, bool t ) {
	QDomDocument doc;
	QDomElement m=doc.createElement("m");
	m.setAttribute("to", to);

	if(t)
		m.setAttribute("tp", "t");
	else
		m.setAttribute("tp", "u");

	doc.appendChild(m);
	write(doc);
}