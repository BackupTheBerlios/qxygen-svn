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

#include <QHelpEvent>
#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QMenu>

#include "roster_widget.h"
#include "roster_view.h"
#include "roster_delegate.h"
#include "roster_tiplabel.h"

rosterTipLabel *rosterTipLabel::instance=0;

rosterWidget::rosterWidget(QObject *parent): QTreeView((QWidget*)parent)
{
	setUniformRowHeights(FALSE);
	header()->setHidden(TRUE);
}

rosterWidget::~rosterWidget()
{
}

void rosterWidget::resizeEvent(QResizeEvent *event)
{
	((rosterDelegate*)itemDelegate())->setWidth(viewport()->width());
	((rosterView*)model())->emitLayoutChanged();
	QTreeView::resizeEvent(event);
}

bool rosterWidget::viewportEvent(QEvent *event)
{
	switch(event->type())
	{
	case QEvent::ToolTip:
		if(isActiveWindow())
		{
			QHelpEvent *e=static_cast<QHelpEvent*>(event);
			QModelIndex idx=indexAt(e->pos());
			if(idx.isValid())
			{
				paintToolTip(idx,e->globalPos(),this);
				return true;
			}
		}
		return false;
	break;

	default:
		return QTreeView::viewportEvent(event);
	break;
	}
	return QTreeView::viewportEvent(event);
}

void rosterWidget::paintToolTip(const QModelIndex &idx, const QPoint &pos, QWidget *w)
{
	if(rosterTipLabel::instance && rosterTipLabel::instance->index()==idx)
		return;

	int scr;
	if (QApplication::desktop()->isVirtualDesktop())
		scr = QApplication::desktop()->screenNumber(pos);
	else
		scr = QApplication::desktop()->screenNumber(w);

#ifdef Q_WS_MAC
	QRect screen = QApplication::desktop()->availableGeometry(scr);
#else
	QRect screen = QApplication::desktop()->screenGeometry(scr);
#endif

    QLabel *label = new rosterTipLabel(idx, QApplication::desktop()->screen(scr));

	QPoint p = pos;

	p += QPoint(2,
#ifdef Q_WS_WIN
			24
#else
			16
#endif
	);

	QSize sh=label->sizeHint();

	if (p.x() + sh.width() > screen.x() + screen.width())
		p.rx() -= 4 + sh.width();
	if (p.y() + sh.height() > screen.y() + screen.height())
		p.ry() -= 24 + sh.height();
	if (p.y() < screen.y())
		p.setY(screen.y());
	if (p.x() + sh.width() > screen.x() + screen.width())
		p.setX(screen.x() + screen.width() - sh.width());
	if (p.x() < screen.x())
		p.setX(screen.x());
	if (p.y() + sh.height() > screen.y() + screen.height())
		p.setY(screen.y() + screen.height() - sh.height());

	label->move(p);
	label->show();
}

rosterTipLabel::rosterTipLabel(const QModelIndex &index,QWidget *parent): QLabel(parent, Qt::ToolTip)
{
	delete instance;
	instance=this;

	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft);

	qApp->installEventFilter(this);
	hideTimer.start(10000, this);
	img=(index.model()->data(index,rosterView::FotoRole)).value<QImage>();
	status=(index.model()->data(index,Qt::DecorationRole)).value<QImage>();
	nick=(index.model()->data(index,Qt::DisplayRole)).toString();
	if(index.model()->data(index,rosterView::SubscriptionRole).toString()=="none")
		nick+=" "+tr("(No authorization)");
	else if(index.model()->data(index,rosterView::SubscriptionRole).toString()=="subscribe")
		nick+=" "+tr("(Awaiting authorization)");

	descr=(index.model()->data(index,rosterView::DescriptionForTooltipRole)).toString();
	idx=index;

	QPalette pal(	QColor(0,0,0) /*windowText*/,
			QColor(255,255,220,255) /*button*/,
			QColor(255,255,220,255) /*light*/,
			QColor(255,255,220,255) /*dark*/,
			QColor(255,255,220,255) /*mid*/,
			QColor(255,255,220,255) /*text*/,
			QColor(255,255,220,255) /*bright_text*/,
			QColor(255,255,220,255)/*base*/,
			QColor(255,255,220)/*window*/);

	setPalette(pal);
}

