/****************************************************************************
** Meta object code from reading C++ file 'detectdialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "detectdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'detectdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DetectDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      40,   13,   13,   13, 0x08,
      74,   54,   13,   13, 0x08,
     122,  116,   13,   13, 0x08,
     159,   13,   13,   13, 0x08,
     171,   13,   13,   13, 0x08,
     185,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DetectDialog[] = {
    "DetectDialog\0\0on_browseButton_clicked()\0"
    "detectImage()\0exitCode,exitStatus\0"
    "processFinished(int,QProcess::ExitStatus)\0"
    "error\0processError(QProcess::ProcessError)\0"
    "setBrowse()\0setOkButton()\0rdEffectCase()\0"
};

void DetectDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DetectDialog *_t = static_cast<DetectDialog *>(_o);
        switch (_id) {
        case 0: _t->on_browseButton_clicked(); break;
        case 1: _t->detectImage(); break;
        case 2: _t->processFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 3: _t->processError((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 4: _t->setBrowse(); break;
        case 5: _t->setOkButton(); break;
        case 6: _t->rdEffectCase(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DetectDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DetectDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DetectDialog,
      qt_meta_data_DetectDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DetectDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DetectDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DetectDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DetectDialog))
        return static_cast<void*>(const_cast< DetectDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int DetectDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
