#include "item_templates_model.h"

#include "helper/settings_scope.h"
#include "item_scene.h"
#include "project/project_gui.h"

#include <functional>

char const* const mimeType = "application/x-itemframework-items";
char const* const settingsKey = "ItemTemplates";

template <typename F, typename T>
auto bind1st(F&& f, T&& t) -> decltype(std::bind(std::forward<F>(f), std::forward<T>(t), std::placeholders::_1))
{
    return std::bind(std::forward<F>(f), std::forward<T>(t), std::placeholders::_1);
}

template <typename F, typename T>
auto bind2nd(F&& f, T&& t) -> decltype(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<T>(t)))
{
    return std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<T>(t));
}

ItemTemplatesModel::ItemTemplatesModel(QObject* parent)
    : QStandardItemModel{parent}, _templates{}
{
    loadTemplates();

    auto for_each_template = [this](int first, int last, QModelIndex const& parent,
                                    std::function<void(QString const&, QDomDocument const&)> f) {
        for (int current = first; current <= last; current++) {
            auto nameVariant = this->data(this->index(current, 0, parent));
            auto templateVariant = this->data(this->index(current, 0, parent), Qt::UserRole);

            if (nameVariant.isValid() && templateVariant.isValid()) {
                auto name = nameVariant.value<QString>();
                auto documentBytes = templateVariant.value<QByteArray>();
                QDomDocument templates{};
                templates.setContent(documentBytes);

                f(name, templates);
            }
        }
    };

    auto has_name = [](Template const& templ, QString const& name) {
        return templ.first == name;
    };

    connect(this, &ItemTemplatesModel::rowsInserted,
            [=](QModelIndex const& parent, int first, int last) {
        for_each_template(first, last, parent, [this](QString const& name, QDomDocument const& templ) {
            Template itemTemplate{name, templ};
            this->_templates.append(itemTemplate);
        });
        SettingsScope::globalScope()->setValue(settingsKey, QVariant::fromValue(_templates));
    });

    connect(this, &ItemTemplatesModel::rowsAboutToBeRemoved,
            [=](QModelIndex const& parent, int first, int last) {
        for_each_template(first, last, parent, [=](QString const& name, QDomDocument const&) {
                auto match = std::find_if(_templates.begin(), _templates.end(), bind2nd(has_name, name));
                if (match != _templates.cend()) {
                    _templates.erase(match);
                }
        });
        SettingsScope::globalScope()->setValue(settingsKey, QVariant::fromValue(_templates));
    });

    connect(this, &ItemTemplatesModel::templateRenamed, [=](QString const& oldName, QString const& newName) {
        auto match = std::find_if(_templates.begin(), _templates.end(), bind2nd(has_name, oldName));
        if (match != _templates.end()) {
            match->first = newName;

            SettingsScope::globalScope()->setValue(settingsKey, QVariant::fromValue(_templates));
        }
    });
}

void ItemTemplatesModel::loadTemplates()
{
    auto templatesVariant = SettingsScope::globalScope()->value(settingsKey);

    if (templatesVariant.isValid()) {
        auto templates = templatesVariant.value<TemplatesContainer>();

        loadTemplates(templates);
    }
}

void ItemTemplatesModel::loadTemplates(TemplatesContainer const& templates)
{
    _templates = templates;

    auto loadTemplate = [this](Template const& templ) {
        auto templateName = templ.first;
        auto templateDocument = templ.second;
        auto item = new QStandardItem{templateName};

        ItemScene scene{{}};
        auto pixmap = scene.createPixmap(templateDocument);
        if (pixmap.isNull()) {
            qDebug() << "pixmap for template " << templateName << " is null";
            return;
        }
        item->setData(templ.second.toByteArray(), Qt::UserRole);
        item->setData(pixmap, Qt::UserRole + 1);

        this->appendRow(item);
    };

    std::for_each(_templates.cbegin(), _templates.cend(), loadTemplate);
}

bool ItemTemplatesModel::canDropMimeData(QMimeData const*, Qt::DropAction, int, int, QModelIndex const&) const
{
    return false;
}

bool ItemTemplatesModel::dropMimeData(QMimeData const*, Qt::DropAction, int, int, QModelIndex const&)
{
    return false;
}

QStringList ItemTemplatesModel::mimeTypes() const
{
    return { mimeType };
}

QMimeData* ItemTemplatesModel::mimeData(QModelIndexList const& indexes) const
{
    auto mimeData = new QMimeData{};

    mimeData->setData(mimeType, data(indexes.first(), Qt::UserRole).toString().toLatin1());

    return mimeData;
}

bool ItemTemplatesModel::setData(QModelIndex const& index, QVariant const& value, int role)
{
    if (role == Qt::EditRole) {
        auto newText = value.toString();
        if (isDuplicate(newText)) {
            return false;
        } else {
            auto oldText = this->data(index).toString();
            emit templateRenamed(oldText, newText);
        }
    }

    return QStandardItemModel::setData(index, value, role);
}

bool ItemTemplatesModel::isDuplicate(QString const& text) const
{
    auto items = findItems(text);

    return !items.isEmpty();
}

void ItemTemplatesModel::init()
{
    qRegisterMetaType<Template>();
    qRegisterMetaType<TemplatesContainer>();
    qRegisterMetaTypeStreamOperators<Template>();
    qRegisterMetaTypeStreamOperators<TemplatesContainer>();
}

QDataStream& operator<<(QDataStream& out, ItemTemplatesModel::Template const& t)
{
    out << t.first;
    out << t.second.toByteArray();

    return out;
}

QDataStream& operator>>(QDataStream& in, ItemTemplatesModel::Template& t)
{
    QByteArray xml;
    QString name;

    in >> name;
    in >> xml;

    t.first = name;
    t.second.setContent(xml);

    return in;
}

QDataStream& operator<<(QDataStream& out, ItemTemplatesModel::TemplatesContainer const& templates)
{
    out << templates.size();

    for (auto t : templates) {
        out << t;
    }

    return out;
}

QDataStream& operator>>(QDataStream& in, ItemTemplatesModel::TemplatesContainer& templates)
{
    int size;
    in >> size;

    templates.reserve(size);
    for (int i = 0; i < size; i++) {
        ItemTemplatesModel::Template t;

        in >> t;
        templates.append(t);
    }

    return in;
}
