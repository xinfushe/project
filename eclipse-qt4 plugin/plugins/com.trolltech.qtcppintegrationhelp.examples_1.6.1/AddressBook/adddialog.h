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

#ifndef ADDDIALOG_H_
#define ADDDIALOG_H_

#include "ui_adddialog.h"

class AddDialog : public QDialog
{
	Q_OBJECT

public:
	AddDialog(QWidget *parent = 0);
	~AddDialog();

	QString name() { return ui.nameEdit->text(); }
	QString email() { return ui.emailEdit->text(); }
private:
    Ui::AddDialogClass ui;
};

#endif /*ADDDIALOG_H_*/
