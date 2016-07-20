#include <QDockWidget>
#include "item_manager.h"
#include "item_scene.h"
#include "item_toolbox.h"
#include "gui/gui_manager.h"
#include "item/abstract_item.h"
#include "plugin/plugin_manager.h"
#include "item/item_templates_widget.h"

#include "helper/startup_helper.h"

STARTUP_ADD_SINGLETON(Item_Manager)

Item_Manager::Item_Manager()
    : _itemTemplatesWidget{new ItemTemplatesWidget{}}
{
    registerItemToolbox();
    registerItemTemplatesWidget();
}

Item_Manager::~Item_Manager()
{}

void Item_Manager::registerItemToolbox()
{
    auto toolbox = new Item_Toolbox();
    auto items = PluginManager::instance()->createInstances<AbstractItem>();

    for (auto item : items) {
        toolbox->addItem(item);
        delete item;
    }

    Gui_Manager::instance()->include_In_Layout(toolbox, Window_Layout::Left_Area);
}

void Item_Manager::registerItemTemplatesWidget()
{
    Gui_Manager::instance()->include_In_Layout(_itemTemplatesWidget.data(), Window_Layout::Left_Area);
}

QSharedPointer<ItemTemplatesWidget> Item_Manager::itemTemplatesWidget()
{
    return _itemTemplatesWidget;
}

bool Item_Manager::preDestroy()
{
    return true;
}

bool Item_Manager::postInit()
{
    return true;
}
