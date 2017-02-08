#ifndef GRAPHICS_ITEM_TOOLBOX_H
#define GRAPHICS_ITEM_TOOLBOX_H

#include <QDockWidget>

namespace Ui
{
class Graphics_Item_Toolbox;
}

class Item_List_Model;
class Item_Toolbox : public QDockWidget
{
    Q_OBJECT

public:
    explicit Item_Toolbox(QWidget* parent = 0);
    ~Item_Toolbox();
    void addItem(class AbstractItem* item);
private:
    Item_List_Model* model;

private:
    Ui::Graphics_Item_Toolbox* ui;
};

#endif // GRAPHICS_ITEM_TOOLBOX_H
