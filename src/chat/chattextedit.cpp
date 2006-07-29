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

#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QScrollBar>

#include "chattextedit.h"

chatTextEdit::chatTextEdit( QWidget* parent ): QTextBrowser( parent ) {
	setReadOnly(FALSE);
	displayM=new QMenu();
	inputM=new QMenu();
	selectAllA=new QAction(tr("Select All"), this);
	copyA=new QAction(tr("Copy"), this);
	cutA=new QAction(tr("Cut"), this);
	pasteA=new QAction(tr("Paste"), this);
	clearA=new QAction(tr("Clear"), this);
	redoA=new QAction(tr("Redo"), this);
	undoA=new QAction(tr("Undo"), this);

	displayM->addAction(copyA);
	displayM->addAction(selectAllA);

	inputM->addAction(undoA);
	inputM->addAction(redoA);
	inputM->addSeparator();
	inputM->addAction(cutA);
	inputM->addAction(copyA);
	inputM->addAction(pasteA);
	inputM->addAction(clearA);
	inputM->addSeparator();
	inputM->addAction(selectAllA);

	copyA->setEnabled(FALSE);
	cutA->setEnabled(FALSE);
	pasteA->setDisabled(QApplication::clipboard()->text().isEmpty());
	redoA->setEnabled(FALSE);
	undoA->setEnabled(FALSE);

	connect(this, SIGNAL(copyAvailable(bool)), copyA, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(copyAvailable(bool)), cutA, SLOT(setEnabled(bool)));
	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(checkPaste()));

	connect(this, SIGNAL(textChanged()), this, SLOT(checkActions()));
	connect(this, SIGNAL(redoAvailable(bool)), redoA, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(undoAvailable(bool)), undoA, SLOT(setEnabled(bool)));

	connect(selectAllA, SIGNAL(triggered()), this, SLOT(selectAll()));
	connect(copyA, SIGNAL(triggered()), this, SLOT(copy()));
	connect(cutA, SIGNAL(triggered()), this, SLOT(cut()));
	connect(pasteA, SIGNAL(triggered()), this, SLOT(paste()));
	connect(undoA, SIGNAL(triggered()), document(), SLOT(undo()));
	connect(redoA, SIGNAL(triggered()), document(), SLOT(redo()));
}

void chatTextEdit::scrollToBottom() {
	verticalScrollBar()->setValue( verticalScrollBar()->maximum() );
}

void chatTextEdit::contextMenuEvent( QContextMenuEvent* e ) {
	if(isReadOnly())
		displayM->popup(e->globalPos());
	else
		inputM->popup(e->globalPos());
}

void chatTextEdit::checkPaste() {
	if(QApplication::clipboard()->text().isEmpty())
		pasteA->setEnabled(FALSE);
	else
		pasteA->setEnabled(TRUE);
}

void chatTextEdit::checkActions() {
	selectAllA->setDisabled(toPlainText().isEmpty());
	clearA->setDisabled(toPlainText().isEmpty());
}

void chatTextEdit::append(const QString &string) {
	QTextEdit::append(string);
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
