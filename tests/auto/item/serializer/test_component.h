#ifndef TEST_COMPONENT_H
#define TEST_COMPONENT_H

#include "appcore.h"
#include <qobject.h>

class TestComponent
{
    Q_GADGET

    Q_CLASSINFO("dependsOn", "PluginManager")
    Q_CLASSINFO("dependsOn", "GuiManager")

public:
    static void init();
};

#endif
