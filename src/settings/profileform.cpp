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

#include <QMessageBox>

#include "profileform.h"
#include "settings.h"

profileForm::profileForm( QWidget *parent): QDialog( parent ) {
	ui.setupUi(this);
	setWindowTitle(tr("Create new profile"));

	ui.passwordLineEdit->setEchoMode(QLineEdit::Password);

	connect(ui.createPushButton, SIGNAL(clicked()), this, SLOT(profileConfirm()));
	connect(ui.cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
}

void profileForm::profileConfirm() {
	if(ui.loginLineEdit->text().isEmpty() || ui.passwordLineEdit->text().isEmpty()) {
		QMessageBox::warning(this, tr("Missing fields"), tr("You have to fill up at least \"Login\" and \"Password\" fields."), tr("Ok"));
		return;
	}

	if(ui.profileLineEdit->text().isEmpty())
		ui.profileLineEdit->setText(ui.loginLineEdit->text());

	if(settings->defaultValue("profiles/list").value<QStringList>().contains(ui.profileLineEdit->text(), Qt::CaseInsensitive)) {
		QMessageBox::warning(this, tr("Profile exists"), tr("Porfile \"%1\" already exists. Pick another profile name.").arg(ui.profileLineEdit->text()), tr("Ok"));
		return;
	}

	emit addProfile(ui.profileLineEdit->text(), ui.loginLineEdit->text(), ui.passwordLineEdit->text());
	close();
}
