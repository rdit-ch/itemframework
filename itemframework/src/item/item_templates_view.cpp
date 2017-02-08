#include "item_templates_view.h"

#include <QDrag>

ItemTemplatesView::ItemTemplatesView(QWidget* parent)
    : QListView{parent}
{
    setDragEnabled(true);
    setDragDropMode(DragOnly);
}

void ItemTemplatesView::startDrag(Qt::DropActions supportedActions)
{
    auto drag = new QDrag{this};
    auto mimeData = model()->mimeData(selectedIndexes());

    auto pixvariant = model()->data(selectedIndexes().first(), Qt::UserRole + 1); //try fetching drag pixmap
    auto pixmap = pixvariant.value<QPixmap>();
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint((pixmap.size().width() + 1) / 2, (pixmap.size().height() + 1) / 2));

    auto dropAction = Qt::IgnoreAction;

    if (defaultDropAction() != Qt::IgnoreAction &&
            (supportedActions & defaultDropAction())) {
        dropAction = defaultDropAction();
    } else if (supportedActions & Qt::CopyAction &&
               dragDropMode() != QAbstractItemView::InternalMove) {
        dropAction = Qt::CopyAction;
    }

    drag->exec(supportedActions, dropAction);
}
