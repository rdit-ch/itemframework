#include "include/gui/importer_column.h"
#include "importer_column_p.h"

#include <QtAlgorithms>

ImporterColumn::ImporterColumn()
    : d_ptr(new ImporterColumnPrivate())
{
}

ImporterColumn::ImporterColumn(QString identifier, QString description)
    : d_ptr(new ImporterColumnPrivate())
{
    Q_D(ImporterColumn);

    d->_identifier = identifier;
    d->_description = description;
}

ImporterColumn::ImporterColumn(ImporterColumn const& other)
    : d_ptr(new ImporterColumnPrivate(*other.d_ptr.data()))
{
}

ImporterColumn::~ImporterColumn()
{
    Q_D(ImporterColumn);

    d->_data.clear();

    // TODO qDeleteAll(d->_children);
    d->_children.clear();
}

ImporterColumn& ImporterColumn::operator=(ImporterColumn const& other)
{
    Q_D(ImporterColumn);

    if (this != &other) {
        auto& o = other.d_ptr;

        // TODO: d = o; ?

        d->_identifier = o->_identifier;
        d->_description = o->_description;
        d->_data = o->_data;
        d->_type = o->_type;
        d->_options = o->_options;
        d->_userdata = o->_userdata;
        d->_children = o->_children;
    }

    return *this;
}

QString ImporterColumn::identifier() const
{
    Q_D(const ImporterColumn);

    return d->_identifier;
}

void ImporterColumn::setIdentifier(const QString& identifier)
{
    Q_D(ImporterColumn);

    d->_identifier = identifier;
}

QString ImporterColumn::description() const
{
    Q_D(const ImporterColumn);

    return d->_description;
}

void ImporterColumn::setDescription(const QString& description)
{
    Q_D(ImporterColumn);

    d->_description = description;
}

QVector<QVariant> ImporterColumn::data() const
{
    Q_D(const ImporterColumn);

    return d->_data;
}

void ImporterColumn::setData(QVector<QVariant>& data)
{
    Q_D(ImporterColumn);

    if (! d->_data.empty()) {
        d->_data.clear();
    }

    d->_data = data;
}

QVariant ImporterColumn::userdata() const
{
    Q_D(const ImporterColumn);

    return d->_userdata;
}

void ImporterColumn::setUserdata(QVariant data)
{
    Q_D(ImporterColumn);

    d->_userdata = data;
}

bool ImporterColumn::hasChildren() const
{
    Q_D(const ImporterColumn);

    return ! d->_children.empty();
}

QVector<ImporterColumn> const& ImporterColumn::children() const
{
    Q_D(const ImporterColumn);

    return d->_children;
}

void ImporterColumn::append(QVector<ImporterColumn>& rows)
{
    Q_D(ImporterColumn);

    d->_children.append(rows);
}

void ImporterColumn::append(ImporterColumn row)
{
    Q_D(ImporterColumn);

    d->_children.append(row);
}
