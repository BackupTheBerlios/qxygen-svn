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

#include <QPushButton>
#include <QHBoxLayout>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDomNode>
#include <QFile>
#include <QStringList>

#include <QDebug>

#include "filetransfer.h"
#include "settings.h"
#include "sha1.h"

void hexdump(unsigned char *buf, size_t len) {
	qDebug()<<"Hex:";
	size_t i;
	for (i = 0; i < len; i++) {
		printf("%02x ", buf[i]);
		if ((i + 1) % 8 == 0 || i+1==len) {
			printf("\n");
		} else if ((i + 1) % 4 == 0) {
			printf("\t");
		}
	}
}

fileTransferDialog::fileTransferDialog(QDomNode n, bool receiveMode, QWidget *parent): QDialog(parent) {
	ui.setupUi(this);
	buttonLay = new QHBoxLayout();
	buttonLay->setMargin(0);
	ui.bottomFrame->setContentsMargins(0,0,0,0);
	statusLabel = new QLabel;
	buttonLay->addWidget(statusLabel);
	buttonLay->addStretch();
	abort = new QPushButton(tr("Abort"));

	current=new QFile(this);

	QStringList labelList;
	labelList<<tr("Filename")<<tr("Speed")<<tr("Elapsed")<<tr("Estimated")<<tr("Progress")<<tr("Size")<<tr("Path")<<"RealSize";
	ui.fileListTreeWidget->setHeaderLabels(labelList);
	ui.fileListTreeWidget->setColumnHidden(7,TRUE);

	QDomElement e=n.toElement();
	rndid=e.attribute("i").toInt();
	setWindowIcon( QIcon(":logo.png") );
	setWindowTitle( QString("%1@tlen.pl sends you file(s)").arg( e.attribute("f") ) );

	socket = new QTcpSocket();
	connect( socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
	connect( socket, SIGNAL( error(QAbstractSocket::SocketError) ), this, SLOT( error(QAbstractSocket::SocketError) ) );
	connect( socket, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );

	if(receiveMode) {
		connect( socket, SIGNAL( connected() ), this, SLOT( estabilishFileReceiving() ) );
		socket->connectToHost( e.attribute("a"), e.attribute("p").toUInt() );
	} else {
		addFile = new QPushButton(tr("Add file to send"));
		clearList = new QPushButton(tr("Clear list"));
		send = new QPushButton(tr("Send"));
		buttonLay->addWidget(addFile);
		buttonLay->addWidget(clearList);
		buttonLay->addWidget(send);
		server=new QTcpServer();
		server->listen(QHostAddress::Any, 443);
		connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
	}
	buttonLay->addWidget(abort);
	ui.bottomFrame->setLayout(buttonLay);
}

void fileTransferDialog::readyRead() {
	while( socket->bytesAvailable() ) {
		if(stream.isEmpty()) {
			streamHeader=socket->read(8);
			memcpy(&streamSize, streamHeader.mid(4).data(), 4);
		}
		stream+=socket->read( streamSize-stream.size() );

		if( streamSize > (quint32)stream.size() ) //packet is not full
			break;

		if( streamHeader.at(0) == ConnectionRequest ) {
			qDebug()<<"ConnectionRequest";
			stream.clear();
		} else if( streamHeader.at(0) == ConnectionRequestAck) {
			qDebug()<<"ConnectionRequestAck";
			stream.clear();
		} else if( streamHeader.at(0) == FileList) {
			qDebug()<<"FileList";
			parseFileList( stream );
			stream.clear();	
		} else if( streamHeader.at(0) == FileListAck) {
			qDebug()<<"FileListAck";
			stream.clear();
		} else if( streamHeader.at(0) == FileRequest) {
			qDebug()<<"FileRequest";
			stream.clear();
		} else if( streamHeader.at(0) == FileData) {
			qDebug()<<"FileData";
			parseWriteData( stream );
			stream.clear();
		} else if( streamHeader.at(0) == EndOfFile) {
			qDebug()<<"EndOfFile";
			parseEndOfFile();
			stream.clear();
		} else if( streamHeader.at(0) == TransferAbort) {
			qDebug()<<"TransferAbort";
			stream.clear();
		} else {
			qDebug()<<"Unknown type";
			stream.clear();
		}
	}
}

void fileTransferDialog::estabilishFileReceiving() {
	char head[8];
	char packet[28];
	quint32 tmp;
	char str[300];
	tmp = ConnectionRequest;
	memcpy(&packet, &tmp, 4);
	memcpy(&head, &tmp, 4);
	memcpy(&packet[4], &rndid, 4);
	char *hash;
	char username[128];
	sprintf(username, settings->profileValue("user/login").toByteArray().data() );
	snprintf(str, sizeof(str), "%08X%s%d", rndid, username, rndid );
	hash=TlenSha1( str, strlen(str) );
	memcpy(&packet[8], hash, 20);
	tmp=28;
	memcpy(&head[4], &tmp, 4);
	socket->write( head, sizeof(head) );
	socket->write( packet, sizeof(packet) );
}

void fileTransferDialog::estabilishFileTransfering() {
}

void fileTransferDialog::error(QAbstractSocket::SocketError e) {
	qDebug()<<"Error: "<<e;
}

void fileTransferDialog::disconnected() {
	qDebug()<<"Disconnected!";
}

void fileTransferDialog::newConnection() {
	socket=server->nextPendingConnection();
	connect( socket, SIGNAL( connected() ), this, SLOT( estabilishFileTransfering() ) );
}

void fileTransferDialog::parseFileList( const QByteArray &fl ) {
	fc=0;
	allSize=0;
	currentFile=0;
	QTreeWidgetItem *item;
	QStringList finfo;
	QString path = settings->profileValue( QString("filetransfer/%1").arg(rndid) ).toString(), fname;
	memcpy(&fc, fl.mid(0,4).data(), 4);
	for(quint32 i=0; i<fc; ++i) {
		memcpy(&fs, fl.mid(4+i*260, 4).data(), 4);
		fname=fl.mid(4+i*260+4, 256);
		finfo<<fname<<""<<""<<""<<""<<QString("%L1 kB").arg( (float)qRound( ( (float)fs/1024 )*10 )/10 )<<path<<QString("%1").arg(fs);
		item = new QTreeWidgetItem(ui.fileListTreeWidget, finfo);
		item->setIcon(0, QIcon(":offline.png"));
		fileMap.insert(i,item);
		allSize+=fs;
		finfo.clear();
	}
	ui.fileCountLabel->setText( tr("File count: %1").arg(fc) );
	ui.filesRemainedLabel->setText( tr("Files remained: %1 (%2 kB)").arg(fc).arg(allSize/1024) );
	ui.filesSizeLabel->setText( tr("Files size: %1 kB").arg(allSize/1024) );
	settings->removeProfileValue( QString("filetransfer/%1").arg(rndid) );
	char packet[8];
	quint32 tmp=FileListAck;
	memcpy(&packet, &tmp,4);
	tmp=0;
	memcpy(&packet[4], &tmp,4);
	socket->write(packet,sizeof(packet));

	requestFile();
}

void fileTransferDialog::requestFile() {
	if(currentFile < fc) {
		char fpacket[20];
		quint32 tmp=FileRequest;
		memcpy(&fpacket, &tmp,4);
		tmp=12;
		memcpy(&fpacket[4], &tmp, 4);
		tmp=0;
		memcpy(&fpacket[8], &currentFile, 4);
		memcpy(&fpacket[12], &tmp, 4);
		memcpy(&fpacket[16], &tmp, 4);
		socket->write( fpacket, sizeof(fpacket) );
		current->setFileName( fileMap[currentFile]->data(6,Qt::DisplayRole).toString()+"/"+fileMap[currentFile]->data(0,Qt::DisplayRole).toString() );
		current->open(QIODevice::WriteOnly);
		fileMap[currentFile]->setIcon(0, QIcon(":online.png"));
	}
}

void fileTransferDialog::parseWriteData( const QByteArray &data ) {
	current->write( data.mid(8) ); //skip file offset
	updateTransferData(streamSize-8);
}

void fileTransferDialog::parseEndOfFile() {
	current->close();
	++currentFile;
	requestFile();
}

void fileTransferDialog::updateTransferData(quint32) {

}
