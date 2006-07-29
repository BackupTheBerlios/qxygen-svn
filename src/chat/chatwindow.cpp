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

#include <QSplitter>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDateTime>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include <QDebug>

#include "chatwindow.h"
#include "settings.h"
#include "tlen.h"

chatWindow::chatWindow( QString label, QString jid, QWidget *parent ): QWidget( parent ) {
	firstmsg=1;
	installEventFilter(this);
	setGeometry(150,150,350,300);
	title=label;
	owner=jid;
	setWindowTitle(title);
	QGridLayout *layout=new QGridLayout(this);
	layout->setMargin(2);
	QSplitter *splitter=new QSplitter(Qt::Vertical);
	layout->addWidget(splitter,0,0);
	splitter->setChildrenCollapsible(FALSE);
	display=new chatTextEdit(splitter);
	display->setReadOnly(TRUE);
	display->setWordWrapMode(QTextOption::WrapAnywhere);
	cursor=QTextCursor(display->textCursor());
	cursor.movePosition(QTextCursor::Start);

	QWidget *lower=new QWidget(splitter);

	QGridLayout *lowerLayout=new QGridLayout(lower);
	lowerLayout->setMargin(0);

	input=new chatTextEdit();
	input->installEventFilter(this);
	input->setTabChangesFocus(TRUE);
	input->setAcceptRichText(FALSE);
	updateSettings();
	sendButton=new QPushButton(tr("Send"));
	sendButton->setIcon(QIcon(":send.png"));
	sendButton->adjustSize();
	sendButton->setDisabled(TRUE);

	setReturnSend=new QPushButton();
	setReturnSend->setCheckable(TRUE);
	setReturnSend->setChecked(settings->profileValue("chat/returnSend").toBool());
	if(setReturnSend->isChecked())
		setReturnSend->setIcon(QIcon(":return_on.png"));
	else
		setReturnSend->setIcon(QIcon(":return_off.png"));

	QBoxLayout *top=new QBoxLayout(QBoxLayout::LeftToRight);
	top->setMargin(0);
	QBoxLayout *bottom=new QBoxLayout(QBoxLayout::RightToLeft);
	bottom->setMargin(0);

	typingNotifyLabel=new QLabel();
	typingNotifyLabel->setPixmap(QPixmap(":noff.png"));

	bottom->addWidget(sendButton);
	bottom->addWidget(setReturnSend);
	bottom->insertStretch(-1,10);

	top->addWidget(typingNotifyLabel);
	top->insertStretch(-1,10);

	lowerLayout->addLayout(top,0,0);
	lowerLayout->addWidget(input,1,0);
	lowerLayout->addLayout(bottom,2,0);

	connect(input,SIGNAL(textChanged()),this,SLOT(checkSend()));
	connect(sendButton, SIGNAL(clicked()),this,SLOT(sendMsg()));
	connect(setReturnSend, SIGNAL(clicked()), this, SLOT(setupReturnSend()));

	typingNotifyLabel->adjustSize();
	setReturnSend->adjustSize();

	msgTimer=new QTimer();
	notifyTimer=new QTimer();

	connect(msgTimer, SIGNAL(timeout()), this, SLOT(swapTitleBar()));
	connect(notifyTimer, SIGNAL(timeout()), this, SLOT(chatNotifyStop()));
	input->setFocus();

	move(settings->profileValue("chat/position").value<QPoint>());
	resize(settings->profileValue("chat/size").value<QSize>());
	setWindowFlags(Qt::Window);
	show();
}

void chatWindow::checkSend() {
	sendButton->setDisabled(input->toPlainText().isEmpty());
}

bool chatWindow::eventFilter(QObject *obj, QEvent *ev) {
	if(obj==input) {
		if(ev->type()==QEvent::KeyPress) {
			QKeyEvent *keyEvent=static_cast<QKeyEvent*>(ev);
			if(keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter) {
				if( setReturnSend->isChecked() && QApplication::keyboardModifiers()!=Qt::ShiftModifier )
					sendMsg();
				else
					return QWidget::eventFilter(obj,ev);

				return true;
			}
			else if(!keyEvent->text().isEmpty() && keyEvent->key()!=Qt::Key_Tab) {
				if(!notifyTimer->isActive())
					emit chatNotify(owner, TRUE);
				notifyTimer->start(5000);
			}
		}
	}
	else if(ev->type()==QEvent::WindowActivate) {
		msgTimer->stop();
		if(windowTitle()==" ")
			setWindowTitle(title);
	}

	return QWidget::eventFilter(obj,ev);
}

