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

#include <QtGui>

#include "roster_view.h"

rosterView *rosterModel=0;

rosterView::rosterView( QObject *parent ): QAbstractItemModel( parent ) {
	rootItem = new rosterItem();
	subgroups=FALSE;
	descr=FALSE;
}

rosterView::~rosterView() {
	delete rootItem;
}

int rosterView::columnCount( const QModelIndex &parent ) const {
	if (parent.isValid())
			return static_cast<rosterItem*>(parent.internalPointer())->columnCount();
		else
			return rootItem->columnCount();
}

QVariant rosterView::data( const QModelIndex &index, int role ) const {
	if (!index.isValid())
		return QVariant();

	rosterItem *item = static_cast<rosterItem*>(index.internalPointer());

	if( role == Qt::DisplayRole )
		return item->data(0);
	else if( role == TypeRole )
		return item->type();
	else if( role == FotoRole )
		return QImage(":/nofoto.png");
	else if( role == Qt::DecorationRole )
		return item->icon();
	else if( role == rosterView::StatusRole ) {
		QString s=item->data(4);
		if(s=="available")
			return tr("Available");
		else if(s=="chat")
			return tr("Free for Chat");
		else if(s=="away")
			return tr("Away");
		else if(s=="xa")
			return tr("Not Available");
		else if(s=="dnd")
			return tr("Do not Disturb");

		return tr("Unavailable");
	}
	else if( role == DescriptionRole ) {
		if(descr)
			return item->data(1);
		else
			return QVariant();
	}
	else if( role == DescriptionForTooltipRole )
		return item->data(1);
	else if( role == ChildCountRole )
		return item->childCount();
	else if( role == ContactCountRole )
		return item->contactCount();
	else if( role == AvailCountRole )
		return item->availCount();
	else if( role == JidRole )
		return item->data(2);
	else if( role == SubscriptionRole )
		return item->data(3);

	return QVariant();
}

