#include "item_scene.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QMimeData>
#include <QDebug>
#include <QMenu>
#include <QDomElement>
#include <QHash>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QInputDialog>
#include <qmath.h>
#include <functional>

#include "item/abstract_item.h"
#include "item/item_view.h"
#include "item/item_input.h"
#include "item/item_output.h"
#include "item_connector.h"
#include "item_note.h"
#include "plugin/plugin_manager.h"
#include "res/resource.h"
#include "project/project_gui.h"
#include "item_templates_widget.h"
#include "gui/gui_manager.h"
#include "helper/settings_scope.h"

#define RASTER 50.0 //Raster to 50px
#define AUTOCONNECT_DISTANCE  30 //connect line if nearer than 10px

static const char* const DragDropMimeType = "application/x-itemframework-item";
static const char* const CopyPasteMimeType = "application/x-itemframework-items";
static const char* const CopyPasteDocType = "ItemframeworkItemData";
static const char* const CopyPasteRootTag = "items";
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

#include "item/item_manager.h"

ItemScene::ItemScene(QSharedPointer<ProjectGui> projectGui, QObject* parent) : QGraphicsScene(parent)
{
    _projectGui = projectGui;
    //setItemIndexMethod(QGraphicsScene::NoIndex);
}

ItemScene::~ItemScene()
{
}

void ItemScene::updateConnectionLine()
{
    // endPoint changed due to mouse movement. Realign the connection line, possibly
    // connecting the end of the connection line to a nearby input or output

    QPointF mousePosition = _endPoint;

    //Find all items which boundingrect intersects with a HitTestBox around the mouseposition
    //Note that the background is also in the collection!
    QList<QGraphicsItem*> nearbyItems = items(QRectF(mousePosition - QPointF(AUTOCONNECT_DISTANCE, AUTOCONNECT_DISTANCE), QSizeF(2 * AUTOCONNECT_DISTANCE, 2 * AUTOCONNECT_DISTANCE)), Qt::IntersectsItemBoundingRect, Qt::DescendingOrder);

    qreal smallestDistance = AUTOCONNECT_DISTANCE;
    bool found = false;

    for (auto i : nearbyItems) {
        if (i->type() == QGraphicsLineItem::Type) {
            continue;
        }

        // TODO: DRY!

        if (! _reversed) { // connection starts with an input, we need an output
            auto output = qobject_cast<ItemOutput*>(i->toGraphicsObject());

            if (output != nullptr) {
                if (output->transportType() != _type) {
                    continue;
                }

                qreal d = distance(mousePosition, output->scenePosition());

                if (d < smallestDistance) {
                    smallestDistance = d;
                    _endPoint = output->scenePosition();
                    _output = output;
                    found = true;
                }
            }
        } else { // connection starts with an output, we need an input
            auto input = qobject_cast<ItemInput*>(i->toGraphicsObject());

            if (input != nullptr) {
                if (input->transportType() != _type || input->isConnected()) {
                    continue;
                }

                qreal d = distance(mousePosition, input->scenePosition());

                if (d < smallestDistance) {
                    smallestDistance = d;
                    _endPoint = input->scenePosition();
                    _input = input;
                    found = true;
                }
            }
        }
    }

    // TODO: redundant?
    if (! found) {
        if (_reversed) {
            _input = nullptr;
        } else {
            _output = nullptr;
        }
    }

    _connectionLine->setLine(QLineF{_startPoint, _endPoint});
}

QList<QGraphicsItem*> ItemScene::readItems(QDomDocument const& document)
{
    QList<QGraphicsItem*> items;

    QDomElement root = document.documentElement();

    if (!loadFromXml(root, &items, false)) {
        qDebug() << "Error while trying to load items: Copy-Creation from xml failed";
    }

    return items;
}

QList<QGraphicsItem*> ItemScene::readItems(QMimeData const& mimeData, char const* const mimeType, char const* const docType)
{
    QByteArray bar = mimeData.data(mimeType);
    QDomDocument doc;
    QString parserError;


    if (!doc.setContent(bar, &parserError)) {
        qDebug() << "Error while trying to load items:" << parserError;
        return {};
    }

    if (doc.doctype().name() != docType) {
        qDebug() << "Error while trying to load items: Xml has wrong Doctype" << doc.doctype().name();
        return {};
    }

    return readItems(doc);
}

