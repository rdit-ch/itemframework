#include "gui/livedoc.h"
#include "livedoc_widget.h"
#include "gui/interface_livedoc_provider.h"
#include "livedoc_home_provider.h"
#include "gui/gui_manager.h"
#include "plugin/plugin_manager.h"
#include "helper/startup_helper.h"
#include <QAction>

STARTUP_ADD_SINGLETON(Livedoc)

Livedoc::Livedoc()
{
    browser = new Livedoc_Widget(GuiManager::instance()->widgetReference());
    connect(GuiManager::instance()->action("LiveDoc"), SIGNAL(triggered(bool)), this, SLOT(toggle(bool)));
    connect(browser, SIGNAL(signal_close()), this, SLOT(widget_closed()));

    add_provider(new Livedoc_Home_Provider());

    QVector<Interface_Livedoc_Provider*> plugin_providers = PluginManager::instance()->createInstances<Interface_Livedoc_Provider>();

    QVectorIterator<Interface_Livedoc_Provider*> it(plugin_providers);

    while (it.hasNext()) {
        Interface_Livedoc_Provider* provider = it.next();
        add_provider(provider);
    }
}

Livedoc::~Livedoc()
{
}

bool Livedoc::postInit()
{
    return true;
}

bool Livedoc::preDestroy()
{
    return true;
}

void Livedoc::widget_closed()
{
    GuiManager::instance()->action("LiveDoc")->setChecked(false);
}

const QList<Interface_Livedoc_Provider*>& Livedoc::get_providers() const
{
    return lis_providers;
}

void Livedoc::add_provider(Interface_Livedoc_Provider* p)
{
    if (p != NULL && !lis_providers.contains(p)) {
        lis_providers.append(p);
    }
}

void Livedoc::remove_provider(Interface_Livedoc_Provider* p)
{
    if (lis_providers.contains(p)) {
        lis_providers.removeOne(p);
    }
}

void Livedoc::navigate(QUrl doc)
{
    browser->navigate(doc);
}

void Livedoc::show()
{
    browser->show();
}

void Livedoc::hide()
{
    browser->hide();
}

void Livedoc::toggle(bool visible)
{
    if (visible) {
        show();
    } else {
        hide();
    }

}


QString Livedoc::get_doc(QUrl doc)
{
    foreach (Interface_Livedoc_Provider* prov, lis_providers) {
        QString html = prov->provide_livedoc(doc);

        if (!html.isEmpty()) {
            return html;
        }
    }

    return QString("<html><body><h2>404 - Not found</h2>No such page: " + doc.toString() + "</body></html>");
}

