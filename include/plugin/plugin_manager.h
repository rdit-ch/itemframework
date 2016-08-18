#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include "appcore.h"
#include "helper/singleton.h"
#include <QObject>
#include <QMetaObject>

class PluginManagerPrivate;
class PluginMetaData;

/**
 * @brief The PluginManager class
 */
class CORE_EXPORT PluginManager : public QObject, public Singleton<PluginManager>
{

    //*****************************************************************************
    // Singleton
    //*****************************************************************************
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)
public:
    Q_INVOKABLE PluginManager();
    ~PluginManager();
protected:
    bool postInit();
    bool preDestroy();
    //*****************************************************************************

    //*****************************************************************************
    // PluginManager public interface
    //*****************************************************************************
public:
    /**
     * @brief pluginPath
     * @return a QStringList containing all plugin directories
     */
    QStringList const& pluginPath();

    /**
     * @brief setPluginPath adds a new directory path to the plugin path
     * @param directory, the path to be added
     */
    void setPluginPath(const QStringList &directory);

    /**
     * @brief lastError returns the last error message of the plugin manager
     * @return a QString error message
     */
    QString const& lastError();

    /**
     * @brief pluginMetaDataList returns the list of plugins found in the specified directories
     * @return a QList of PluginMetaData*
     */
    QList<PluginMetaData*> pluginMetaDataList();

    /**
     * @brief addPluginComponent adds a plugin component to internal list from where it can be instantiated later
     * @param class Derived the class of the component
     * @param class Base the class from wich the component inherits
     */
    template<class Derived, class Base> void addPluginComponent()
    {
        appendPluginComponentHelper(Derived::staticMetaObject, Base::staticMetaObject);
    }

    /**
     * @brief createInstances creates all instances of a plugin and returns them in a vector
     * @param T the plugin type
     * @return QVector<T*> a vector with plugin component instances
     */
    template<class T> QVector<T*> createInstances()
    {
        QVector<T*> outVec;
        QVectorIterator<QObject*> it(createInstancesHelper(T::staticMetaObject));

        while (it.hasNext()) {
            T* obj = (T*)qobject_cast<T*>(it.next());

            if (obj == nullptr) {
                qWarning() << "Create Instances, cast failed";
            } else {
                outVec.append(obj);
            }
        }

        return outVec;
    }

    /**
     * @brief createInstance creates a single instance of a class from a given className
     * @param className the name of the class to be instantiated
     * @return T* the instance of a class or a nullptr
     */
    template<class T> T* createInstance(QString const& className)
    {
        QObject* obj = createInstanceHelper(className);
        T* ret = (T*)qobject_cast<T*>(obj);

        if (ret == nullptr) {
            qWarning() << "Create instance, cast failed for type: " << className;
        }

        return ret;
    }
    //*****************************************************************************

public slots:
    /**
     * @brief serializePluginState serializes the submitted meta data
     * @param metaData the plugin meta data
     */
    void serializePluginMetaData(PluginMetaData* metaData);

private:

    QVector<QObject*> createInstancesHelper(QMetaObject interfaceObject);
    void appendPluginComponentHelper(QMetaObject derivedMeta, QMetaObject baseMeta);
    QObject* createInstanceHelper(QString className);

    PluginManagerPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(PluginManager)

};

#endif // PLUGINMANAGER_H
