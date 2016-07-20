#include "startup_helper_p.h"
#include "helper/singleton.h"
#include <QMetaType>
#include <QMetaClassInfo>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QTimer>

//Source: http://stackoverflow.com/a/1340291/2606757
template <typename T>
QList<T> reversed(const QList<T>& in)
{
    QList<T> result;
    result.reserve(in.size());   // reserve is new in Qt 4.7
    std::reverse_copy(in.begin(), in.end(), std::back_inserter(result));
    return result;
}


QVector<QSharedPointer<ObjectState>> StartupHelperPrivate::_classes;
QStringList StartupHelperPrivate::_orderedClasses;
bool StartupHelperPrivate::_initalized = false;
QMutex StartupHelperPrivate::_registredMutex;
bool StartupHelperPrivate::_registred = false;


void StartupHelperPrivate::addComponentHelper(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc)
{
    registerHandlers();
    _classes.append(QSharedPointer<ObjectState>(new ObjectState(metaobj, initFunc, deinitFunc)));
}

void StartupHelperPrivate::addSingletonHelper(const QMetaObject& metaobj)
{
    registerHandlers();
    _classes.append(QSharedPointer<ObjectState>(new ObjectState(metaobj)));
}

void StartupHelper::addComponentHelper(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc)
{
    StartupHelperPrivate::addComponentHelper(metaobj, initFunc, deinitFunc);
}

void StartupHelper::ensureCoreGetsLinked()
{
    //This function is intended to be emtpy
}

void StartupHelper::addSingletonHelper(const QMetaObject& metaobj)
{
    StartupHelperPrivate::addSingletonHelper(metaobj);
}

static void start_handler()
{
    if (StartupHelperPrivate::start()) {
        qDebug() << "Startup finished. Application Version:" << QApplication::applicationVersion();
    } else {
        qWarning() << "Problems on startup of Application Version" << QApplication::applicationVersion();
    }
}

static void stop_handler()
{
    if (!StartupHelperPrivate::stop()) {
        qWarning() << "Problems while stopping the application. See above.";
    }
}

void StartupHelperPrivate::registerHandlers()
{
    QMutexLocker locker(&_registredMutex); //lock the entire function with mutex

    if (_registred) {
        return;
    }

    QTimer::singleShot(0, start_handler); //Call start_handler as soon as we enter QCoreApplication::exec()
    qAddPostRoutine(stop_handler); //Call stop_handler when QCoreApplication exits
    _registred = true;
    Q_UNUSED(locker);
}

bool StartupHelperPrivate::start()
{
    if (_classes.isEmpty()) {
        return true;
    }

    if (qobject_cast<QGuiApplication*>(QCoreApplication::instance()) == nullptr) { //remove all gui modules when we dont hava a Q(Gui)Application
        for (int i = _classes.count() - 1; i >= 0; i--) {
            if (_classes.at(i)->guiModule()) {
                _classes.removeAt(i);
            }
        }
    }

    for (const QSharedPointer<ObjectState>& obj : _classes) {
        if (!getClassesRecursive(QStringList(), _orderedClasses, *obj.data())) {
            return false; //Circular or Missing Dependency
        }
    }

    //qDebug()<< "Order of instantiation" << _orderedClasses;

    for (const QString& classname : _orderedClasses) {
        ObjectState* obj = findClass(classname);

        if (!obj->init()) {
            qWarning() << "Couldn't startup because init()/constructor failed on " << classname;
            return false;
        }
    }

    for (const QString& classname : _orderedClasses) {
        ObjectState* obj = findClass(classname);

        if (!obj->postInit()) {
            qWarning() << "Couldn't startup because postinit() failed on " << classname;
            return false;
        }
    }

    _initalized = true;
    return true;
}

bool StartupHelperPrivate::stop()
{
    if (!_initalized) {
        return false;
    }

    bool allOk = true;
    const QStringList reverseOrderedClasses = reversed(_orderedClasses);

    for (const QString& classname : reverseOrderedClasses) {
        ObjectState* obj = findClass(classname);

        if (!obj->preDestroy()) {
            qWarning() << "preDestroy() failed on " << classname;
            allOk = false;
        }
    }

    for (const QString& classname : reverseOrderedClasses) {
        ObjectState* obj = findClass(classname);

        if (!obj->destroy()) {
            qWarning() << "destroy()/delete failed on " << classname;
            allOk = false;
        }
    }

    _initalized = false;
    return allOk;
}