void chatWindow::chatNotifyStop() {
	emit chatNotify(owner, FALSE);
	notifyTimer->stop();
}

void chatWindow::sendMsg() {
	if(Tlen->strStatus()=="unavailable") {
		QMessageBox::information(	this,
						tr("Connection error"),
						tr("You can't send message. You're not connected. Connect to server first."),
						1);
		return;
	}

	if(input->toPlainText().isEmpty())
		return;

	emit writeMsg(input->toPlainText(),owner);



	QTextBlockFormat bf;
	bf.setForeground( QBrush( settings->profileValue("look/color/yourc").value<QColor>() ) );
	bf.setBackground( QBrush( settings->profileValue("look/color/yourbg").value<QColor>() ) );
	if(firstmsg) {
		cursor.beginEditBlock();
		cursor.setBlockFormat(bf);
	} else
		cursor.insertBlock(bf);

	QTextCharFormat tf;
	tf.setFont( settings->profileValue("look/font/headermsg").value<QFont>() );
	cursor.insertText( settings->profileValue("user/profile").toString()+" :: "+QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss\n"),tf);
	tf.setFont( settings->profileValue("look/font/yourmsg").value<QFont>() );
	cursor.insertText( input->toPlainText(), tf );

	if(firstmsg) {
		cursor.endEditBlock();
		firstmsg=0;
	}


	notifyTimer->stop();
	input->clear();
	display->scrollToBottom();
}

void chatWindow::displayMsg(QString msg, QString time) {
	QTextBlockFormat bf;
	bf.setForeground( QBrush( settings->profileValue("look/color/userc").value<QColor>() ) );
	bf.setBackground( QBrush( settings->profileValue("look/color/userbg").value<QColor>() ) );
	if(firstmsg) {
		cursor.beginEditBlock();
		cursor.setBlockFormat(bf);
	} else
		cursor.insertBlock(bf);

	QTextCharFormat tf;
	tf.setFont( settings->profileValue("look/font/headermsg").value<QFont>() );
	cursor.insertText( title+" :: "+time+"\n",tf);
	tf.setFont( settings->profileValue("look/font/usermsg").value<QFont>() );
	cursor.insertText( msg, tf );

	if(firstmsg) {
		cursor.endEditBlock();
		firstmsg=0;
	}

	if(!isActiveWindow())
		msgTimer->start(500);

	typingNotifyLabel->setPixmap(QPixmap(":noff.png"));
	display->scrollToBottom();
}

void chatWindow::swapTitleBar() {
	windowTitle()==" "?setWindowTitle(title):setWindowTitle(" ");
}

void chatWindow::setupReturnSend() {
	if(setReturnSend->isChecked())
		setReturnSend->setIcon(QIcon(":return_on.png"));
	else
		setReturnSend->setIcon(QIcon(":return_off.png"));

	settings->setProfileValue("chat/returnSend", QVariant(setReturnSend->isChecked()));
}

void chatWindow::resizeEvent(QResizeEvent *e) {
	settings->setProfileValue("chat/size", QVariant(size()));
	settings->setProfileValue("chat/position", QVariant(pos()));
	QWidget::resizeEvent(e);
}

void chatWindow::moveEvent(QMoveEvent *e) {
	settings->setProfileValue("chat/size", QVariant(size()));
	settings->setProfileValue("chat/position", QVariant(pos()));
	QWidget::moveEvent(e);
}

void chatWindow::typingNotify(bool t) {
	if(t)
		typingNotifyLabel->setPixmap(QPixmap(":non.png"));
	else
		typingNotifyLabel->setPixmap(QPixmap(":noff.png"));
}

void chatWindow::updateSettings() {
	input->setFont( settings->profileValue("look/font/yourmsg").value<QFont>() );
	QPalette tmp=input->palette();
	tmp.setColor( QPalette::Base, settings->profileValue("look/color/teditbg").value<QColor>() );
	tmp.setColor( QPalette::Text, settings->profileValue("look/color/teditc").value<QColor>() );
	input->setPalette(tmp);
}
