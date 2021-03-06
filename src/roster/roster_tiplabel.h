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

#ifndef ROSTER_TIPLABEL_H
#define ROSTER_TIPLABEL_H

#include <QLabel>
#include <QBasicTimer>
#include <QImage>
#include <QModelIndex>

class rosterTipLabel: public QLabel {
Q_OBJECT

public:
	rosterTipLabel(const QModelIndex &idx,QWidget *parent);
	~rosterTipLabel();
	static rosterTipLabel *instance;

	bool eventFilter(QObject *, QEvent *);

	QBasicTimer hideTimer, deleteTimer;

	QModelIndex index(){return idx;}

	QSize sizeHint () const;
	void hideTip();
protected:
	QImage img,status;
	QString nick,descr;

	QModelIndex idx;
	void enterEvent(QEvent*){hideTip();}
	void timerEvent(QTimerEvent *e);
	void paintEvent(QPaintEvent *e);
};
#endif