Qt::ItemFlags rosterView::flags( const QModelIndex &index ) const {
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	switch(data(index,rosterView::TypeRole).toInt()) {
	case rosterItem::Contact: return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	case rosterItem::Group: return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	return 0;
}

QVariant rosterView::headerData( int, Qt::Orientation, int ) const {
	return QVariant();
}

QModelIndex rosterView::index( int row, int column, const QModelIndex &parent ) const {
    rosterItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<rosterItem*>(parent.internalPointer());

    rosterItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex rosterView::parent( const QModelIndex &index ) const {
    if (!index.isValid())
        return QModelIndex();

    rosterItem *childItem = static_cast<rosterItem*>(index.internalPointer());
    rosterItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int rosterView::rowCount( const QModelIndex &parent ) const {
    rosterItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<rosterItem*>(parent.internalPointer());

    return parentItem->childCount();
}

rosterItem* rosterView::itemAt( int row ) {
	return rootItem->child(row);
}

void rosterView::addGroup( QString name,bool sort ) {
	if(rootItem->child(name))
		return;

	if(name.isEmpty())
		name=tr("Contacts");

	rosterItem *group=new rosterItem(name, name, rosterItem::Group, rootItem);

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	rootItem->appendChild(group);
	endInsertRows();

	emit groupAdded(index(group->row(),0));
	if(sort)
		rootItem->sort();
	emit layoutChanged();
}

void rosterView::addItem( QString jid, QString dn, QString subscription, QString group,bool sort ) {
	if(!rootItem->child(group))
		addGroup(group,sort);

	rosterItem *parent=rootItem->child(group);

	if(rosterItem *item=find(jid)) {
		if(item->data(0)==dn && (item->data(3)==subscription || !subgroups))
			sort=FALSE;

		if(item->parent()->data(0)!=group) {
			//remove and create new in other group
			item->parent()->takeChild(item);
			item->parent()->sort(subgroups);
			addItem(jid,dn,subscription,group,sort);
			sort=TRUE;
		}
		else {
			//update existing item
			if(item->data(0)!=dn) {
				item->setName(dn);
				sort=TRUE;
			}

			if(item->data(3)!=subscription) {
				if(subscription=="none") {
					item->setStatus("unavailable");
					item->setDescr("");
				}
				item->setSubscription(subscription);
			}
		}
	}
	else {
		beginInsertRows(index(parent->row(),0),parent->childCount(),parent->childCount());
		parent->appendChild(new rosterItem(jid, dn, rosterItem::Contact, parent, subscription));
		endInsertRows();
	}

	if(sort)
		parent->sort(subgroups);

	emit layoutChanged();
}

void rosterView::setShowDescr( bool d ) {
	if(d==descr)
		return;

	descr=d;
	emit layoutChanged();
}

void rosterView::setSubgroups( bool s ) {
	if(s==subgroups)
		return;

	subgroups=s;

	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	while(gi.hasNext()) {
		rosterItem *group=gi.next();
		group->sort(subgroups);
	}
	emit layoutChanged();
}

bool rosterView::setData( const QModelIndex &index, const QVariant &value, int role ) {
	if( role==rosterView::ExpandRole ) {
		rosterItem *item = static_cast<rosterItem*>(index.internalPointer());
		item->setExpanded(value.toBool());
		return true;
	}

	return false;
}

void rosterView::emitLayoutChanged() {
	emit layoutAboutToBeChanged();
	emit layoutChanged();
}

QList<rosterItem*> rosterView::groupList() {
	return rootItem->childList();
}

void rosterView::clearRoster() {
	QList<rosterItem*> current=groupList();
	QListIterator<rosterItem*> i(current);
	while(i.hasNext())
		rootItem->takeChild(i.next());
}

void rosterView::presenceUpdate( QString from, QString status, QString descr ) {
	rosterItem *item=find(from);

	if( !item )
		return;

	bool sort=TRUE;

	if((item->data(4)=="unavailable" && status=="unavailable") || (item->data(4)!="unavailable" && status!="unavailable"))
		sort=FALSE;

	item->setStatus(status);
	item->setDescr(descr);

	if(sort)
		item->parent()->sort(subgroups);

	emit windowUpdate(from, QIcon(QPixmap::fromImage(item->icon())));
	emit layoutChanged();
}

void rosterView::presenceDisconnected() {
	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	while(gi.hasNext()) {
		rosterItem *g=gi.next();
		QList<rosterItem*> items=g->childList();
		QListIterator<rosterItem*> it(items);
		while(it.hasNext()) {
			rosterItem* item=it.next();
			item->setStatus("unavailable");
			item->setDescr("");
		}
		g->sort(subgroups);
	}
	emit layoutChanged();
}

void rosterView::sortRoster() {
	rootItem->sort();
	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	while(gi.hasNext()) {
		rosterItem *group=gi.next();
		group->sort(subgroups);
		emit groupAdded(index(group->row(),0));
	}
	emit layoutChanged();
}

QStringList rosterView::groupNames() {
	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	QMap<QString,QString> gmap;
	while(gi.hasNext()) {
		QString n=gi.next()->data(0);
		gmap.insert(n.toLower(),n);
	}
	return QStringList(gmap.values());
}

void rosterView::removeItem( QString jid ) {
	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	while(gi.hasNext()) {
		rosterItem *g=gi.next();
		if(rosterItem *item=g->child(jid,2)) {
			g->takeChild(item);
			if(subgroups)
				g->sort(subgroups);
			checkGroup(g);
			emit layoutChanged();
		}
	}
}

void rosterView::checkGroup( rosterItem *group ) {
	if(!group->childCount())
		rootItem->takeChild(group);
}

rosterItem* rosterView::find( QString jid ) {
	QList<rosterItem*> groups=groupList();
	QListIterator<rosterItem*> gi(groups);
	while(gi.hasNext()) {
		if(rosterItem *item=gi.next()->child(jid,2))
			return item;
	}
	return 0;
}
