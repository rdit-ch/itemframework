#include "item_toolbox.h"
#include "ui_graphics_item_toolbox.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>

#include "item/abstract_item.h"
#include "item_list_model.h"
#include "item_scene.h"

Item_Toolbox::Item_Toolbox(QWidget* parent) :
    QDockWidget(parent),
    ui(new Ui::Graphics_Item_Toolbox)
{
    ui->setupUi(this);
    model = new Item_List_Model(parent);
    ui->treeView->setModel(model);
}

void Item_Toolbox::addItem(AbstractItem* g_item)
{
    QStandardItem* item = new QStandardItem(g_item->typeName());
    item->setData(QIcon(QPixmap::fromImage(g_item->image())), Qt::DecorationRole); //set icon for treeview
    item->setData(g_item->metaObject()->className(), Qt::UserRole); //set class name for drag&drop (mimedata)

    // drag&drop pixmap
    ItemScene scene{{}};
    QPixmap pixmap = scene.createPixmap({g_item}, g_item->boundingRect());
    item->setData(pixmap, Qt::UserRole + 1);
    model->appendRow(item);
}


Item_Toolbox::~Item_Toolbox()
{
    delete ui;
}
