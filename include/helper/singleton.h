#ifndef SINGLETON_H
#define SINGLETON_H

#include "appcore.h"
#include <QtGlobal>
#include <QDebug>

/**
 * @brief The AbstractSingleton class defines functions that every singleton must implement
 * Every Singleton must provide a constructor, destructor a postInit() and a preDestroy() method
 */
class CORE_EXPORT AbstractSingleton
{
    Q_GADGET //Needed in order to use qobject_cast / interfaces stuff
public:
    AbstractSingleton() {} //In order to stay constructible (needed because of Q_DISABLE_COPY macro)
    virtual ~AbstractSingleton() {}
protected:
    friend class ObjectState; //Grant ObjectState access to the following members:
    /**
     * @brief postInit will be called after all components and singletons have been constructed/initialized
     * @return true if post initialization was successfull
     */
    virtual bool postInit() = 0;
    /**
     * @brief preDestroy will be called before any of the components and singletons will be destructed/deinitailized
     * @return true if pre destruction was successfull
     */
    virtual bool preDestroy() = 0;

    Q_DISABLE_COPY(AbstractSingleton)
};
Q_DECLARE_INTERFACE(AbstractSingleton, "Itemframework.Interface.Singleton/1.0")

/**
 * @brief The SingletonStorage class is a helper to provide a unique, once-per-application, shared-across-all-plugins, hashmap to save Singletons into.
 */
class CORE_EXPORT SingletonStorage
{
    //Why do we need this class, and can't we just add a static data member of Type T* to the Singleton<T> class?
    // Well, the static member would need to be defined and initialized in a separate cpp file. This is not possible for template classes.
    // But you could return a reference to a local static variable from a static function and use that as storage!
    //  Yes, this works on Linux, but not on Windows :( (Reason unknown).
protected:
    static AbstractSingleton* getInstance(const char* name);
    static void storeInstance(const char* name, AbstractSingleton* inst);
};

/**
 * @brief The Singleton Template figures as baseclass for all Singletons.
 * It provides a instance() functions which returns the only instance.
 * Template parameter T: Class that you implement (Curiously recurring template pattern)
 */
template<typename T> class CORE_EXPORT Singleton : public AbstractSingleton, private SingletonStorage
{
public:
    Singleton()   //Should only be called from ObjectState Class
    {
        storeInstance(T::staticMetaObject.className(), this); //Store this instance in the Hashmap
    }

    ~Singleton()  //Should only be called from ObjectState Class
    {
        storeInstance(T::staticMetaObject.className(), nullptr); //Remove this instance from the Hashmap
    }

    /**
     * @brief Returns the current instance of the singleton
     * @return the one and only instance
     */
    static T* instance()
    {
        return static_cast<T*>(getInstance(T::staticMetaObject.className())); //Retrieve Instance from Hashmap by classname
    }
};



#endif // SINGLETON_H
