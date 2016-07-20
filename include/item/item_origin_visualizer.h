#ifndef GRAPHICS_ITEM_ORIGIN_VISUALIZER_H
#define GRAPHICS_ITEM_ORIGIN_VISUALIZER_H

#include "appcore.h"

#include <QWidget>

class AbstractItem;

namespace Ui
{
class GraphicsItemOriginVisualizer;
}

/**
 * @brief The ItemOriginVisualizer class is a widget that displays the origin of
 * the input data of an item with one input in the form of a list of intermediary
 * items.
 *
 * The origin is displayed as a list of items the data passes through before it
 * is made available to the item via its input.
 *
 * An ItemOriginVisualizer is commonly used as a component of custom items widget.
 */
class CORE_EXPORT ItemOriginVisualizer : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an empty ItemOriginVisualizer with no item
     * @param parent The widget this visualizer is embedded in, if any
     */
    explicit ItemOriginVisualizer(QWidget* parent = 0);

    /**
     * @brief Constructs an ItemOriginVisualizer with an item
     * @param item The item whose datas origin this visualizer displays
     * @param parent The widget this visualizer is embedded in, if any
     */
    explicit ItemOriginVisualizer(AbstractItem* item, QWidget* parent = 0);

    ~ItemOriginVisualizer();

    /**
     * @param item  The new item whose datas origin this visualizer should display
     */
    void setItem(AbstractItem* item);

private:
    QScopedPointer<class ItemOriginVisualizerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ItemOriginVisualizer)
};

#endif // GRAPHICS_ITEM_ORIGIN_VISUALIZER_H
