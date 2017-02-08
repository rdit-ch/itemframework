#ifndef GRAPHICS_ITEM_LIST_MODEL_H
#define GRAPHICS_ITEM_LIST_MODEL_H

#include <QStandardItemModel>

class Item_List_Model : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit Item_List_Model(QObject* parent = 0);
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
protected:
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual QStringList mimeTypes() const;
};

#endif // GRAPHICS_ITEM_LIST_MODEL_H
