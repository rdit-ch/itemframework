#ifndef STARTUPHELPER_H
#define STARTUPHELPER_H

#include <QMetaObject>
#include <QApplication>

#include "appcore.h"
#include "helper/singleton.h"
#include "helper/startup_helper_templates.h"

class StartupHelperPrivate;

/*! \file */ //Otherwise we won't see the marcos documented.

/**
 * @brief The StartupHelper class manages dependencies between global singletons and other components.
 * It ensures that the singletons/components are constructed/deleted in the right order.
 */
class CORE_EXPORT StartupHelper
{
public:
    /**
     * @brief Adds a Singleton to the Helper. The Singleton will be constructed after it's dependencies. \n
     * Dependencies can be marked by adding a Q_CLASSINFO "dependsOn" or "optionallyDependsOn" to the class definition. \n
     * @code
     *     Q_CLASSINFO("dependsOn","Livedoc")
     *     Q_CLASSINFO("optionallyDependsOn","TType")
     * @endcode
     * Singleton must use the Q_GADGET or Q_OBJECT macro and derive from AbstractSingleton. \n
     * You must either call this function before running QCoreApplication::exec() or use STARTUP_ADD_SINGLETON().
     */
    template<class T> static void addSingleton()
    {
        static_assert(std::is_base_of<Singleton<T>, T>::value, "Class is not derived from AbstractSingleton");
        static_assert(internal::HasMetaObject<T>::value, "Class has no Q_OBJECT or Q_GADGET macro");
        addSingletonHelper(T::staticMetaObject);
    }

    /** @brief Adds a component to the Helper. The StartupHelper will call init() and deinit() on the component class. \n
     *  Every class with at least a static void init() or void deinit() function can be specified. \n
     *  Dependencies can be marked by adding a Q_CLASSINFO "dependsOn" or "optionallyDependsOn" to the class definition. (see addSingleton() ). \n
     *  The components must use the Q_GADET or Q_OBJECT macro. \n
     *  You must either call this function before running QCoreApplication::exec() or use STARTUP_ADD_COMPONENT().
     */
    template<class T> static void addComponent()
    {
        static_assert(internal::HasMetaObject<T>::value, "Class has no Q_OBJECT or Q_GADGET macro");
        static_assert((internal::GetInitFnPtr<T>::value != nullptr || internal::GetDeinitFnPtr<T>::value != nullptr),
                      "Class has neither an init() nor a deinit() function");
        addComponentHelper(T::staticMetaObject, internal::GetInitFnPtr<T>::value, internal::GetDeinitFnPtr<T>::value);
    }

    /**
     * @brief This function does nothing.
     * But you can call it if you want to make sure that this translation unit is not optimized out by the linker.
     */
    static void ensureCoreGetsLinked();

private:
    static void addSingletonHelper(const QMetaObject& metaobj);
    static void addComponentHelper(const QMetaObject& metaobj, internal::VoidFnPtr initFunc, internal::VoidFnPtr deinitFunc);
};



/**
 * @brief This macro will automatically call StartupHelper::addSingleton() for the passed Type before the app starts.
 * \sa StartupHelper::addSingleton()
 */
#define STARTUP_ADD_SINGLETON(type) \
static void startup_singleton_##type() { \
   StartupHelper::addSingleton<type>(); \
} \
Q_COREAPP_STARTUP_FUNCTION(startup_singleton_##type)

/**
* @brief This macro will automatically call StartupHelper::addComponent() for the passed Type before the app starts.
* \sa StartupHelper::addComponent()
 */
#define STARTUP_ADD_COMPONENT(type) \
static void startup_component_##type() { \
   StartupHelper::addComponent<type>(); \
} \
Q_COREAPP_STARTUP_FUNCTION(startup_component_##type)



#endif // STARTUP_HELPER_H
