/*********************************************************************************
**
** This file is part of Qt Eclipse Integration
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation (qt-info@nokia.com)
**
** Windows(R) users may use this file under the terms of the Qt Eclipse
** Plug In License Agreement Version 1.0 as attached in the LICENSE.TXT file.
**
** Linux(R) users may use this file under the terms of the GNU Lesser
** General Public License Agreement version 2.1 as shown in the LGPL-2_1.TXT file.
**
**********************************************************************************/

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <QtGui/QWidget>
#include "ui_addressbook.h"

class AddressBook : public QWidget
{
    Q_OBJECT

public:
    AddressBook(QWidget *parent = 0);
    ~AddressBook();

private slots:
	void on_addButton_clicked();
	void on_addressList_currentItemChanged();
	void on_deleteButton_clicked();

private:
    Ui::AddressBookClass ui;
};

#endif // ADDRESSBOOK_H
