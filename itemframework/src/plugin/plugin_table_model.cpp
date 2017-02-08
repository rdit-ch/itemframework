#include "plugin/plugin_table_model.h"
#include "plugin/plugin_manager.h"
#include <QMetaProperty>
#include <QDebug>
#include <QBrush>
#include <QStandardItem>

PluginTableModel::PluginTableModel(QObject* parent) : QAbstractTableModel(parent)
{
}

int PluginTableModel::rowCount(const QModelIndex&) const
{
    return  _metaDataList.count();
}

int PluginTableModel::columnCount(const QModelIndex&) const
{
    return  PluginMetaData::staticMetaObject.propertyCount() - PluginMetaData::staticMetaObject.propertyOffset();
}

Qt::ItemFlags PluginTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        if (index.model()->headerData(index.column(), Qt::Horizontal).toString() == "Enabled") {
            flags |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable ;
        } else {
            flags |= Qt::ItemNeverHasChildren | Qt::ItemIsEnabled;
        }
    }

    return flags;
}

QVariant PluginTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.model() != this || index.internalPointer() == nullptr) {
        return QVariant();
    }

    QString field = index.model()->headerData(index.column(), Qt::Horizontal).toString();

    switch (role) {
    // get the checkbox data
    case Qt::CheckStateRole: {
        if (field == "Enabled") {
            if (_metaDataList.at(index.row())->isPluginEnabled() == true) {
                return QVariant(static_cast<int>(Qt::Checked));
            } else {
                return QVariant(static_cast<int>(Qt::Unchecked));
            }
        }

        return QVariant();
    }

    // get plugin meta data
    case Qt::DisplayRole: {
        if (field != "Enabled") {
            int propIndex = index.column() + PluginMetaData::staticMetaObject.propertyOffset();
            return PluginMetaData::staticMetaObject.property(propIndex).read(reinterpret_cast<QObject*>(index.internalPointer()));
        }
    }

    // draw background
    case Qt::BackgroundRole: {
        if (!_metaDataList.at(index.row()).data()->isCompatible()) {
            return QVariant(QBrush(QColor(Qt::lightGray)));
        }
    }

    // text color
    case Qt::ForegroundRole: {
        if (!_metaDataList.at(index.row()).data()->isCompatible()) {
            return QVariant(QBrush(QColor(Qt::red)));
        }
    }
    }

    return QVariant();
}


bool PluginTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid()) {
        QString field = index.model()->headerData(index.column(), Qt::Horizontal).toString();

        if (field == "Enabled") {
            if (role == Qt::CheckStateRole) {
                PluginMetaData* metaData = reinterpret_cast<PluginMetaData*>(index.internalPointer());

                if (value == Qt::Checked) {
                    if (metaData->isCompatible()) {
                        metaData->setPluginEnabled(true);
                        emit pluginMetaDataChanged(metaData);
                    }
                } else if (value == Qt::Unchecked) {
                    if (metaData->isCompatible()) {
                        metaData->setPluginEnabled(false);
                        emit pluginMetaDataChanged(metaData);
                    }
                }
            }

            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
    }

    return false;
}

QModelIndex PluginTableModel::index(int row, int column, const QModelIndex&) const
{
    if (hasIndex(row, column)) {
        return createIndex(row, column, _metaDataList[row].data());
    }

    return QModelIndex();
}

QVariant PluginTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section > columnCount()) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        int propIndex = section + PluginMetaData::staticMetaObject.propertyOffset();
        return tr(PluginMetaData::staticMetaObject.property(propIndex).name());
    }

    return QVariant();
}

void PluginTableModel::appendPluginMetaDataList(QList<PluginMetaData*> metaDataList)
{
    beginResetModel();

    foreach (PluginMetaData* metaData, metaDataList) {
        _metaDataList.append(QSharedPointer<PluginMetaData>(metaData));
    }

    endResetModel();
}
