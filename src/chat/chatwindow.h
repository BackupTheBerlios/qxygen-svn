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

#include <QDialog>
#include <QPushButton>
#include <QLabel>

#include "chattextedit.h"

class chatWindow: public QDialog
{
Q_OBJECT

public:
	chatWindow(QString,QString, QWidget *parent=0);

	QString jid(){return owner;}
	void displayMsg(QString,QString);

	void typingNotify(bool);

private slots:
	void checkSend();
	void sendMsg();
	void swapTitleBar();
	void setupReturnSend();
	void chatNotifyStop();

signals:
	void writeMsg(QString,QString);
	void chatNotify(QString, bool);

protected:
	void resizeEvent(QResizeEvent*);
	void moveEvent(QMoveEvent*);
	bool eventFilter(QObject *obj, QEvent *ev);

private:
	QString owner,title;

	QTimer *msgTimer,*notifyTimer;

	chatTextEdit *input,*display;
	QLabel *typingNotifyLabel;
	QPushButton *sendButton, *sendAlarm, *setReturnSend;
};
#endif
