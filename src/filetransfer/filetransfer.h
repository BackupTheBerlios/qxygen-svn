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

#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QAbstractSocket>
#include <QThread>

#include "ui_filetransferdialog.h"

class QTcpSocket;
class QTcpServer;
class QDomNode;
class QFile;

class fileTransferThread: public QThread
{
Q_OBJECT

public:
	enum PacketType {
		ConnectionRequest = 0x01,
		ConnectionRequestAck = 0x02,
		FileList = 0x32,
		FileListAck = 0x33,
		FileRequest = 0x34,
		FileData = 0x35,
		EndOfFile = 0x37,
		TransferAbort = 0x39
	};

	fileTransferThread(QString,int,QString,quint16,bool,QObject *parent=0);
	~fileTransferThread();
	void run();
	int id(){return rndid;}
	void transferingAccepted();

public slots:
	void resetFilesData();
	void addFilesToSend();
	void send();

private slots:
	void readyRead();
	void error(QAbstractSocket::SocketError);
	void disconnected();
	void newConnection();
	void estabilishFileReceiving();
	void estabilishFileTransfering( const QByteArray& );

signals:
	void updateTransferData(quint32);
	void addListItem(QTreeWidgetItem*);
	void updateFilesData(quint32, quint32);
	void prepareTransfering();

private:
	void parseFileList( const QByteArray& );
	void parseWriteData( const QByteArray& );
	void parseEndOfFile();
	void requestFile();
	void sendFileList();

	QTcpSocket *socket;
	QTcpServer *server;
	QFile *current;
	QByteArray	stream,
			streamHeader;

	int rndid;
	quint16 port;

	QString	host,
		owner;

	quint32	fc,
		fs,
		allSize,
		currentFile,
		streamSize;

	QList<QTreeWidgetItem*> fileMap;
};

class fileTransferDialog: public QDialog
{
Q_OBJECT

public:
	fileTransferDialog(int id, QString f, QString host=QString(), quint16 port=0, bool receiveMode=FALSE, QWidget *parent=0);
	int id(){return thread->id();}
	~fileTransferDialog();

	void transferingAccepted();

public slots:
	void updateTransferData(quint32);
	void addListItem(QTreeWidgetItem*);
	void updateFilesData(quint32, quint32);
	void prepareTransfering();

private slots:
	void resetDialog();

private:
	Ui::downloadDialog ui;
	QPushButton *addFile, *clearList, *send, *abort, *goToFiles;
	QHBoxLayout *buttonLay;
	QLabel *statusLabel;
	fileTransferThread *thread;
};
#endif
