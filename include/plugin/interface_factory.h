
#ifndef INTERFACE_FACTORY_H
#define INTERFACE_FACTORY_H
#include <QObject>
#include <QMetaObject>
#include "appcore.h"
#include "plugin/plugin_manager.h"

/**
 * @brief The plugin factory interface. You need to have one factory per plugin container.
 */
class ITEMFRAMEWORK_EXPORT InterfaceFactory : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief Add your classes to the plugin manager in this method. \sa Plugin_Manager::add_plugin()
     * @return
     */
    virtual bool init() = 0;
};
Q_DECLARE_INTERFACE(InterfaceFactory, "Itemframework.Interface.Factory/1.0")
#endif // INTERFACE_FACTORY_H
//eof
