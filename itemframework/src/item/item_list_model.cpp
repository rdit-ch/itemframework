#include "item_list_model.h"
#include <QMimeData>

Item_List_Model::Item_List_Model(QObject* parent) :
    QStandardItemModel(parent)
{
}

bool Item_List_Model::canDropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&) const
{
    return false; //disallow dropping of stuff
}

bool Item_List_Model::dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&)
{
    return false; //disallow dropping of stuff
}

QMimeData* Item_List_Model::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.length() != 1) { //we can only handle exactly one item
        return NULL;
    }

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-itemframework-item", data(indexes.first(), Qt::UserRole).toString().toLatin1()); //set mime data to UserRole of the passed row
    return mimeData;
}

QStringList Item_List_Model::mimeTypes() const
{
    return (QStringList() << "application/x-itemframework-item"); //set the mime type returned by this model
}