void ItemScene::insertItems(QList<QGraphicsItem*> items, QPointF const& scenePos, QRectF const& boundingBox)
{
    //Add items to scene, and move them to the cursor
    for (QGraphicsItem* itm : items) { //foreach item, connector and note
        addItem(itm);

        if (qobject_cast<Item_Connector*>(itm->toGraphicsObject()) == nullptr) {
            //only for notes and items, but not connectors (as they will auto-update their position)
            auto const itemPositionAtCursor = itm->pos() - boundingBox.center() + scenePos; //move to cursor
            auto const itemPositionRastered = QPointF(RASTER * round(itemPositionAtCursor.x() / RASTER),
                                                      RASTER * round(itemPositionAtCursor.y() / RASTER));
            itm->setPos(itemPositionRastered);
        }

        AbstractItem* aitm = qobject_cast<AbstractItem*>(itm->toGraphicsObject());

        if (aitm != nullptr) { //only for Items: Change item name
            QString name = aitm->name(); //Current name of the item we want to copy
            static QRegularExpression copyBasename(R"/(^(.*)_copy\d+$)/");
            QRegularExpressionMatch match = copyBasename.match(name);

            if (match.hasMatch()) { //current name ends with _copy*
                name = match.captured(1); //take the basename (= whole name without the _copy* suffix)
            }

            _copyCountHash[name]++; //increase copy count, or set it to 1 (if it wasn't in the hash already)
            name = QString("%1_copy%2").arg(name).arg(_copyCountHash[name]);

            aitm->setName(name); //set name of the new item with current copy count
        }
    }

    updateBoundingRect(); //realgin items/scene by recalulating the bounding box
    emit sceneRealChanged();
}

void ItemScene::insertTemplate(QMimeData const* const mimeData, QPointF const& position)
{
    if (mimeData == nullptr || !mimeData->hasFormat(CopyPasteMimeType)) {
        return;
    }

    auto copiedItems = readItems(*mimeData, CopyPasteMimeType, CopyPasteDocType);
    if (copiedItems.size() == 0) {
        qDebug() << "Failed to read items for insertion!";

        return;
    }

    QRectF boundingBox{};
    QPointF scenePos{};

    if (!position.isNull()) {
        boundingBox = calculateBoundingBox(constList(copiedItems), true);
        scenePos = position;
    } else {
        boundingBox = calculateBoundingBox(constList(copiedItems), true);

        //Calculate mouse cursor position in scene coordinates
        QPoint const viewPos = views().first()->viewport()->mapFromGlobal(QCursor::pos());
        scenePos = views().first()->mapToScene(viewPos);
    }

    insertItems(copiedItems, scenePos, boundingBox);
}

void ItemScene::paste()
{
    const QClipboard* const clipboard = QApplication::clipboard();
    const QMimeData* const mimeData = clipboard->mimeData();

    if (mimeData == nullptr || !mimeData->hasFormat(CopyPasteMimeType)) {
        return;
    }

    QList<QGraphicsItem*> copiedItems = readItems(*mimeData, CopyPasteMimeType, CopyPasteDocType);

    //Calculate the boundingBox for the selected items
    QRectF boundingBox = calculateBoundingBox(constList(copiedItems), true);

    //Calculate mouse cursor position in scene coordinates
    QPoint viewPos = views().first()->viewport()->mapFromGlobal(QCursor::pos());
    QPointF scenePos = views().first()->mapToScene(viewPos);

    insertItems(copiedItems, scenePos, boundingBox);
}

QRectF ItemScene::calculateBoundingBox(QList<QGraphicsItem const*> items, bool ignoreConnectors) const
{
    QList<QGraphicsItem const*> items_without_connectors{};
    QRectF boundingBox = items.first()->boundingRect().translated(items.first()->pos());

    auto is_not_connector = [](QGraphicsItem const * item) {
        return qobject_cast<Item_Connector const*>(item->toGraphicsObject()) == nullptr;
    };

    auto fitBoundingBox = [&](QGraphicsItem const * item) {
        boundingBox = boundingBox.united(item->boundingRect().translated(item->pos()));
    };

    if (ignoreConnectors) {
        std::copy_if(items.cbegin(), items.cend(), std::back_inserter(items_without_connectors), is_not_connector);
        std::for_each(items_without_connectors.cbegin(), items_without_connectors.cend(), fitBoundingBox);
    } else {
        std::for_each(items.cbegin(), items.cend(), fitBoundingBox);
    }

    return boundingBox;
}


