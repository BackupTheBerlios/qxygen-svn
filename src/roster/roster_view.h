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

#ifndef ROSTER_VIEW_H
#define ROSTER_VIEW_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QIcon>

class rosterItem;

class rosterView : public QAbstractItemModel
{
Q_OBJECT

public:
	enum {	DescriptionRole=32,
		ExpandRole=33,
		FotoRole=34,
		TypeRole=35,
		StatusRole=36,
		ChildCountRole=37,
		AvailCountRole=38,
		ContactCountRole=39,
		JidRole=40,
		SubscriptionRole=41,
		DescriptionForTooltipRole=42};
	rosterView(QObject *parent = 0);
	~rosterView();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	rosterItem *itemAt(int row);

	QList<rosterItem*> groupList();
	QStringList groupNames();

	bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	void setShowDescr(bool d);
	void setSubgroups(bool s);

	void addGroup(QString name,bool sort=TRUE);

	void emitLayoutChanged();

	rosterItem *find(QString);

	void checkGroup(rosterItem*);

public slots:
	void clearRoster();
	void removeItem(QString);
	void addItem(QString jid, QString dn, QString subscription, QString group,bool sort=FALSE);
	void presenceUpdate(QString from, QString status, QString descr);
	void presenceDisconnected();
	void sortRoster();

signals:
	void windowUpdate(QString, QIcon);
	void groupAdded(const QModelIndex &idx);

private:
	rosterItem *rootItem;
	bool descr, subgroups;
};

extern rosterView *rosterModel;
#endif
