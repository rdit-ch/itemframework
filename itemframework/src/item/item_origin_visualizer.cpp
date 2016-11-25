#include "item/item_origin_visualizer.h"
#include "item_origin_visualizer_p.h"
#include "ui_graphics_item_origin_visualizer.h"
#include "item_origin_visualizer_entry.h"
#include "item/abstract_item.h"
#include "item/item_input.h"
#include "item/item_output.h"

ItemOriginVisualizer::ItemOriginVisualizer(QWidget* parent) :
    QWidget(parent), d_ptr(new ItemOriginVisualizerPrivate(this, nullptr))
{
}

ItemOriginVisualizer::ItemOriginVisualizer(AbstractItem* item, QWidget* parent) :
    QWidget(parent), d_ptr(new ItemOriginVisualizerPrivate(this, item))
{
}

ItemOriginVisualizerPrivate::ItemOriginVisualizerPrivate(ItemOriginVisualizer* parent,
        AbstractItem* item) :
    _parent(parent), _item(item), _ui(new Ui::GraphicsItemOriginVisualizer)
{
    _ui->setupUi(_parent);

    if (_item != nullptr) {
        updateContent();
    }
}

ItemOriginVisualizerPrivate::~ItemOriginVisualizerPrivate()
{
    delete _ui;
}

void ItemOriginVisualizer::setItem(AbstractItem* item)
{
    Q_D(ItemOriginVisualizer);

    d->_item = item;

    d->updateContent();
}

void ItemOriginVisualizerPrivate::updateContent()
{
    for (int i = _entries.count() - 1; i >= 0; i--) {
        QWidget* wi = _entries.at(i);
        _ui->verticalLayout->removeWidget(wi);
        _entries.removeAt(i);
        delete wi;
    }

    AbstractItem* item = _item;

    while (true) {
        Item_Origin_Visualizer_Entry* entry = new Item_Origin_Visualizer_Entry(item, _parent);
        _ui->verticalLayout->insertWidget(0, entry);
        _entries.append(entry);

        if (item->inputs().count() != 1) {
            break;
        }

        ItemInput* inp = item->inputs().at(0);
        // TODO proper disconnect again!
        connect(inp, SIGNAL(outputConnected()), this, SLOT(updateContent()));
        connect(inp, SIGNAL(outputDisconnected()), this, SLOT(updateContent()));

        if (!inp->isConnected()) {
            break;
        }

        item = inp->output()->owner();
    }
}

ItemOriginVisualizer::~ItemOriginVisualizer()
{
}
