#include "test_component.h"
#include "helper/startup_helper.h"
#include "plugin/plugin_manager.h"
#include "project/project_manager_gui.h"
#include "some_transporter.h"
#include "some_item.h"

STARTUP_ADD_COMPONENT(TestComponent)

void TestComponent::init()
{
    PluginManager::instance()->addPluginComponent<SomeItem, AbstractItem>();
    AbstractItem::registerConnectorStyle(Qt::blue, qMetaTypeId<SomeTransporter*>());

    GuiManager::instance()->setMode(GuiMode::Headless);
}
