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

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>

class chatTextEdit;
class QShortcut;
class QPushButton;
class QLabel;

class chatWindow: public QWidget
{
Q_OBJECT

public:
	chatWindow(QString,QString, QString, QWidget *parent=0);

	QString jid(){return owner;}
	void displayMsg(QString,QString);

private slots:
	void checkSend();
	void sendMsg();
	void swapTitleBar();

signals:
	void writeMsg(QString,QString);

protected:
	bool eventFilter(QObject *obj, QEvent *ev);

private:
	QString owner,title,profileName;

	QTimer *msgTimer;

	chatTextEdit *input,*display;
	QLabel *typingNotify;
	QPushButton *sendButton, *sendAlarm, *setReturnSend;
};
#endif
