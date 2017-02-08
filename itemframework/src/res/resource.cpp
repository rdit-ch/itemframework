#include "resource.h"
#include <QPen>

bool Resource::b_init = false;
QMap<QString, QVariant> Resource::map_res;

void Resource::init()
{
    if (b_init) {
        return;
    }

    b_init = true;

    map_res.insert("item_connector_selpoint_color", QColor(Qt::black));
    map_res.insert("item_connector_inactive", QBrush(Qt::white, Qt::BDiagPattern));

    map_res.insert("item_box_bgcolor", QColor(0xF5, 0xF5, 0xF5));
    map_res.insert("item_box_bordercolor", QColor(Qt::gray));
    map_res.insert("item_box_progresscolor", QColor(Qt::green));
    map_res.insert("item_box_typecolor", QColor(Qt::black));
    map_res.insert("item_box_namecolor", QColor(Qt::darkGray));
    map_res.insert("item_box_selected_pen", QPen(Qt::black, 1, Qt::DashLine));

    map_res.insert("item_selection_pen", QPen(Qt::black, 1, Qt::DotLine));

    map_res.insert("item_note_bgcolor", QColor(249, 238, 122));

    map_res.insert("project_active_bgcolor", QColor(Qt::white));
    map_res.insert("project_inactive_bgcolor", QColor(Qt::lightGray));
    map_res.insert("project_active_fgcolor", QColor(Qt::black));
    map_res.insert("project_inactive_fgcolor", QColor(Qt::darkGray));
}
