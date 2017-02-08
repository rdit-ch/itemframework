#include "item_toolbox_view.h"
#include <QDrag>

Item_Toolbox_View::Item_Toolbox_View(QWidget* parent) :
    QTreeView(parent)
{
}

void Item_Toolbox_View::startDrag(Qt::DropActions supportedActions)
{
    QVariant pixvariant = model()->data(selectedIndexes().first(), Qt::UserRole + 1); //try fetching drag pixmap

    if (pixvariant.isValid() && pixvariant.type() == QVariant::Pixmap) { //theres indeed a pixmap available
        //Code Copied (+Modified) from: https://qt.gitorious.org/qt/qtbase/source/159465536f0dd1a373f0c67e5530f9af6ccec433:src/widgets/itemviews/qabstractitemview.cpp#L3583-3606

        QDrag* drag = new QDrag(this);
        QMimeData* mimedata = model()->mimeData(selectedIndexes());
        drag->setMimeData(mimedata);

        QPixmap pixmap = pixvariant.value<QPixmap>();
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint((pixmap.size().width() + 1) / 2, (pixmap.size().height() + 1) / 2));


        Qt::DropAction action = Qt::IgnoreAction;

        if (defaultDropAction() != Qt::IgnoreAction && (supportedActions & defaultDropAction())) {
            action = defaultDropAction();
        } else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove) {
            action = Qt::CopyAction;
        }

        drag->exec(supportedActions, action);

    } else { //no drag pixmap
        QTreeView::startDrag(supportedActions); //call default implementation
    }
}
