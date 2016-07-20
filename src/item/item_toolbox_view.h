#ifndef GRAPHICS_ITEM_TOOLBOX_VIEW_H
#define GRAPHICS_ITEM_TOOLBOX_VIEW_H

#include <QTreeView>

class Item_Toolbox_View : public QTreeView
{
    Q_OBJECT
public:
    explicit Item_Toolbox_View(QWidget* parent = 0);

protected:
    virtual void startDrag(Qt::DropActions supportedActions);

};

#endif // GRAPHICS_ITEM_TOOLBOX_VIEW_H
