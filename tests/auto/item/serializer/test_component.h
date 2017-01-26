#ifndef TEST_COMPONENT_H
#define TEST_COMPONENT_H

#include "appcore.h"
#include <qobject.h>

class ITEMFRAMEWORK_TEST_EXPORT TestComponent
{
    Q_GADGET

    Q_CLASSINFO("dependsOn", "PluginManager")
    Q_CLASSINFO("dependsOn", "GuiManager")

public:
    static void init();
};

#endif
