#include "item_toolbox.h"
#include "ui_graphics_item_toolbox.h"
#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include "item/abstract_item.h"
#include "item_list_model.h"

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
    QStandardItem* item = new QStandardItem(g_item->typeName()); //Create Item and set text
    item->setData(QIcon(QPixmap::fromImage(g_item->image())), Qt::DecorationRole); //set icon for treeview
    item->setData(g_item->metaObject()->className(), Qt::UserRole); //set class name for drag&drop (mimedata)

    //Paint the item into a pixmap for the drag&drop pixmap
    QRectF bounding = g_item->boundingRect();
    QPixmap pixmap = QPixmap(bounding.size().toSize());
    pixmap.fill(Qt::white);
    QPainter p(&pixmap);
    p.translate(-bounding.topLeft());
    p.setRenderHint(QPainter::Antialiasing);
    g_item->paint(&p, 0, 0);
    p.end();
    item->setData(pixmap, Qt::UserRole + 1);

    model->appendRow(item); //set the drag&drop pixmap
}


Item_Toolbox::~Item_Toolbox()
{
    delete ui;
}
