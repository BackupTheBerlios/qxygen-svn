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

#include "descrdialog.h"

descrDialog::descrDialog(QString status, QString descr, QWidget *parent): QDialog(parent) {
	ui.setupUi(this);
	ui.availRadio->setIcon(QIcon(":onlinei.png"));
	ui.chattyRadio->setIcon(QIcon(":chattyi.png"));
	ui.awayRadio->setIcon(QIcon(":awayi.png"));
	ui.unavailableRadio->setIcon(QIcon(":unavailablei.png"));
	ui.dndRadio->setIcon(QIcon(":dndi.png"));
	ui.invisRadio->setIcon(QIcon(":invisiblei.png"));
	ui.offlineRadio->setIcon(QIcon(":offlinei.png"));

	if(status=="available")
		ui.availRadio->setChecked(TRUE);
	else if(status=="chatty")
		ui.chattyRadio->setChecked(TRUE);
	else if(status=="away")
		ui.awayRadio->setChecked(TRUE);
	else if(status=="xa")
		ui.unavailableRadio->setChecked(TRUE);
	else if(status=="dnd")
		ui.dndRadio->setChecked(TRUE);
	else if(status=="invisible")
		ui.invisRadio->setChecked(TRUE);
	else if(status=="unavailable")
		ui.offlineRadio->setChecked(TRUE);

	ui.descrTextEdit->setPlainText(descr);
	ui.descrTextEdit->setFocus();
	if(descr.isEmpty())
		ui.okPushButton->setEnabled(FALSE);

	connect(ui.descrTextEdit, SIGNAL(textChanged()), this, SLOT(enableButton()));
	connect(ui.okPushButton, SIGNAL(clicked()), this, SLOT(setDescr()));
	connect(ui.cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
}

void descrDialog::enableButton() {
	if(ui.descrTextEdit->toPlainText().isEmpty())
		ui.okPushButton->setEnabled(FALSE);
	else
		ui.okPushButton->setEnabled(TRUE);
}

void descrDialog::setDescr() {
	QString status;
	if(ui.availRadio->isChecked())
		status="available";
	else if(ui.chattyRadio->isChecked())
		status="chatty";
	else if(ui.awayRadio->isChecked())
		status="away";
	else if(ui.unavailableRadio->isChecked())
		status="xa";
	else if(ui.dndRadio->isChecked())
		status="dnd";
	else if(ui.invisRadio->isChecked())
		status="invisible";
	else if(ui.offlineRadio->isChecked())
		status="unavailable";

	emit statusChanged(status, ui.descrTextEdit->toPlainText());
	close();
}
