#include "item_serializer.h"
#include "item/abstract_item.h"
#include "item/item_input.h"
#include "item/item_output.h"
#include "item/item_note.h"
#include "item/item_connector.h"
#include "plugin/plugin_manager.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsObject>

#include <functional>

static const char* const GraphicsItemTag = "GraphicsItem";
static const char* const TypeAttrTag = "type";
static const char* const NameAttrTag = "name";
static const char* const XPosAttrTag = "x";
static const char* const YPosAttrTag = "y";
static const char* const IdAttrTag = "id";
static const char* const GraphicsItemDataTag = "GraphicsItemData";
static const char* const GraphicsItemConnectorTag = "GraphicsItemConnector";
static const char* const FromItemAttrTag = "fromItem";
static const char* const FromIndexAttrTag = "fromIndex";
static const char* const ToItemAttrTag = "toItem";
static const char* const ToIndexAttrTag = "toIndex";
static const char* const TransportTypeAttrTag = "transportType";
static const char* const GraphicsItemNoteTag = "GraphicsItemNote";
static const char* const GraphicsItemNoteDataTag = "GraphicsItemNoteData";

template <typename T>
inline QList<T const*> const& constList(QList<T*> const& list)
{
    return reinterpret_cast<QList<T const*> const&>(list);
}

// This is a template because even though ItemNote has a similar interface to
// the common AbstractItem base class, it does not inherit from it, so
// polymorphism does not work.
// In particular, both classes implement load(QDomElement const&), which we use
// here.
template <typename T>
bool loadItemData(QDomElement const& element, T* item, char const* const tag)
{
    if (element.hasChildNodes()) {
        QDomElement dataElement  = element.firstChild().toElement();

        if (dataElement.isNull()) {
            qDebug() << "Item has non element subtype";
            return false;
        }

        if (dataElement.tagName() != tag) {
            qDebug() << "Unknown subelementtype for Item";
            return false;
        }

        if (!item->load(dataElement)) {
            qWarning() << "Couldn't load item data. (Wrong version?)";
            return false;
        }
    }

    return true;
}