rosterTipLabel::~rosterTipLabel()
{
	instance=0;
}

bool rosterTipLabel::eventFilter(QObject *, QEvent *e)
{
	switch (e->type())
	{
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	{
		int key = static_cast<QKeyEvent*>(e)->key();
		Qt::KeyboardModifiers mody = static_cast<QKeyEvent*>(e)->modifiers();

		if((mody & Qt::KeyboardModifierMask) || (key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta))
			break;
	}
	case QEvent::Leave:
	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::FocusIn:
	case QEvent::FocusOut:
		hideTip();
	default:
		;
	}
	return false;
}

QSize rosterTipLabel::sizeHint () const
{
	QFontMetrics fm(font());

	if(idx.model()->data(idx,rosterView::TypeRole).toInt()>1)
		return QSize(fm.width(nick)+4, fm.height()+4);

	int imgh=img.height();
	int texth=0;
	texth+=fm.height()+5+qMax(fm.height(),status.height()); // DEFAULT DISPLAYED DATA - NICK AND STATUS

	if(!descr.isEmpty())
		texth+=5+fm.boundingRect(0,0,400-img.width()-(img.height()?15:10),1000,Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere,descr).height();

	return QSize(400, qMax(texth,imgh)+10);
}

void rosterTipLabel::hideTip()
{
	hide();
	// timer based deletion to prevent animation
	deleteTimer.start(250, this);
}

void rosterTipLabel::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == hideTimer.timerId())
		hideTip();
	else if (e->timerId() == deleteTimer.timerId())
		delete this;
}

void rosterTipLabel::paintEvent(QPaintEvent */*e*/)
{
	QSize sh=sizeHint();
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(palette().windowText());
	painter.drawRect(QRect(QPoint(0,0),sh));
	painter.setBrush(palette().window());
	painter.drawRect(QRect(1,1,sh.width()-2,sh.height()-2));
	painter.setPen(palette().windowText().color());

	if(idx.model()->data(idx,rosterView::TypeRole).toInt()>1)
	{
		painter.drawText(2,2,sh.width()-2,sh.height()-2,Qt::AlignTop|Qt::AlignLeft, nick);
	}
	else
	{
		if(img.height())
			painter.drawImage(QRect(5,5,img.width(),img.height()),img);

		QFontMetrics fm(font());

		painter.drawText(img.width()+(img.height()?10:5), 5, 400-img.width()-(img.height()?15:10), fm.height(), Qt::AlignTop|Qt::AlignLeft,nick);

		// STATUS
		painter.drawText(img.width()+(img.height()?10:5), fm.height()+10, 400-img.width()-(img.height()?15:10), qMax(fm.height(),status.height()), Qt::AlignVCenter|Qt::AlignLeft, tr("Status:"));

		if(fm.height()<=status.height())
			painter.drawImage(QPoint(img.width()+(img.height()?10:5)+fm.width(tr("Status:"))+5, fm.height()+10), status);
		else
			painter.drawImage(QPoint(img.width()+(img.height()?10:5)+fm.width(tr("Status:"))+5, fm.height()+10+((fm.height()-status.height())/2)), status);

		painter.drawText(img.width()+(img.height()?10:5)+fm.width(tr("Status:"))+5+status.width()+5, fm.height()+10, 100, qMax(fm.height(),status.height()), Qt::AlignVCenter|Qt::AlignLeft, idx.model()->data(idx,rosterView::StatusRole).toString());
		// STATUS END

		if(!descr.isEmpty())
			painter.drawText(img.width()+(img.height()?10:5), 5 /*margin*/+fm.height()/*nick height*/+5/*spacing*/+qMax(fm.height(),status.height())/*status line height*/+5/*spacing*/, 400-img.width()-(img.height()?15:10), fm.boundingRect(0,0,400-img.width()-(img.height()?15:10),1000,Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere,descr).height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWrapAnywhere, descr);
	}
}

void rosterWidget::contextMenuEvent(QContextMenuEvent *e)
{
	QModelIndex idx=indexAt(e->pos());
	if(idx.isValid() && idx.model()->data(idx,rosterView::TypeRole)==0)
		menu->popup(e->globalPos());
}