QList<QGraphicsItem*> ItemScene::copyableItems() const
{
    return ItemScene::copyableItems(selectedItems());
}

QList<QGraphicsItem*> ItemScene::copyableItems(QList<QGraphicsItem*> items)
{
    using namespace std::placeholders;

    auto is_temporary_connection_line = [](QGraphicsItem const * item) {
        return item->type() == QGraphicsLineItem::Type;
    };

    auto both_ends_in_list = [&](QGraphicsItem const * item) {
        auto connector = qobject_cast<Item_Connector const*>(item->toGraphicsObject());

        if (connector) {
            if (!items.contains(connector->get_output()->owner()) || !items.contains(connector->get_input()->owner())) {
                return false;
            }
        }

        return true;
    };

    auto shouldRemove = std::bind(std::logical_or<bool> {},
                                  std::bind(is_temporary_connection_line,
                                            _1),
                                  std::bind(std::logical_not<bool> {},
                                            std::bind(both_ends_in_list,
                                                    _1)));

    items.erase(std::remove_if(items.begin(), items.end(), shouldRemove), items.end());

    return items;
}

void ItemScene::cut()
{
    QList<QGraphicsItem*> selItems = selectedItems();
    QList<QGraphicsItem*> copyableItems = ItemScene::copyableItems(selItems);

    if (copyableItems.isEmpty()) {
        return;
    }

    copy(copyableItems);
    deleteItems(selItems);
}

void ItemScene::copy() const
{
    copy(copyableItems());
}

void ItemScene::copy(QList<QGraphicsItem*> itms) const
{
    if (itms.isEmpty()) {
        return;
    }

    //Create an xml document and save the selected items to it (using their save method)
    QDomDocument doc(CopyPasteDocType);
    QDomElement root = doc.createElement(CopyPasteRootTag);
    saveToXml(doc, root, constList(itms));
    doc.appendChild(root);

    //Create mime data and push it to clipboard
    QMimeData* md = new QMimeData();
    md->setData(CopyPasteMimeType, doc.toByteArray());
    QApplication::clipboard()->setMimeData(md);
}

QPixmap ItemScene::createPixmap(QDomDocument const& itemsDocument)
{
    auto items = readItems(itemsDocument);
    auto bounding = calculateBoundingBox(constList(items));

    return createPixmap(items, bounding);
}

QPixmap ItemScene::createPixmap(QList<QGraphicsItem*> items, QRectF const& bounding) const
{
    QPixmap pixmap{bounding.size().toSize()};
    pixmap.fill(Qt::lightGray);

    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-bounding.topLeft());

    QStyleOptionGraphicsItem opt;

    for (auto item : items) {
        p.save();
        p.translate(item->pos());

        item->setSelected(false);
        item->paint(&p, &opt);

        for (auto child : item->childItems()) {
            p.save();
            p.translate(child->pos());
            child->paint(&p, &opt);
            p.restore();
        }

        item->setSelected(true);
        p.restore();
    }

    return pixmap;
}

void ItemScene::createTemplate()
{
    QDomDocument document{CopyPasteDocType};
    QDomElement rootElement = document.createElement(CopyPasteRootTag);
    auto items = copyableItems();
    saveToXml(document, rootElement, constList(items));
    document.appendChild(rootElement);

    QRectF const bounding = calculateBoundingBox(constList(items));
    QPixmap pixmap = createPixmap(items, bounding);

    Item_Manager::instance()->itemTemplatesWidget()->createTemplate(document, pixmap);
}

bool ItemScene::notesInInsertMode()
{
    for (QGraphicsItem* itm : items()) {
        if (itm->type() == QGraphicsLineItem::Type) {
            continue;
        }

        ItemNote* itn = qobject_cast<ItemNote*>(itm->toGraphicsObject());

        if (itn != nullptr && itn->inEditMode()) {
            return true;
        }
    }

    return false;
}

