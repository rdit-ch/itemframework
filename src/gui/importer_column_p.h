#ifndef IMPORTER_COLUMN_P_H
#define IMPORTER_COLUMN_P_H

#include "gui/importer_column.h" // TODO: is this the right place for the header file?
#include "helper/ttype.h"

#include <QVector>
#include <QVariant>
#include <QString>

class ImporterColumnPrivate
{
public:
    QVector<QVariant> _data;
    QString _identifier;
    QString _description;
    TType::StorageType _type;
    TType::StorageOptions _options;
    QVector<ImporterColumn> _children;
    QVariant _userdata{};
};

#endif // IMPORTER_COLUMN_P_H

