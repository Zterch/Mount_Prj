/****************************************************************************
** Meta object code from reading C++ file 'udpmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../MountShow/udpmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'udpmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MountShow__UdpReceiverThread_t {
    QByteArrayData data[9];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MountShow__UdpReceiverThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MountShow__UdpReceiverThread_t qt_meta_stringdata_MountShow__UdpReceiverThread = {
    {
QT_MOC_LITERAL(0, 0, 28), // "MountShow::UdpReceiverThread"
QT_MOC_LITERAL(1, 29, 14), // "packetReceived"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 4), // "data"
QT_MOC_LITERAL(4, 50, 8), // "fromAddr"
QT_MOC_LITERAL(5, 59, 8), // "fromPort"
QT_MOC_LITERAL(6, 68, 9), // "setSocket"
QT_MOC_LITERAL(7, 78, 11), // "QUdpSocket*"
QT_MOC_LITERAL(8, 90, 6) // "socket"

    },
    "MountShow::UdpReceiverThread\0"
    "packetReceived\0\0data\0fromAddr\0fromPort\0"
    "setSocket\0QUdpSocket*\0socket"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MountShow__UdpReceiverThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   31,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QByteArray, QMetaType::QString, QMetaType::UShort,    3,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void MountShow::UdpReceiverThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UdpReceiverThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->packetReceived((*reinterpret_cast< const QByteArray(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        case 1: _t->setSocket((*reinterpret_cast< QUdpSocket*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QUdpSocket* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UdpReceiverThread::*)(const QByteArray & , const QString & , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpReceiverThread::packetReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MountShow::UdpReceiverThread::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_MountShow__UdpReceiverThread.data,
    qt_meta_data_MountShow__UdpReceiverThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MountShow::UdpReceiverThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MountShow::UdpReceiverThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MountShow__UdpReceiverThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int MountShow::UdpReceiverThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MountShow::UdpReceiverThread::packetReceived(const QByteArray & _t1, const QString & _t2, quint16 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_MountShow__UdpManager_t {
    QByteArrayData data[14];
    char stringdata0[155];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MountShow__UdpManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MountShow__UdpManager_t qt_meta_stringdata_MountShow__UdpManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "MountShow::UdpManager"
QT_MOC_LITERAL(1, 22, 16), // "connectedChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 9), // "connected"
QT_MOC_LITERAL(4, 50, 14), // "packetReceived"
QT_MOC_LITERAL(5, 65, 4), // "data"
QT_MOC_LITERAL(6, 70, 8), // "fromAddr"
QT_MOC_LITERAL(7, 79, 8), // "fromPort"
QT_MOC_LITERAL(8, 88, 10), // "logMessage"
QT_MOC_LITERAL(9, 99, 3), // "msg"
QT_MOC_LITERAL(10, 103, 5), // "level"
QT_MOC_LITERAL(11, 109, 16), // "heartbeatTimeout"
QT_MOC_LITERAL(12, 126, 11), // "onHeartbeat"
QT_MOC_LITERAL(13, 138, 16) // "onPacketReceived"

    },
    "MountShow::UdpManager\0connectedChanged\0"
    "\0connected\0packetReceived\0data\0fromAddr\0"
    "fromPort\0logMessage\0msg\0level\0"
    "heartbeatTimeout\0onHeartbeat\0"
    "onPacketReceived"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MountShow__UdpManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    3,   47,    2, 0x06 /* Public */,
       8,    2,   54,    2, 0x06 /* Public */,
      11,    0,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   60,    2, 0x08 /* Private */,
      13,    3,   61,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::QByteArray, QMetaType::QString, QMetaType::UShort,    5,    6,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    9,   10,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray, QMetaType::QString, QMetaType::UShort,    5,    6,    7,

       0        // eod
};

void MountShow::UdpManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UdpManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->packetReceived((*reinterpret_cast< const QByteArray(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        case 2: _t->logMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->heartbeatTimeout(); break;
        case 4: _t->onHeartbeat(); break;
        case 5: _t->onPacketReceived((*reinterpret_cast< const QByteArray(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UdpManager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpManager::connectedChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UdpManager::*)(const QByteArray & , const QString & , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpManager::packetReceived)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UdpManager::*)(const QString & , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpManager::logMessage)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (UdpManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpManager::heartbeatTimeout)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MountShow::UdpManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_MountShow__UdpManager.data,
    qt_meta_data_MountShow__UdpManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MountShow::UdpManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MountShow::UdpManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MountShow__UdpManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MountShow::UdpManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void MountShow::UdpManager::connectedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MountShow::UdpManager::packetReceived(const QByteArray & _t1, const QString & _t2, quint16 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MountShow::UdpManager::logMessage(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MountShow::UdpManager::heartbeatTimeout()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
