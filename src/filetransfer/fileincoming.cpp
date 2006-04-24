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

#include <QDomNode>
#include <QDir>

#include "fileincoming.h"
#include "settings.h"

fileIncomingDialog::fileIncomingDialog(QDomNode fnode, QWidget *parent): QDialog(parent) {
	ui.setupUi(this);
	QDomElement e=fnode.toElement();

	rndid=e.attribute("i");
	sender=e.attribute("f");

	if( QVariant( e.attribute("c") ).toInt()>1 )
		setWindowTitle( sender+"@tlen.pl "+tr("sends you files") );
	else
		setWindowTitle( sender+"@tlen.pl "+tr("sends you file") );

	QString msg;
	if( e.hasAttribute("n") )
		msg+=tr("File name: %1 Size: %2kB Do you want to receive file?").arg( e.attribute("n") ).arg( QVariant( e.attribute("s") ).toInt()/1024 );
	else
		msg+=tr("File count: %1 Size: %2kB Do you want to receive files?").arg( e.attribute("c") ).arg( QVariant( e.attribute("s") ).toInt()/1024 );

	ui.label->setText(msg);

	ui.dirLineEdit->setText( settings->profileValue("files/receivedir").toString() );
	if( ui.dirLineEdit->text().isEmpty() )
		ui.dirLineEdit->setText( QDir::homePath() );

	connect(ui.ignorePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.receivePushButton, SIGNAL(clicked()), this, SLOT(receiveF()));
	connect(ui.dontReceivePushButton, SIGNAL(clicked()), this, SLOT(dontReceiveF()));
	connect(ui.changeDirPushButton, SIGNAL(clicked()), this, SLOT(changeDownloadDir()));
}

void fileIncomingDialog::receiveF() {
	emit receive(rndid, sender, TRUE);
	close();
}

void fileIncomingDialog::dontReceiveF() {
	emit receive(rndid, sender, FALSE);
	close();
}

void fileIncomingDialog::changeDownloadDir() {
}