bool ItemSerializer::loadFromXml(QDomElement const& items,
                                 QList<QGraphicsItem*>* itemsOut,
                                 ProgressReporter progress,
                                 bool shouldConnectIO)
{
    if (itemsOut == nullptr) {
        return false;
    }

    if (items.childNodes().size() == 0) {
        return true;
    }

    auto countChildrenIf = [](QDomElement const& parent,
                              std::function<bool(QDomElement const&)> predicate) {
        int count = 0;
        for (QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (predicate(n.toElement())) {
                count++;
            }
        }
        return count;
    };

    auto extractAbstractItem = [](QDomElement const& element)
            -> std::pair<AbstractItem*, quint32> {
        QString const type = element.attribute(TypeAttrTag);
        QScopedPointer<AbstractItem> itemPtr{
            PluginManager::instance()->createInstance<AbstractItem>(type)
        };

        if (itemPtr.isNull()) {
            qDebug() << "Couldn't cast Item to" << type;
            qDebug() << "Wrong version of Plugin?";
            return {};
        }

        if (!loadItemData(element, itemPtr.data(), GraphicsItemDataTag)) {
            return {};
        }

        QString const name = element.attribute(NameAttrTag);
        qreal   const x    = element.attribute(XPosAttrTag).toDouble();
        qreal   const y    = element.attribute(YPosAttrTag).toDouble();
        qint32  const id   = element.attribute(IdAttrTag).toInt();

        itemPtr->setPos(x, y);
        itemPtr->setName(name);

        return {itemPtr.take(), id};
    };

    auto extractConnector = [shouldConnectIO](QDomElement const& element,
            QHash<qint32, AbstractItem*> const& itemIds) -> Item_Connector* {

        auto interval = [](int const low, int const high) {
            return std::make_pair(low, high);
        };

        auto contains = [](std::pair<int, int> const& interval, int const value) {
            auto const low  = interval.first;
            auto const high = interval.second;
            return value >= low && value < high;
        };

        auto loadConnector = [](QDomElement const& element, ItemInput* input, ItemOutput* output) {
            auto connector = new Item_Connector(output, input);
            connector->load_additional(element); // Load user modified path or custom routing
            connector->do_update();
            return connector;
        };

        qint32  const sourceObject          = element.attribute(FromItemAttrTag).toInt();
        qint32  const sourceOutputIndex     = element.attribute(FromIndexAttrTag).toInt();
        qint32  const destinationObject     = element.attribute(ToItemAttrTag).toInt();
        qint32  const destinationInputIndex = element.attribute(ToIndexAttrTag).toInt();
        QString const typeName              = element.attribute(TransportTypeAttrTag);
        qint32  const typeId                = QMetaType::type(typeName.toStdString().c_str());

        if (!itemIds.contains(sourceObject) || !itemIds.contains(destinationObject)) {
            qCritical() << "Couldn't find begin and end object of connector. Connector deleted.";
            return nullptr;

        } else if (typeId == QMetaType::UnknownType) {
            qCritical() << "Unknown transport type \"" << typeName << "\"! Connector deleted.";
            return nullptr;
        }

        auto sourceItem      = itemIds[sourceObject];
        auto destinationItem = itemIds[destinationObject];

        auto const outputs = sourceItem->outputs();
        auto const inputs  = destinationItem->inputs();

        if (!contains(interval(0, outputs.count()), sourceOutputIndex)) {
            qCritical() << QString("Couldn't find ouput for connector on item %1. Connector deleted.").arg(sourceItem->name());
            return nullptr;
        }

        if (!contains(interval(0, inputs.count()), destinationInputIndex)) {
            qCritical() << QString("Couldn't find input for connector on item %1. Connector deleted.").arg(destinationItem->name());
            return nullptr;
        }

        auto input  = inputs.at(destinationInputIndex);
        auto output = outputs.at(sourceOutputIndex);

        if (input->transportType() != typeId) {
            qCritical() << QString("Connector Type is %1. Input of Item %2 is of Type %3. Connector deleted.").arg(typeId).arg(destinationItem->name()).arg(input->transportType());
            return nullptr;
        }

        if (output->transportType() != typeId) {
            qCritical() << QString("Connector Type is %1. Output of Item %2 is of Type %3. Connector deleted.").arg(typeId).arg(sourceItem->name()).arg(output->transportType());
            return nullptr;
        }

        // Disable signals if an actual data connection is not required,
        // for example when we are only loading these items in order to
        // render a pixmap image of their appearance.  Simply not connecting
        // the input and output would not work since their appearance depends
        // on whether or not they are connected.
        if (!shouldConnectIO) {
            input->blockSignals(true);
            output->blockSignals(true);
        }
        input->connectOutput(output); // Data connection

        return loadConnector(element, input, output);
    };

    auto extractNote = [](QDomElement const& element) -> ItemNote* {
        auto loadNote = [](QDomElement const& element) {
            auto note = new ItemNote();
            qreal const x = element.attribute(XPosAttrTag).toDouble();
            qreal const y = element.attribute(YPosAttrTag).toDouble();
            note->setPos(x, y);
            return note;
        };

        QScopedPointer<ItemNote> notePtr{loadNote(element)};

        if (!loadItemData(element, notePtr.data(), GraphicsItemNoteDataTag)) {
            return {};
        }

        return notePtr.take();
    };

    auto const progressGoal = countChildrenIf(items, [](QDomElement const& element) {
        return element.tagName() == GraphicsItemTag ||
               element.tagName() == GraphicsItemNoteTag;
    });

    progress.reset(progressGoal, 0);
    progress.report();

    QHash<qint32, AbstractItem*> itemIds{};

    // since itemsOut is a pointer, it could also be captured by value, but we
    // want to express that it is written to.
    auto extractGraphicsItem = [&itemIds, &itemsOut,
                                extractAbstractItem,
                                extractConnector, extractNote]
            (QDomElement const& element, ProgressReporter& progress) {
        auto const tagName = element.tagName();

        if (tagName == GraphicsItemTag) {
            progress.advance();
            progress.report("Loading graphics item ...");
            AbstractItem *item;
            qint32 id;
            std::tie(item, id) = extractAbstractItem(element);
            if (item != nullptr) {
                itemsOut->append(item);
                itemIds.insert(id, item);
                progress.report("Successfully loaded item of type " + item->typeName());
                return true;
            } else {
                qWarning() << "Failed to load graphics item!";
                return false;
            }

        } else if (tagName == GraphicsItemConnectorTag) {
            // Do not advance progress as connectors are not included in the total item count.
            progress.report("Loading connection between items");

            auto connector = extractConnector(element, itemIds);
            itemsOut->append(connector);

            return true;

        } else if (tagName == GraphicsItemNoteTag) {
            progress.advance();
            progress.report("Loading notes");

            auto note = extractNote(element);
            itemsOut->append(note);

            return true;
        }

        return false;
    };

    for (QDomNode node = items.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement const element = node.toElement();

        if (element.isNull()) {
            continue;
        }

        if (!extractGraphicsItem(element, progress)) {
            qWarning() << "Failed to load item from element tagged " << element.tagName();
        }
    }

    progress.done();
    progress.report("Finished loading items");

    return true;
}