bool StartupHelperPrivate::getClassesRecursive(QStringList backtrace, QStringList& globalDependencies, const ObjectState& object)
{
    /* This method mainly helps detecting circular or missing dependencies.
     * How do we detect circular dependencies?
     *  Imagine the following Dependency Tree:
     *      A
     *          D
     *          B
     *      B
     *          C
     *      C
     *          D
     *          A
     *  In this example we have a Dependency cycle A->B->C->A
     *  We maintain a global dependency list which should contain all classnames in the right order in the end.
     *  The second parameter contains a list of the "parent items". If we enter this function and the passed classname (via object)
     *   is already in the backtrace list, he have a circular dependeny.
     */

    if (backtrace.contains(object.className())) { //this function has already been called with the same object
        qWarning() << "Couldn't startup because" << object.className()
                   << "is involved in a circular dependency";
        return false;
    }

    backtrace.append(object.className());

    //Walk through dependencies
    for (const QString& depClassName : object.dependencies()) {
        ObjectState* depClass = findClass(depClassName); //search for dependency by name

        if (depClass == nullptr) {
            qWarning() << "Couldn't startup because" << object.className()
                       << "has a missing dependency:" << depClassName;
            return false;
        }

        if (!getClassesRecursive(backtrace, globalDependencies, *depClass)) { //dependency has itself a missing/circular dependency
            return false;
        }
    }

    //Walk throught optional dependencies
    for (const QString& optDepClassName : object.optionalDependencies()) {
        ObjectState* optDepClass = findClass(optDepClassName); //search for dependency by name

        if (optDepClass == nullptr) {
            continue;
        }

        if (!getClassesRecursive(backtrace, globalDependencies, *optDepClass)) { //dependency has itself a missing/circular dependency
            return false;
        }
    }

    if (!globalDependencies.contains(object.className())) { //make sure this class has not yet been added by another dependency branch
        globalDependencies.append(object.className());
    }

    return true;
}

ObjectState* StartupHelperPrivate::findClass(QString classname)
{
    for (const QSharedPointer<ObjectState>& obj : _classes) {
        if (obj->className() == classname) {
            return obj.data();
        }
    }

    return nullptr;
}

ObjectState::ObjectState(const QMetaObject& metaobj) : _metaObject(metaobj), _isSingleton(true)
{
    _initialized = false;
    _instance = nullptr;
    _initFunc = nullptr;
    _deinitFunc = nullptr;
    _guiModule = false;

    int guiModuleIndex = metaobj.indexOfClassInfo("guiModule");

    if (guiModuleIndex != -1) {
        _guiModule = (metaobj.classInfo(guiModuleIndex).value() == QString("true"));
    }

    for (int i = 0; i < metaobj.classInfoCount(); i++) {
        const QMetaClassInfo classinfo = metaobj.classInfo(i);

        if (strcmp(classinfo.name(), "dependsOn") == 0) {
            _dependencies << classinfo.value();
        }

        if (strcmp(classinfo.name(), "optionallyDependsOn") == 0) {
            _optionalDependencies << classinfo.value();
        }
    }
}

ObjectState::ObjectState(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc)
    : _metaObject(metaobj), _isSingleton(false), _initFunc(initFunc), _deinitFunc(deinitFunc)
{
    _initialized = false;
    _guiModule = false;
    _instance = nullptr;

    for (int i = 0; i < metaobj.classInfoCount(); i++) {
        const QMetaClassInfo classinfo = metaobj.classInfo(i);

        if (strcmp(classinfo.name(), "dependsOn") == 0) {
            _dependencies << classinfo.value();
        }

        if (strcmp(classinfo.name(), "optionallyDependsOn") == 0) {
            _optionalDependencies << classinfo.value();
        }
    }
}

QString ObjectState::className() const
{
    return _metaObject.className();
}

QStringList ObjectState::dependencies() const
{
    return _dependencies;
}

QStringList ObjectState::optionalDependencies() const
{
    return _optionalDependencies;
}

bool ObjectState::initialized() const
{
    return _initialized;
}

bool ObjectState::guiModule() const
{
    return _guiModule;
}

bool ObjectState::init()
{
    if (_initialized) {
        return false;
    }

    if (_isSingleton) {
        //Creating the new (and first) instance
        QObject* obj = _metaObject.newInstance();

        if (obj == nullptr) {
            qWarning() << "Couldn't create (first) instance of singleton" << className() << ". Forgot Q_INVOKABLE macro on constructor?";
            return false;
        }

        _instance = qobject_cast<AbstractSingleton*>(obj);

        if (_instance == nullptr) {
            qWarning() << "Couldn't cast instance of singleton" << className() << ". Forgot to public inherit from AbstractSingleton?";
            return false;
        }

        //qDebug() << "Constructed" << className();
    } else {
        if (_initFunc != nullptr) {
            _initFunc();
            //qDebug() << "Called init() on " << className();
        }
    }

    _initialized = true;
    return true;
}

bool ObjectState::postInit()
{
    if (!_initialized) {
        return false;
    }

    if (_isSingleton) {
        return _instance->postInit();
    } else {
        return true;
    }
}

bool ObjectState::preDestroy()
{
    if (!_initialized) {
        return false;
    }

    if (_isSingleton) {
        return _instance->preDestroy();
    } else {
        return true;
    }
}

bool ObjectState::destroy()
{
    if (!_initialized) {
        return false;
    }

    if (_isSingleton) {
        delete _instance;
        _instance = nullptr;
        _initialized = false;
        //qDebug() << "Deleted " << className();
    } else {
        if (_deinitFunc != nullptr) {
            _deinitFunc();
            //qDebug() << "Called deinit() on " << className();
            _initialized = false;
        }
    }

    return true;
}



