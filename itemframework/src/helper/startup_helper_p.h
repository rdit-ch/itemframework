#ifndef STARTUP_HELPER_P_H
#define STARTUP_HELPER_P_H

#include "helper/startup_helper.h"
#include <QSharedPointer>
#include <QMutex>

/**
 * @brief The ObjectState class represents the State of a Singleton or Component and helps calling create/delete/init/deinit them
 */
class ObjectState
{
public:
    /**
     * @brief Constructs a ObjectState for a Singleton
     * @param metaobj The static metaobject of the singleton
     */
    ObjectState(const QMetaObject& metaobj);
    /**
     * @brief Constructs a ObjectState for a Component
     * @param metaobj The static metaobject of the component
     * @param initFunc A Function Pointer to the static void init() function
     * @param deinitFunc A Function Pointer to the static void deinit() function
     */
    ObjectState(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc);

    /**
     * @brief Returns the name of the class represented by the ObjectState
     * @return Classname as QString
     */
    QString className() const;
    /**
     * @brief Returns a list of all dependencies of the represented class
     * @return Names of the classes on which the represented class depends as StringList
     */
    QStringList dependencies() const;

    /**
     * @brief Returns a list of all optional dependencies of the represented class
     * @return Names of the classes on which the represented class optionally depends in form of a StringList
     */
    QStringList optionalDependencies() const;

    /**
     * @brief Whether or not the represented class has already been initialized
     * @return true if the class has been initialized
     */
    bool initialized() const;


    /**
     * @brief Whether or not the represented class is a gui module (= it requires a Q(Gui)Application to run)
     * @return true if the class is a gui module
     */
    bool guiModule() const;

    /**
     * @brief Tries to construct the represented singleton or call init() on the represented component
     * @return false on error
     */
    bool init();
    /**
     * @brief Tries to call postInit() on the represented singleton. Does nothing if the represented class is a component
     * @return false on error
     */
    bool postInit();
    /**
     * @brief Tries to call preDestroy() on the represented singleton. Does nothing if the represented class is a component
     * @return false on error
     */
    bool preDestroy();
    /**
     * @brief Tries to delete the represented singleton or call deinit() on the represented component
     * @return false on error
     */
    bool destroy();

private:
    const QMetaObject _metaObject;
    bool _initialized;
    bool _guiModule;
    QStringList _dependencies;
    QStringList _optionalDependencies;
    const bool _isSingleton;
    AbstractSingleton* _instance;
    internal::VoidFnPtr _initFunc;
    internal::VoidFnPtr _deinitFunc;
};


class StartupHelperPrivate
{
public:

    /**
     * @brief Creates all Singletons and calls init() on Components in the correct order
     * The function ensures that all dependencies marked with "dependsOn" are respected.
     * @return true if creation of all singletons, calling init() on all components and postInit() on all singletons succeeded.
     */
    static bool start();

    /**
     * @brief Deletes all Singleton Instances and calls deinit() on all Components
     * The Singleton Instances and Components are called in reversed order compared to the startup.
     * @return true if calling preDestroy() on all singletons, deinit() on all components and deleting all singletons succeeded
     */
    static bool stop();

    /**
     * @brief Registers the start and stop function at QApplication so that they will be called on appstart/appterm
     * It's safe to call this function multiple times.
     */
    static void registerHandlers();

    /**
     * @brief getClassesRecursive gets all classes and their dependencies starting from the passed ObjectState.
     * @param backtrace the list of classes which depend on the current class
     * @param globalDependencies the ordered list of classes which should be extended
     * @param object the class to start with
     * @return true if dependencies are fine, false if we have missing or circular dependencies
     */
    static bool getClassesRecursive(QStringList backtrace, QStringList& globalDependencies, const ObjectState& object);

    /**
     * @brief addComponentHelper adds a component to the list of "tracked" modules
     * @param metaobj the metaobject of the component to add
     * @param initFunc a function pointer to the static init() function of the class or a nullptr
     * @param deinitFunc a function pointer to the static deinit() function of the class or a nullptr
     */
    static void addComponentHelper(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc);

    /**
     * @brief addSingletonHelper adds a singleton to the list of "tracked" modules
     * @param metaobj the metaobject of the singleton to add
     */
    static void addSingletonHelper(const QMetaObject& metaobj);

    /**
     * @brief findClass finds a class in the list of registerd classes by name
     * @param classname name of the class to find
     * @return pointer to ObjectState or nullptr if not found
     */
    static ObjectState* findClass(QString classname);
private:

    static QVector<QSharedPointer<ObjectState>> _classes;
    static bool _initalized;
    static QStringList _orderedClasses;
    static QMutex _registredMutex;
    static bool _registred;
};

#endif // STARTUP_HELPER_P_H

