#ifndef PLUGIN_TABLE_MODEL_H
#define PLUGIN_TABLE_MODEL_H

#include "plugin_meta_data.h"
#include <QAbstractTableModel>
#include <QSharedPointer>
#include <QList>

class PluginTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    explicit PluginTableModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const ;
    QModelIndex index(int row, int column, const QModelIndex&) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    void appendPluginMetaDataList(QList<PluginMetaData*>);

private:
    QList<QSharedPointer<class PluginMetaData>> _metaDataList;

signals:
    void pluginMetaDataChanged(PluginMetaData*);
};
#endif // PLUGIN_TABLE_MODEL_H

