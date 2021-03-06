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

#include <QCoreApplication>
#include <QTranslator>

#include "qxygen.h"

int main( int argc, char *argv[] ) {
	QApplication app(argc, argv);
	QTranslator trans;
	QString lang=qxygen::getLanguage();
	if(!lang.isEmpty()) {
		trans.load(":/translation/"+lang);
		app.installTranslator(&trans);
	}

	new qxygen;
	Qxygen->show();
	QApplication::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
	return app.exec();
}
