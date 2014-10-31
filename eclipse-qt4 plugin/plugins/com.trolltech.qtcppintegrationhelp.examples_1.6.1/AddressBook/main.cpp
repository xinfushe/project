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

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AddressBook w;
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
