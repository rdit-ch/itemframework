#ifndef ABSTRACT_WINDOW_ITEM_P_H
#define ABSTRACT_WINDOW_ITEM_P_H

#include "item/abstract_window_item.h"

#include <QObject>

class AbstractWindowItem;

class AbstractWindowItemPrivate
{
public:
    explicit AbstractWindowItemPrivate(AbstractWindowItem* parent);

    AbstractWindowItem* const q_ptr;
    Q_DECLARE_PUBLIC(AbstractWindowItem)

    void setupWindow();

    QMainWindow* _window = nullptr;
    mutable bool _openWindowContextActionEnabled = true;
    QList<QAction*> _contextMenuEntries;
};

#endif // ABSTRACT_WINDOW_ITEM_P_H
