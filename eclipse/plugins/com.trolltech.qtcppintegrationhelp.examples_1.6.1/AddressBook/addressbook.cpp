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

#include "addressbook.h"
#include "adddialog.h"

AddressBook::AddressBook(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
}

AddressBook::~AddressBook()
{

}

void AddressBook::on_addButton_clicked()
{
	AddDialog dialog(this);
	if (dialog.exec()) {
	    QString name = dialog.name();
	    QString email = dialog.email();

		if (!name.isEmpty() && !email.isEmpty()) {
			QListWidgetItem *item = new QListWidgetItem(name, ui.addressList);
			item->setData(Qt::UserRole, email);
			ui.addressList->setCurrentItem(item);
		}
	}    
}

void AddressBook::on_addressList_currentItemChanged()
{
	QListWidgetItem *curItem = ui.addressList->currentItem();

	if (curItem) {
		ui.nameLabel->setText("Name: " + curItem->text());
		ui.emailLabel->setText("Email: " + curItem->data(Qt::UserRole).toString());
	} else {
    	ui.nameLabel->setText("<No item selected>");
        ui.emailLabel->clear();
    }
}

void AddressBook::on_deleteButton_clicked()
{
	QListWidgetItem *curItem = ui.addressList->currentItem();

	if (curItem) {
		int row = ui.addressList->row(curItem);
		ui.addressList->takeItem(row);
		delete curItem;

		if (ui.addressList->count() > 0)
			ui.addressList->setCurrentRow(0);
		else
		    on_addressList_currentItemChanged();
	}	
}
