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

#include "filetransfer.h"

fileTransferDialog::fileTransferDialog(QWidget *parent, bool receiveMode): QDialog(parent) {
	buttonLay = new QHBoxLayout();
	buttonLay->addStretch();
	abort = new QPushButton(tr("Abort"));

	if(!receiveMode) {
		addFile = new QPushButton(tr("Add file to send"));
		clearList = new QPushButton(tr("Clear list"));
		send = new QPushButton(tr("Send"));
		buttonLay->addWidget(addFile);
		buttonLay->addWidget(clearList);
		buttonLay->addWidget(send);
	}
	buttonLay->addWidget(abort);
	layout()->addItem(buttonLay);
}
