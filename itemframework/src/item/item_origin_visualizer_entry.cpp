#include "item_origin_visualizer_entry.h"
#include "ui_graphics_item_origin_visualizer_entry.h"
#include "item/abstract_item.h"

#include <QStylePainter>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QMenu>

Item_Origin_Visualizer_Entry::Item_Origin_Visualizer_Entry(AbstractItem* item, QWidget* parent) :
    QFrame(parent), ui(new Ui::Graphics_Item_Origin_Visualizer_Entry)
{
    ui->setupUi(this);
    this->item = item;
    connect(item, SIGNAL(descriptionChanged()), this, SLOT(update_content()));
    connect(item, SIGNAL(nameChanged()), this, SLOT(update_content()));
    update_content();

    int w = 100;
    int h = ui->lab1->fontMetrics().height();

    //todo: antialaised scaling
    ui->icon->setPixmap(QPixmap::fromImage(item->image().scaled(w, h, Qt::KeepAspectRatio)));
}

Item_Origin_Visualizer_Entry::~Item_Origin_Visualizer_Entry()
{
    delete ui;
}

void Item_Origin_Visualizer_Entry::update_content()
{
    ui->lab1->setText(item->name());
    QString desc = item->description();
    ui->lab2->setText(desc);
    ui->lab2->setVisible(desc.trimmed().length());
}

void Item_Origin_Visualizer_Entry::mouseDoubleClickEvent(QMouseEvent*)
{
    QGraphicsSceneMouseEvent e(QMouseEvent::MouseButtonDblClick);

    item->grabMouse();
    item->mouseDoubleClickEvent(&e);
}

void Item_Origin_Visualizer_Entry::contextMenuEvent(QContextMenuEvent*)
{
    // load target items context menu entries
    QMenu menu;
    item->contextMenuPrepare(menu);

    // display context menu
    auto mousePos = QCursor::pos();
    menu.exec(mousePos);
}
