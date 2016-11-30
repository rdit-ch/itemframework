#ifndef GRAPHICS_ITEM_ORIGIN_VISUALIZER_ENTRY_H
#define GRAPHICS_ITEM_ORIGIN_VISUALIZER_ENTRY_H

#include <QFrame>

class AbstractItem;

namespace Ui
{
class Graphics_Item_Origin_Visualizer_Entry;
}

class Item_Origin_Visualizer_Entry : public QFrame
{
    Q_OBJECT

public:
    explicit Item_Origin_Visualizer_Entry(AbstractItem* item, QWidget* parent = 0);
    ~Item_Origin_Visualizer_Entry();

private slots:
    void update_content();
    void mouseDoubleClickEvent(class QMouseEvent* event);
    void contextMenuEvent(class QContextMenuEvent* event);

private:
    AbstractItem* item;
    Ui::Graphics_Item_Origin_Visualizer_Entry* ui;
};

#endif // GRAPHICS_ITEM_ORIGIN_VISUALIZER_ENTRY_H
