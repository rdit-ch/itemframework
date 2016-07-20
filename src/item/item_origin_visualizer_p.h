#ifndef ITEM_ORIGIN_VISUALIZER_P_H
#define ITEM_ORIGIN_VISUALIZER_P_H

#include "item/item_origin_visualizer.h"
#include "item_origin_visualizer_entry.h"

class ItemOriginVisualizerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ItemOriginVisualizerPrivate(ItemOriginVisualizer* parent, AbstractItem* item);
    ~ItemOriginVisualizerPrivate();

    ItemOriginVisualizer* _parent;
    AbstractItem* _item;
    QList<Item_Origin_Visualizer_Entry*> _entries;
    Ui::GraphicsItemOriginVisualizer* _ui;

public slots:
    void updateContent();
};

#endif // ITEM_ORIGIN_VISUALIZER_P_H

