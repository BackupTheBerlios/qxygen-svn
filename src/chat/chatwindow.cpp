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
#include <QPushButton>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDateTime>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QLabel>
#include <QTimer>

#include <QDebug>

#include "chatwindow.h"
#include "chattextedit.h"
#include "settings.h"

chatWindow::chatWindow( QString label, QString jid, QWidget *parent ): QWidget( parent ) {
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
	display->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	QWidget *lower=new QWidget(splitter);

	QGridLayout *lowerLayout=new QGridLayout(lower);
	lowerLayout->setMargin(0);

	input=new chatTextEdit();
	input->installEventFilter(this);
	input->setTabChangesFocus(TRUE);
	input->setAcceptRichText(FALSE);
	sendButton=new QPushButton(tr("Send"));
	sendButton->setIcon(QIcon(":send.png"));
	sendButton->adjustSize();
	sendButton->setDisabled(TRUE);

	setReturnSend=new QPushButton();
	setReturnSend->setIcon(QIcon(":enter.png"));
	setReturnSend->setCheckable(TRUE);
	setReturnSend->setChecked(settings->profileValue("chat/returnSend").toBool());

	QBoxLayout *top=new QBoxLayout(QBoxLayout::LeftToRight);
	top->setMargin(0);
	QBoxLayout *bottom=new QBoxLayout(QBoxLayout::RightToLeft);
	bottom->setMargin(0);

	typingNotify=new QLabel();
	typingNotify->setPixmap(QPixmap(":noff.png"));

	bottom->addWidget(sendButton);
	bottom->addWidget(setReturnSend);
	bottom->insertStretch(-1,10);

	top->addWidget(typingNotify);
	top->insertStretch(-1,10);

	lowerLayout->addLayout(top,0,0);
	lowerLayout->addWidget(input,1,0);
	lowerLayout->addLayout(bottom,2,0);

	connect(input,SIGNAL(textChanged()),this,SLOT(checkSend()));
	connect(sendButton, SIGNAL(clicked()),this,SLOT(sendMsg()));
	connect(setReturnSend, SIGNAL(clicked()), this, SLOT(setupReturnSend()));

	typingNotify->adjustSize();
	setReturnSend->adjustSize();

	msgTimer=new QTimer();

	connect(msgTimer, SIGNAL(timeout()), this, SLOT(swapTitleBar()));
	input->setFocus();

	move(settings->profileValue("chat/position").value<QPoint>());
	resize(settings->profileValue("chat/size").value<QSize>());
}

void chatWindow::checkSend() {
	sendButton->setDisabled(input->toPlainText().isEmpty());
}

bool chatWindow::eventFilter(QObject *obj, QEvent *ev) {
	if(obj==input) {
		if(ev->type()==QEvent::KeyPress) {
			QKeyEvent *keyEvent=static_cast<QKeyEvent*>(ev);
			if(keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter) {
				if(QApplication::keyboardModifiers()==Qt::ShiftModifier)
					input->insertPlainText("\n");
				else if(setReturnSend->isChecked())
					sendMsg();
				else
					return QWidget::eventFilter(obj,ev);

				return true;
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

void chatWindow::sendMsg() {
	QString msg=input->toPlainText();
	if(msg.isEmpty())
		return;
	emit writeMsg(msg,owner);
	input->clear();
	msg.replace("<","&lt;");
	msg.replace(">","&gt;");
	msg.replace(" ", "&ensp;");
	msg.replace("\n", "<br/>");
	display->append("<table width=\"100%\" style=\"background-color: #ffffff;\"><tr><td><b>"+settings->profileValue("user/profile").toString()+" :: "+QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")+"</b></td></tr><tr><td>"+msg+"</td></tr></table>");
}

void chatWindow::displayMsg(QString msg, QString time) {
	msg.replace("<","&lt;");
	msg.replace(">","&gt;");
	msg.replace(" ", "&ensp;");
	msg.replace("\n","<br/>");
	display->append("<table width=\"100%\" style=\"background-color: #e0e0e0;\"><tr><td><b>"+title+" :: "+time+"</b></td></tr><tr><td>"+msg+"</td></tr></table>");

	if(!isActiveWindow())
		msgTimer->start(500);
}

void chatWindow::swapTitleBar() {
	windowTitle()==" "?setWindowTitle(title):setWindowTitle(" ");
}

void chatWindow::setupReturnSend() {
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