bool ItemSerializer::saveToXml(QDomDocument& document,
                               QDomElement& xml,
                               QList<QGraphicsItem const*> itms)
{
    qint32 itemCount = 0;
    QHash<AbstractItem const*, qint32> idMap;

    foreach (QGraphicsItem const* gitm, itms) {
        if (gitm->type() == QGraphicsLineItem::Type) { //Temporary connection line
            continue;
        }

        auto item = qobject_cast<AbstractItem const*>(gitm->toGraphicsObject());

        if (item != nullptr) {
            QDomElement element = document.createElement(GraphicsItemTag);
            element.setAttribute(TypeAttrTag, item->metaObject()->className());
            element.setAttribute(XPosAttrTag, item->pos().x());
            element.setAttribute(YPosAttrTag, item->pos().y());
            element.setAttribute(IdAttrTag, itemCount);
            element.setAttribute(NameAttrTag, item->name());

            QDomElement dataelement = document.createElement(GraphicsItemDataTag);

            if (!item->save(document, dataelement)) {
                qWarning() << item->metaObject()->className() << "Couldn't save it's data.";
            }

            if (dataelement.hasAttributes() || dataelement.hasChildNodes()) {
                element.appendChild(dataelement);
            }

            idMap.insert(item, itemCount++);
            xml.appendChild(element);
        } else {
            auto note = qobject_cast<ItemNote const*>(gitm->toGraphicsObject());

            if (note != nullptr) {
                QDomElement element = document.createElement(GraphicsItemNoteTag);
                element.setAttribute(XPosAttrTag, note->pos().x());
                element.setAttribute(YPosAttrTag, note->pos().y());

                QDomElement dataelement = document.createElement(GraphicsItemNoteDataTag);

                if (!note->save(document, dataelement)) {
                    qWarning() << "Note Couldn't save it's data.";
                }

                if (dataelement.hasAttributes() || dataelement.hasChildNodes()) {
                    element.appendChild(dataelement);
                }

                xml.appendChild(element);
            }

        }
    }

    foreach (QGraphicsItem const* gitm, itms) {
        if (gitm->type() == QGraphicsLineItem::Type) {
            continue;
        }

        auto connector = qobject_cast<Item_Connector const*>(gitm->toGraphicsObject());

        if (connector == nullptr) {
            continue;
        }

        ItemOutput* output = connector->get_output();
        AbstractItem* outputOwner = output->owner();
        ItemInput* input  = connector->get_input();
        AbstractItem* inputOwner = input->owner();

        QDomElement element = document.createElement(GraphicsItemConnectorTag);
        element.setAttribute(FromItemAttrTag, idMap[outputOwner]);
        element.setAttribute(FromIndexAttrTag, output->owner()->outputs().indexOf(output));

        element.setAttribute(ToItemAttrTag, idMap[inputOwner]);
        element.setAttribute(ToIndexAttrTag, input->owner()->inputs().indexOf(input));

        element.setAttribute(TransportTypeAttrTag, QMetaType::typeName(output->transportType()));

        connector->save_additional(document, element);

        xml.appendChild(element);
    }

    return true;
}
