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

#include "ui_filetransferdialog.h"

class QTcpSocket;
class QTcpServer;
class QDomNode;
class QFile;

class fileTransferDialog: public QDialog
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
	fileTransferDialog(QDomNode, bool receiveMode=FALSE, QWidget *parent=0);

private slots:
	void estabilishFileTransfering();
	void estabilishFileReceiving();
	void disconnected();
	void newConnection();
	void error(QAbstractSocket::SocketError);
	void readyRead();

private:
	void parseFileList( const QByteArray& );
	void parseWriteData( const QByteArray& );
	void parseEndOfFile();
	void requestFile();
	void updateTransferData(quint32);

	quint32 fc,fs, allSize, currentFile, streamSize;
	QFile *current;
	QByteArray stream,streamHeader;
	int rndid;
	Ui::downloadDialog ui;
	QPushButton *addFile, *clearList, *send, *abort, *goToFiles;
	QHBoxLayout *buttonLay;
	QLabel *statusLabel;
	QTcpSocket *socket;
	QTcpServer *server;
	QMap<int, QTreeWidgetItem*> fileMap;
};
#endif
