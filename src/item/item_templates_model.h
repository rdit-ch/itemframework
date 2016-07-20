#ifndef ITEMTEMPLATESMODEL_H
#define ITEMTEMPLATESMODEL_H

#include <QStandardItemModel>
#include <QMimeData>
#include <QDomDocument>

#include "helper/startup_helper.h"

class ItemTemplatesModel : public QStandardItemModel
{
    Q_OBJECT

public:
    using Template = QPair<QString, QDomDocument>;
    using TemplatesContainer = QVector<Template>;

    explicit ItemTemplatesModel(QObject* parent = nullptr);
    bool canDropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent) const;
    QMimeData* mimeData(QModelIndexList const& indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool isDuplicate(QString const& text) const;
    void deleteTemplates(QModelIndexList const& indexes);

    static void init();

signals:
    void templateRenamed(QString const& oldName, QString const& newName);

private:
    TemplatesContainer _templates;

    void loadTemplates();
    void loadTemplates(TemplatesContainer const& templates);
};

Q_DECLARE_METATYPE(ItemTemplatesModel::TemplatesContainer)
Q_DECLARE_METATYPE(ItemTemplatesModel::Template)

STARTUP_ADD_COMPONENT(ItemTemplatesModel)

#endif // ITEMTEMPLATESMODEL_H
