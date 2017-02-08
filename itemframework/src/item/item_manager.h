#ifndef GRAPHICS_ITEM_MANAGER_H
#define GRAPHICS_ITEM_MANAGER_H

#include <QObject>
#include <QMutex>
#include "helper/singleton.h"
#include "item/item_templates_widget.h"

class Item_Manager: public QObject, public Singleton<Item_Manager>
{
    //------------------Singleton Stuff---------------------------
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)


    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "GuiManager")
    Q_CLASSINFO("dependsOn", "PluginManager")
    Q_CLASSINFO("dependsOn", "SettingsScope")

public:
    Q_INVOKABLE Item_Manager();
    ~Item_Manager();

protected:
    bool postInit();
    bool preDestroy();
    //------------------End Singleton Stuff---------------------------

public:
    QSharedPointer<ItemTemplatesWidget> itemTemplatesWidget();

private:
    void registerItemToolbox();
    void registerItemTemplatesWidget();

    QSharedPointer<ItemTemplatesWidget> _itemTemplatesWidget;
};
#endif // GRAPHICS_ITEM_MANAGER_H
