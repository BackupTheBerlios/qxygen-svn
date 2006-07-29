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
#include "settings.h"

rosterDelegate::rosterDelegate( QObject *parent ): QItemDelegate( parent ) {
}

void rosterDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
	if (option.state & QStyle::State_Selected)
		painter->setBrush(option.palette.highlight());
	else if(index.model()->data(index,rosterView::TypeRole)==2) // GROUP
		painter->setBrush(option.palette.alternateBase());
	else
		painter->setBrush(option.palette.base());

	QSize sh=sizeHint(option,index);

	painter->setPen(Qt::NoPen);
	painter->drawRect(option.rect.x(),option.rect.y(), sh.width()+2, sh.height());

	if (option.state & QStyle::State_Selected)
		painter->setPen(option.palette.color(QPalette::HighlightedText));
	else
		painter->setPen(option.palette.color(QPalette::Text));

	if(index.model()->data(index,rosterView::TypeRole)==0) {
		QImage img=index.model()->data(index, Qt::DecorationRole).value<QImage>();
		painter->setFont( settings->profileValue("look/font/nick").value<QFont>() );
		QFontMetrics fm( painter->font() );

		int yoffset;

		fm.height()>img.height()?yoffset=(fm.height()-img.height())/2:yoffset=(img.height()-fm.height())/2;

		painter->drawImage(QRect(option.rect.x()+1, option.rect.y()+(fm.height()>img.height()?yoffset:0), img.width(), img.height()), img);
		painter->drawText(QRect(option.rect.x()+img.width()+1, option.rect.y()+(fm.height()>img.height()?0:yoffset), sh.width()-img.width()-1, fm.height()), Qt::AlignLeft|Qt::AlignTop, elidedText(fm, sh.width()-img.width()-1,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString())); // single line elided text
		if(!(index.model()->data(index, rosterView::DescriptionRole).toString()).isEmpty()) {
			painter->setFont( settings->profileValue("look/font/descr").value<QFont>() );

			if (option.state & QStyle::State_Selected)
				painter->setPen(option.palette.color(QPalette::HighlightedText));
			else
				painter->setPen(option.palette.color(QPalette::LinkVisited));

			painter->drawText(QRect(option.rect.x()+2, option.rect.y()+qMax(img.height(),fm.height()), sh.width()-2, sh.height()-img.height()), Qt::TextWrapAnywhere|Qt::AlignLeft|Qt::AlignTop, index.model()->data(index, rosterView::DescriptionRole).toString()); // multiline text
		}
	}
	else if(index.model()->data(index,rosterView::TypeRole)==2) {
		QImage img=index.model()->data(index, Qt::DecorationRole).value<QImage>();
		painter->setFont( settings->profileValue("look/font/group").value<QFont>() );
		QFontMetrics fm( painter->font() );

		int yoffset;

		fm.height()>img.height()?yoffset=(fm.height()-img.height())/2:yoffset=(img.height()-fm.height())/2;

		painter->drawImage(QRect(option.rect.x()+1, option.rect.y()+(fm.height()>img.height()?yoffset:0), img.width(), img.height()), img);
		painter->drawText(QRect(option.rect.x()+img.width()+1, option.rect.y()+(fm.height()>img.height()?0:yoffset), sh.width()-img.width()-1, fm.height()), Qt::AlignLeft|Qt::AlignTop, elidedText(fm, sh.width()-img.width()-1,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString()+" ("+index.model()->data(index, rosterView::AvailCountRole).toString()+"/"+index.model()->data(index, rosterView::ContactCountRole).toString()+")")); // single line elided text
	}
	else if(index.model()->data(index,rosterView::TypeRole)==3) {
		painter->setFont( settings->profileValue("look/font/subgroup").value<QFont>() );
		QFontMetrics fm( painter->font() );
		painter->setPen(option.palette.color(QPalette::Dark));
		painter->drawText(QRect(option.rect.x()+11, option.rect.y(), sh.width()-11, fm.height()+4), Qt::AlignLeft|Qt::AlignTop, elidedText(fm, sh.width()-11,Qt::ElideRight, index.model()->data(index, Qt::DisplayRole).toString())); // single line elided text
	}
}

QSize rosterDelegate::sizeHint( const QStyleOptionViewItem& opt, const QModelIndex &index ) const {
	int height=0,descr=0;
	QFontMetrics fm(opt.fontMetrics);
	QString key;

	switch ( index.model()->data(index,rosterView::TypeRole).toInt() ) {
	case 0:
		//contact sizehint:
		key="look/font/nick";
		fm=QFontMetrics( settings->profileValue("look/font/descr").value<QFont>() );
		descr=fm.boundingRect(0,0,width-4, 1000,  Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere ,index.model()->data(index, rosterView::DescriptionRole).toString()).height();
		height+=((index.model()->data(index,rosterView::DescriptionRole).toString()).isEmpty()?0:descr);
		break;

	case 2:
		key="look/font/group";
		break;

	case 3:
		key="look/font/subgroup";
		break;
	}

	fm=QFontMetrics( settings->profileValue(key).value<QFont>() );
	height+=qMax(fm.height(), (index.model()->data(index, Qt::DecorationRole).value<QImage>()).height());

	return QSize(width-2,height);
/*
		QFont f=option.font;
		f.setItalic(TRUE);
		QFontMetrics fm(f);
		int descr=fm.boundingRect(0,0,width-4, 1000,  Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere ,index.model()->data(index, rosterView::DescriptionRole).toString()).height();
		int row=qMax(option.fontMetrics.height(), (index.model()->data(index, Qt::DecorationRole).value<QImage>()).height());
		return QSize(width-2,row+((index.model()->data(index,rosterView::DescriptionRole).toString()).isEmpty()?0:descr));
*/
}

void rosterDelegate::setWidth( int i ) {
	width=i;
}