void ItemScene::deleteItems(QList<QGraphicsItem*> items)
{
    //remove the connectors first!!
    for (int i = items.count() - 1; i >= 0; i--) {
        Item_Connector* con = qobject_cast<Item_Connector*>(items.at(i)->toGraphicsObject());

        if (con != nullptr) {
            con->remove_and_delete();
            items.removeAt(i);
        }
    }

    //Remove notes and AbstractItems
    for (int i = items.count() - 1; i >= 0; i--) {
        QGraphicsObject* item = items.at(i)->toGraphicsObject();
        AbstractItem* it = qobject_cast<AbstractItem*>(item);

        if (it != nullptr) {
            it->remove();
            delete it;
        } else {
            ItemNote* itn = qobject_cast<ItemNote*>(item);

            if (itn != nullptr) {
                itn->remove();
                delete it;
            }
        }
    }
}

void ItemScene::keyPressEvent(QKeyEvent* event)
{
    if (_inConnectionMode || notesInInsertMode()) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        deleteItems(selectedItems());
    } else if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        copy();
    } else if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier) {
        paste();
    } else if (event->key() == Qt::Key_X && event->modifiers() == Qt::ControlModifier) {
        cut();
    }

    QGraphicsScene::keyPressEvent(event);
}

void ItemScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (itemAt(event->scenePos(), QTransform())) { //there is an item underneath
        QGraphicsScene::contextMenuEvent(event); //redirect the event call to update selected items etc
        return;
    }

    event->accept();
    QMenu menu;

    QAction* actionNote  = menu.addAction(tr("Add Note"));
    QAction* actionPaste  = menu.addAction(tr("Paste Items"));
    actionPaste->setEnabled(QApplication::clipboard()->mimeData()->hasFormat(CopyPasteMimeType));

    QAction* actionSel = menu.exec(event->screenPos());

    if (actionSel == actionNote) {
        ItemNote* note = new ItemNote();
        QPointF posItem = event->scenePos();
        QRectF rectNote = note->boundingRect();
        posItem -= QPointF(rectNote.left() + rectNote.width() / 2, rectNote.top() + rectNote.height() / 2);

        QPointF posItemNew = QPointF(RASTER * round(posItem.x() / RASTER), RASTER * round(posItem.y() / RASTER)); // raster again
        note->setPos(posItemNew);
        connect(note, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
        addItem(note);
        emit sceneRealChanged();
    } else if (actionSel == actionPaste) {
        paste();
    }
}

bool ItemScene::startMove(QGraphicsSceneMouseEvent* event)
{
    //Get the difference between the mouse and the first item and save that (for raster)
    QPointF mousePosition = event->scenePos();
    QList<QGraphicsItem*> items = selectedItems();

    for (int i = 0; i < items.count(); i++) {
        QGraphicsItem* item = qobject_cast<AbstractItem*>(items.at(i)->toGraphicsObject());

        if (item == nullptr) { // if this item is not a GraphicItem
            continue;
        }

        //We found at least one Item. So we need to raster all items
        QPointF itemPosition = item->scenePos();
        _mouseItemDiff = mousePosition - itemPosition;
        _inMovingMode = true;
        //git_first->setZValue(++i_zvalue);  // increase zvalue. it's not that simple.

        break;
    }

    return _inMovingMode;
}

bool ItemScene::startConnection(QGraphicsSceneMouseEvent* mouseEvent)
{
    qreal smallestDistance = AUTOCONNECT_DISTANCE;
    QRectF maskRect(mouseEvent->scenePos() - QPointF(AUTOCONNECT_DISTANCE, AUTOCONNECT_DISTANCE),
                    QSizeF(2 * AUTOCONNECT_DISTANCE, 2 * AUTOCONNECT_DISTANCE));
    auto nearbyItems = items(maskRect, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder);

    // Check if one of the target is an input or output.
    // If so, set it as the start of the connection we're building
    for (auto item : nearbyItems) {
        auto input = qobject_cast<ItemInput*>(item->toGraphicsObject());
        auto output = qobject_cast<ItemOutput*>(item->toGraphicsObject());

        if (input != nullptr) {
            if (input->isConnected()) { // An input can only be connected to one output
                continue;
            }

            qreal d = distance(mouseEvent->scenePos(), input->scenePosition());

            if (d < smallestDistance) {
                smallestDistance = d;
                _inConnectionMode = true;
                _reversed = false;
                _input = input;
                _output = nullptr;
                _type = input->transportType();
                _startPoint = input->scenePosition();
            }
        } else if (output != nullptr) {
            qreal d = distance(mouseEvent->scenePos(), output->scenePosition());

            if (d < smallestDistance) {
                smallestDistance = d;
                _inConnectionMode = true;
                _reversed = true;
                _output = output;
                _input = nullptr;
                _type = output->transportType();
                _startPoint = output->scenePosition();
            }
        }
    }

    return _inConnectionMode;
}

void ItemScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    // Pass right click to Qt
    if (mouseEvent->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(mouseEvent);
        return;
    }

    // Left mouse down starts to move an item
    if (itemAt(mouseEvent->scenePos(), QTransform())) { //there is an item underneath
        QGraphicsScene::mousePressEvent(mouseEvent); //redirect the event call to update selected items etc

        if (startMove(mouseEvent)) {
            return;
        }
    }

    // If the target is an IO connector, start connection mode
    if (startConnection(mouseEvent)) {
        _connectionLine = new QGraphicsLineItem();
        _connectionLine->setPen(AbstractItem::connectorStyle(_type));
        _endPoint = mouseEvent->scenePos();
        updateConnectionLine();
        addItem(_connectionLine);
        mouseEvent->accept();
    }
}

void ItemScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    if (_inConnectionMode) {
        _endPoint = mouseEvent->scenePos();
        updateConnectionLine();
    } else { //moving element(s)
        if (_inMovingMode) {
            //Raster stuff (see also DropEvent and MousePressEvent)
            QPointF posMouse = mouseEvent->scenePos();
            QPointF posIem = posMouse - _mouseItemDiff; //Calc the pos where the item would be (if not already rastered)
            QPointF posItemNew = QPointF(RASTER * round(posIem.x() / RASTER), RASTER * round(posIem.y() / RASTER)); // raster again
            QPointF posMouseNew = posItemNew + _mouseItemDiff;
            mouseEvent->setScenePos(posMouseNew);
        }

        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void ItemScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    if (_inConnectionMode) {
        _inConnectionMode = false;

        if (items().contains(_connectionLine)) {
            removeItem(_connectionLine);
            delete _connectionLine;
            _connectionLine = nullptr;

            if (_input != nullptr && _output != nullptr) {
                Item_Connector* connector = new Item_Connector(_output, _input); //graphical connection
                connector->do_update();
                addItem(connector);
                connect(connector, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
                _input->connectOutput(_output); //data connection
                emit sceneRealChanged();
            }
        }
    } else {
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        _inMovingMode = false;

        if (mouseEvent->button() == Qt::LeftButton) {
            updateBoundingRect(); //Recalc the scene bounding
        }
    }
}


void ItemScene::updateBoundingRect()
{
    QRectF rec_current_size = itemsBoundingRect(); //bounding of all items (expensiv call!)
    qreal x = rec_current_size.x();
    qreal y = rec_current_size.y();
    qreal w = rec_current_size.width();
    qreal h = rec_current_size.height();
    QSizeF view_size = views().at(0)->size() - QSize(20, 20); //estimated displayed size of the scene

    if (_recalculateBoundingRect) { //First time executing this funciton or view resized
        _recalculateBoundingRect = false;
        QRectF rec_last_scene_rect  = sceneRect(); //the initial scene rect
        //Calc a new scene rect, with the old one as center
        qreal x = rec_last_scene_rect.x() + rec_last_scene_rect.width() / 2 - view_size.width() / 2;
        qreal y = rec_last_scene_rect.y() + rec_last_scene_rect.height() / 2 - view_size.height() / 2;
        _sceneRect = QRectF(QPointF(x, y), view_size);
    }

    if (w < view_size.width()) { //bounding of items is smaller than viewsize
        if (x > _sceneRect.left() &&  rec_current_size.right() < (_sceneRect.right())) {
            //bounding width is still inside the old rect
            x = _sceneRect.x(); //take old x pos
        } else {
            x = (x + w / 2) - view_size.width() / 2; //calc new x pos (center items inside of view)
        }

        w = view_size.width();
    }

    if (h < view_size.height()) { //same as above only for the other dimension
        if (y > _sceneRect.top() && rec_current_size.bottom() < (_sceneRect.bottom())) {
            y = _sceneRect.y();
        } else {
            y = (y + h / 2) - view_size.height() / 2;
        }

        h = view_size.height();
    }

    _sceneRect = QRectF(x, y, w, h);
    setSceneRect(_sceneRect);
}

void ItemScene::resetBounding()
{
    _recalculateBoundingRect = true;
    updateBoundingRect();
}

void ItemScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if ((event->mimeData()->hasFormat(DragDropMimeType) || event->mimeData()->hasFormat(CopyPasteMimeType)) && event->proposedAction() == Qt::CopyAction) {
        event->acceptProposedAction();
        event->accept();
        return;
    }

    event->ignore();
}

void ItemScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if ((event->mimeData()->hasFormat(DragDropMimeType) || event->mimeData()->hasFormat(CopyPasteMimeType)) && event->proposedAction() == Qt::CopyAction) {
        event->acceptProposedAction();
        event->accept();
        return;
    }

    event->ignore();
}

void ItemScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat(DragDropMimeType) && event->proposedAction() == Qt::CopyAction) {
        event->acceptProposedAction();
        event->accept();
        QByteArray itemData = event->mimeData()->data(DragDropMimeType);
        QString itemClassName = QString(itemData);

        insertItem(itemClassName, event->scenePos());

    } else if (event->mimeData()->hasFormat(CopyPasteMimeType) && event->proposedAction() == Qt::CopyAction) {
        event->acceptProposedAction();
        event->accept();

        insertTemplate(event->mimeData(), event->scenePos());

    } else {
        event->ignore();
    }
}

void ItemScene::insertItem(QString const& name, QPointF const& position)
{
    AbstractItem* newItem = PluginManager::instance()->createInstance<AbstractItem>(name);

    if (newItem != nullptr) {
        addItem(newItem);
        //Raster Stuff (See also MousePressEvent and MouseMoveEvent:
        QPointF posItemNew = QPointF(RASTER * round(position.x() / RASTER), RASTER * round(position.y() / RASTER)); // raster again
        newItem->setPos(posItemNew);
        connect(newItem, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
        emit sceneRealChanged();
    } else {
        qDebug() << QString("Couldn't create an instance of %1 (Item_Scene)").arg(name);
    }
}

qreal ItemScene::distance(QPointF const& a, QPointF const& b) const
{
    QPointF distanceVector = (a - b);
    return sqrt(pow(distanceVector.x(), 2) + pow(distanceVector.y(), 2));
}

QSharedPointer<ProjectGui> ItemScene::projectGui() const
{
    return _projectGui;
}

bool ItemScene::loadFromXml(QDomElement& dom)
{
    QList<QGraphicsItem*> newItems;

    if (loadFromXml(dom, &newItems, true)) {
        foreach (QGraphicsItem* itm, newItems) {
            addItem(itm);
        }

        updateBoundingRect();
        return true;
    } else {
        return false;
    }
}

bool ItemScene::loadFromXml(QDomElement& dom, QList<QGraphicsItem*>* itemsOut, bool reportProgress)
{
    if (itemsOut == nullptr) {
        return false;
    }

    int progress = 0;
    int childs = 0;
    int actchild = 0;

    QDomNode n = dom.firstChild();

    if (n.isNull()) {
        return true;
    }

    //count all GraphicsItem
    while (!n.isNull()) {
        if (n.toElement().tagName() == GraphicsItemTag || n.toElement().tagName() == GraphicsItemNoteTag) {
            childs++;
        }

        n = n.nextSibling();
    }

    if (reportProgress) {
        emit loadingProgress(progress);
    }

    n = dom.firstChild();

    QHash<qint32, AbstractItem*> idMap;

    while (!n.isNull()) {
        progress = actchild * 100 / childs;

        QDomElement e = n.toElement();

        if (!e.isNull()) {
            if (e.tagName() == GraphicsItemTag) {
                QString const type = e.attribute(TypeAttrTag);

                if (reportProgress)  {
                    emit loadingProgress(progress, "Loading " + type); //show the exact name of what is currently loading
                }

                actchild++;
                AbstractItem* newItem = PluginManager::instance()->createInstance<AbstractItem>(type);

                if (newItem == nullptr) {
                    qDebug() << "Couldn't cast Item to" << type;
                    qDebug() << "Wrong version of Plugin?";
                    return false;
                }

                if (e.hasChildNodes()) {
                    QDomElement se  = e.firstChild().toElement();

                    if (se.isNull()) {
                        qDebug() << "Item has non element subtype";
                        return false;
                    }

                    if (se.tagName() == GraphicsItemDataTag) {
                        if (!newItem->load(se)) {
                            qWarning() << type << "Couldn't load it's data. (Wrong version?)";
                        }
                    } else {
                        qDebug() << "Unknown subelementtype for Item";
                        return false;
                    }
                }

                itemsOut->append(newItem);
                QString name = e.attribute(NameAttrTag);
                qreal x = e.attribute(XPosAttrTag).toDouble();
                qreal y = e.attribute(YPosAttrTag).toDouble();
                qint32 id = e.attribute(IdAttrTag).toUInt();
                newItem->setPos(x, y);
                newItem->setName(name);
                connect(newItem, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
                idMap.insert(id, newItem);
            } else if (e.tagName() == GraphicsItemConnectorTag) {
                if (reportProgress) {
                    emit loadingProgress(progress, "Loading connections");
                }

                qint32 fromObj = e.attribute(FromItemAttrTag).toInt();
                qint32 fromInd = e.attribute(FromIndexAttrTag).toInt();
                qint32 toObj = e.attribute(ToItemAttrTag).toInt();
                qint32 toInd = e.attribute(ToIndexAttrTag).toInt();
                QString typeName = e.attribute(TransportTypeAttrTag);
                qint32 typeId = QMetaType::type(typeName.toStdString().c_str());

                if (!idMap.contains(fromObj) || !idMap.contains(toObj)) {
                    qCritical() << "Couldn't find begin and end object of connector. Connector deleted.";
                } else if (typeId == QMetaType::UnknownType) {
                    qCritical() << "Unknown transport type \"" << typeName << "\"! Connector deleted.";
                } else {
                    AbstractItem* fromItem = idMap[fromObj];
                    AbstractItem* toItem = idMap[toObj];
                    const QList<ItemOutput*> outputs = fromItem->outputs();
                    const QList<ItemInput*> inputs = toItem->inputs();

                    if (fromInd < 0 || fromInd > outputs.count() - 1) {
                        qCritical() << QString("Couldn't find ouput for connector on item %1. Connector deleted.").arg(fromItem->name());
                    } else  if (toInd < 0 || toInd > inputs.count() - 1) {
                        qCritical() << QString("Couldn't find input for connector on item %1. Connector deleted.").arg(toItem->name());
                    } else {
                        ItemInput* input = inputs.at(toInd);
                        ItemOutput* output = outputs.at(fromInd);

                        if (input->transportType() != typeId) {
                            qCritical() << QString("Connector Type is %1. Input of Item %2 is of Type %3. Connector deleted.").arg(typeId).arg(toItem->name()).arg(input->transportType());
                        } else if (output->transportType() != typeId) {
                            qCritical() << QString("Connector Type is %1. Output of Item %2 is of Type %3. Connector deleted.").arg(typeId).arg(fromItem->name()).arg(output->transportType());
                        } else {
                            input->connectOutput(output); //data connection
                            Item_Connector* connector = new Item_Connector(output, input); //graphical connection
                            connector->load_additional(e); //load user modified path or custom routing
                            connector->do_update();
                            itemsOut->append(connector);
                            connect(connector, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
                        }
                    }
                }
            } else if (e.tagName() == GraphicsItemNoteTag) {
                if (reportProgress) {
                    emit loadingProgress(progress, "Loading notes");
                }

                actchild++;

                qreal x = e.attribute(XPosAttrTag).toDouble();
                qreal y = e.attribute(YPosAttrTag).toDouble();
                ItemNote* note = new ItemNote();
                note->setPos(x, y);

                if (e.hasChildNodes()) {
                    QDomElement se  = e.firstChild().toElement();

                    if (se.isNull()) {
                        qDebug() << "ItemNote has non element subtype";
                        return false;
                    }

                    if (se.tagName() == GraphicsItemNoteDataTag) {
                        if (!note->load(se)) {
                            qWarning() << "Note Couldn't load it's data. (Wrong version?)";
                        }
                    } else {
                        qDebug() << "Unknown subelementtype for Item_Note";
                        return false;
                    }
                }

                itemsOut->append(note);
                connect(note, SIGNAL(changed()), this, SIGNAL(sceneRealChanged()));
            }
        }

        n = n.nextSibling();
    }

    if (reportProgress) {
        emit loadingProgress(100);
    }

    return true;
}

bool ItemScene::saveToXml(QDomDocument& document, QDomElement& xml) const
{
    QList<QGraphicsItem*> itms = items();
    return saveToXml(document, xml, constList(itms));
}

bool ItemScene::saveToXml(QDomDocument& document, QDomElement& xml, QList<QGraphicsItem const*> itms) const
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
