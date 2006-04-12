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

#include <QPainter>

#include "roster_delegate.h"
#include "roster_view.h"

rosterDelegate::rosterDelegate( QObject *parent ): QItemDelegate( parent ) {
}

void rosterDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
	if (option.state & QStyle::State_Selected)
		painter->setBrush(option.palette.highlight());
	else if(index.model()->data(index,rosterView::TypeRole)==2) // GROUP
		painter->setBrush(option.palette.alternateBase());
	else
		painter->setBrush(option.palette.base());

	int y=option.rect.y();
	QSize sh=sizeHint(option,index);

	painter->setPen(Qt::NoPen);
	painter->drawRect(option.rect.x(),option.rect.y(), sh.width()+2, sh.height());

	if (option.state & QStyle::State_Selected)
		painter->setPen(option.palette.highlightedText().color());
	else
		painter->setPen(option.palette.windowText().color());

	QImage img=index.model()->data(index, Qt::DecorationRole).value<QImage>();
	painter->drawImage(QRect(option.rect.x()+1, option.rect.y(), img.width(), img.height()), img);

	if(index.model()->data(index,rosterView::TypeRole)==0) {
		painter->drawText(QRect(option.rect.x()+img.width()+1, y, sh.width()-img.width()-1, option.fontMetrics.height()), Qt::AlignLeft|Qt::AlignTop, elidedText(option.fontMetrics, sh.width()-img.width()-1,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString())); // single line elided text
		if(!(index.model()->data(index, rosterView::DescriptionRole).toString()).isEmpty()) {
			QFont pf=painter->font();
			pf.setItalic(TRUE);
			painter->setFont(pf);
			painter->drawText(QRect(option.rect.x()+2, y+img.height(), sh.width()-2, sh.height()-img.height()), Qt::TextWrapAnywhere|Qt::AlignLeft|Qt::AlignTop, index.model()->data(index, rosterView::DescriptionRole).toString()); // multiline text
			pf.setItalic(FALSE);
			painter->setFont(pf);
		}
	}
	else if(index.model()->data(index,rosterView::TypeRole)==2) {
		QFont pf=painter->font();
		pf.setBold(TRUE);
		QFontMetrics fm(pf);
		painter->setFont(pf);
		painter->drawText(QRect(option.rect.x()+img.width()+1, y, sh.width()-img.width()-1, fm.height()), Qt::AlignLeft|Qt::AlignTop, elidedText(fm, sh.width()-img.width()-1,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString()+" ("+index.model()->data(index, rosterView::AvailCountRole).toString()+"/"+index.model()->data(index, rosterView::ContactCountRole).toString()+")")); // single line elided text
		pf.setBold(FALSE);
		painter->setFont(pf);
	}
	else if(index.model()->data(index,rosterView::TypeRole)==3) {
		QFont pf=painter->font();
		pf.setBold(TRUE);
		QFontMetrics fm(pf);
		painter->setFont(pf);
		painter->setPen(option.palette.dark().color());
		painter->drawText(QRect(option.rect.x()+11, y, sh.width()-11, fm.height()+4), Qt::AlignLeft|Qt::AlignTop, elidedText(fm, sh.width()-11,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString())); // single line elided text
		pf.setBold(FALSE);
		painter->setFont(pf);
	}
}

QSize rosterDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
		QFont f=option.font;
		f.setItalic(TRUE);
		QFontMetrics fm(f);
		int descr=fm.boundingRect(0,0,width-4, 1000,  Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere ,index.model()->data(index, rosterView::DescriptionRole).toString()).height();
		int row=qMax(option.fontMetrics.height(), (index.model()->data(index, Qt::DecorationRole).value<QImage>()).height());
		return QSize(width-2,row+((index.model()->data(index,rosterView::DescriptionRole).toString()).isEmpty()?0:descr));
}

void rosterDelegate::setWidth( int i ) {
	width=i;
}
