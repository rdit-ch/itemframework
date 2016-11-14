#include "item/abstract_item.h"
#include "abstract_item_p.h"
#include "item/abstract_item_input_output_base.h"
#include "item/item_scene.h"
#include "item/item_input.h"
#include "item/item_output.h"
#include "project/abstract_project.h"
#include "res/resource.h"
#include "helper/dom_helper.h"
#include "project/project_gui.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QInputDialog>
#include <QMetaProperty>
#include <QDomElement>
#include <QBuffer>
#include <QVariant>

QHash<QString, int> AbstractItemPrivate::_itemTypesCount;

AbstractItem::AbstractItem(QString typeName) : QGraphicsObject(), d_ptr(new AbstractItemPrivate(this))
{
    Q_D(AbstractItem);

    d->_type = typeName;

    // item geometry
    d->_shape = QRectF(-27, -27, 54, 54);
    d->_boundingRect = QRectF(d->_shape.x() - 20, d->_shape.y() - 20, d->_shape.width() + 40, d->_shape.height() + 40);
    d->_image = QImage();

    // type name
    QFont typeFont;
    typeFont.setPixelSize(12);
    typeFont.setWeight(75);
    d->_typeLabel.setFont(typeFont);
    d->_typeLabel.setText(d->_type);
    d->_typeLabel.setPos(-(d->_typeLabel.boundingRect().width() / 2), d->_boundingRect.y());

    // item name
    QString itemTypeName = typeName.toLower();
    int itemNumber = 0;

    if (d->_itemTypesCount.contains(itemTypeName)) {
        itemNumber = d->_itemTypesCount[itemTypeName] + 1;
    }

    QFont nameFont;
    nameFont.setPixelSize(12);
    d->_nameLabel.setFont(nameFont);

    QString name = QString("%1%2").arg(typeName.toLower()).arg(itemNumber);
    setName(name);

    d->_itemTypesCount[itemTypeName] = itemNumber;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    // Create settings scope, name it as item is named
    // Note: the scope has no parent scope yet. The parent scope will be set, when this item is
    // attached to a scene. The item will retrieve the project from the attached scene.
    // see itemChange().
    d->_settingsScope.reset(new SettingsScope(d->_name));
    // Connect for setting scope changes (not settings changes) in order to emit a item
    // changed signal when the scope changes. Mainly to to indicate needed save on change.
    QObject::connect(d->_settingsScope.data(), &SettingsScope::scopeChanged, [this]() {
        emit changed();
    });
}

AbstractItemPrivate::AbstractItemPrivate(AbstractItem* parent) :
    q_ptr(parent), _progress(-1), _autohide_timer(0), _is_autohide_active(false),
    _settingsScope(NULL), _typeLabel(parent), _nameLabel(parent)
{
}

QString AbstractItem::typeName() const
{
    Q_D(const AbstractItem);

    return d->_type;
}

int AbstractItem::progress() const
{
    Q_D(const AbstractItem);

    return d->_progress;
}

void AbstractItem::setProgress(int progr)
{
    Q_D(AbstractItem);

    if (d->_is_autohide_active) {
        killTimer(d->_autohide_timer);
        d->_is_autohide_active = false;
    }

    if (progr < -1) {
        d->_progress = -1;
    } else if (progr > 100) {
        d->_progress = 100;
    } else {
        d->_progress = progr;
    }

    update();

    emit progressChanged();
}

void AbstractItem::setProgressAutohide(int i_ms)
{
    Q_D(AbstractItem);

    if (d->_is_autohide_active) {
        killTimer(d->_autohide_timer);
        d->_is_autohide_active = false;
    }

    if (i_ms > 0) {
        d->_autohide_timer = startTimer(i_ms);
        d->_is_autohide_active = true;
    }
}

void AbstractItem::timerEvent(QTimerEvent* t)
{
    Q_D(AbstractItem);

    if (t->timerId() == d->_autohide_timer) {
        setProgress(-1);
    }
}

QString AbstractItem::name() const
{
    Q_D(const AbstractItem);

    return d->_name;
}

void AbstractItem::setName(QString name)
{
    static qreal const verticalOffset = 5;

    Q_D(AbstractItem);

    if (name.trimmed().length() > 0 && name != d->_name) {
        d->_name = name;
        d->_nameLabel.setText(d->_name);
        d->_nameLabel.setPos(-(d->_nameLabel.boundingRect().width() / 2), d->_shape.y() + d->_shape.height() + verticalOffset);

        // The attached settings scope's name must be in sync with the item name!
        if (d->_settingsScope != nullptr) {
            d->_settingsScope->setName(name);
        }

        update();
        emit nameChanged();
        emit changed();
    }
}

void AbstractItem::setDescription(QString const& description)
{
    Q_D(AbstractItem);

    if (description != d->_description) {
        d->_description = description;
        emit descriptionChanged();
    }
}

QString AbstractItem::description() const
{
    Q_D(const AbstractItem);

    return d->_description;
}

void AbstractItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    Q_D(AbstractItem);

    event->accept();
    QMenu menu;

    contextMenuPrepare(menu);

    if (menu.actions().count() > 0) {
        menu.addSeparator();
    }

    ItemScene* itemScene = qobject_cast<ItemScene*>(scene());
    Q_ASSERT(itemScene != nullptr);

    QAction* actionRename         = menu.addAction(tr("Rename"));
    QAction* actionRemove         = menu.addAction(tr("Remove"));
    QAction* actionCopyItem       = menu.addAction(tr("Copy this item"));
    QAction* actionCopySelection  = menu.addAction(tr("Copy selection"));
    QAction* actionCreateTemplate = menu.addAction(tr("Create template from selection"));
    actionCopySelection->setEnabled(!itemScene->copyableItems().isEmpty()); //only enable action if there are copyable items available

    QAction* actionSel = menu.exec(event->screenPos());
    ungrabMouse(); //Important line! Otherwise the element will be moved after you close the context menu and draw a selection rectangle somewhere.

    if (actionSel == actionRemove) {
        disconnectConnections();
        delete this;
    } else if (actionSel == actionRename) {
        d->showRenameDialog();
    } else if (actionSel == actionCopyItem) {
        itemScene->copy(QList<QGraphicsItem*>() << this); //Only copy this instance of the item
    } else if (actionSel == actionCopySelection) {
        itemScene->copy();
    } else if (actionSel == actionCreateTemplate) {
        itemScene->createTemplate();
    }
}

void AbstractItemPrivate::showRenameDialog()
{
    Q_Q(AbstractItem);

    bool ok;
    QString text = QInputDialog::getText(0, "Rename",
                                         QString("New name for '%1':").arg(q->name()), QLineEdit::Normal,
                                         q->name(), &ok).trimmed();

    if (ok && !text.isEmpty()) {
        q->setName(text);
    }
}

void AbstractItem::contextMenuPrepare(QMenu&) const
{
}

void AbstractItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    ungrabMouse();
}

void AbstractItemPrivate::realignInputs()
{
    realignInputsOutputs(reinterpret_cast<QList<AbstractItemInputOutputBase*>&>(_inputs), inputX());
}

void AbstractItemPrivate::realignOutputs()
{
    realignInputsOutputs(reinterpret_cast<QList<AbstractItemInputOutputBase*>&>(_outputs), outputX());
}

void AbstractItemPrivate::realignInputsOutputs(QList<AbstractItemInputOutputBase*>& elements, float offsetX)
{
    Q_Q(AbstractItem);

    int count = elements.count();
    float spacing = (_shape.height()) / (count + 1);

    for (int i = 0; i < count; i++) {
        float offsetY = (i + 1) * spacing - (_connectorHeight / 2) - (_shape.height() / 2);
        QPointF pos(offsetX, offsetY);
        elements.at(i)->setLocalPosition(pos);
    }

    q->update();
}

float AbstractItemPrivate::inputX(float x) const
{
    return x - (_shape.width() / 2) - (_connectorWidth / 2);
}

float AbstractItemPrivate::inputY(float y) const
{
    return inputOutputY(y);
}

float AbstractItemPrivate::outputX(float x) const
{
    return x + (_shape.width() / 2) + (_connectorWidth / 2);
}

float AbstractItemPrivate::outputY(float y) const
{
    return inputOutputY(y);
}

float AbstractItemPrivate::inputOutputY(float y) const
{
    return y - (_connectorHeight / 2);
}

ItemInput* AbstractItem::addInput(int type, QString const& description)
{
    Q_D(AbstractItem);

    QRectF shape(d->inputX(), d->inputY(), d->_connectorWidth, d->_connectorHeight);
    ItemInput* input = new ItemInput(this, type, description, shape);
    d->_inputs.append(input);

    d->realignInputs();

    return input;
}

ItemOutput* AbstractItem::addOutput(int type, QString const& description)
{
    Q_D(AbstractItem);

    QRectF shape(0, 0, /*d->outputX(), d->outputY(),*/ d->_connectorWidth, d->_connectorHeight);
    ItemOutput* output = new ItemOutput(this, type, description, shape);
    d->_outputs.append(output);

    d->realignOutputs();

    return output;
}

void AbstractItem::setOutputData(ItemOutput* output, QObject* data)
{
    if (output == NULL || output->owner() != this) {
        return;
    }

    output->setData(data);
}

void AbstractItem::clearInputs()
{
    Q_D(AbstractItem);

    for (int i = d->_inputs.count() - 1; i >= 0; i--) {
        ItemInput* input = d->_inputs.at(i);
        input->disconnectOutput();
        d->_inputs.removeAt(i);
        delete input;
    }

    update();
}

void AbstractItem::clearOutputs()
{
    Q_D(AbstractItem);

    for (int i = d->_outputs.count() - 1; i >= 0; i--) {
        ItemOutput* output = d->_outputs.at(i);
        output->disconnectInputs();
        d->_outputs.removeAt(i);
        delete output;
    }

    update();
}

const QList<ItemInput*> AbstractItem::inputs() const
{
    Q_D(const AbstractItem);

    return d->_inputs;
}

const QList<ItemOutput*> AbstractItem::outputs() const
{
    Q_D(const AbstractItem);

    return d->_outputs;
}

QRectF AbstractItem::boundingRect() const
{
    Q_D(const AbstractItem);

    return d->_boundingRect;
}


QPainterPath AbstractItem::shape() const
{
    Q_D(const AbstractItem);

    QPainterPath path;
    path.addRect(d->_shape);

    return path;
}

QVariant AbstractItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    Q_D(AbstractItem);

    switch (change) {
    case QGraphicsItem::ItemScenePositionHasChanged:
        for (auto input : d->_inputs) {
            input->updateScenePosition();
        }

        for (auto output : d->_outputs) {
            output->updateScenePosition();
        }

        emit changed();

        break;

    case QGraphicsItem::ItemSceneHasChanged:

        // If the scene changes, the related project might have changed. The attached settings
        // scope needs a new parent in this case!
        if (d->_settingsScope != nullptr) {
            // Determine the project settings scope.
            SettingsScope* projectScope = nullptr;
            // The related project can be determineprojectd via scene, get new scene from variant
            ItemScene* itemScene = qobject_cast<ItemScene*>(qvariant_cast<QObject*>(value));

            // If we have a valid scene and a attached project, get the related settings scope
            if (itemScene != nullptr && !itemScene->projectGui().isNull()) {
                projectScope = itemScene->projectGui()->project()->settingsScope();
            }

            // Update this item settings scope's parent
            d->_settingsScope->setParentScope(projectScope);
        }

        break;

    default:
        break;
    }

    return value;
}

bool AbstractItem::registerConnectorStyle(QColor const& color, int type)
{
    return Resource::set<QPen>(QString("item_connector_pen%1").arg(type), QPen(color, connectorHeight()));
}

QPen AbstractItem::connectorStyle(int type)
{
    return Resource::get(QString("item_connector_pen%1").arg(type), QPen(Qt::black, connectorHeight()));
}

void AbstractItem::disconnectConnections() {
    Q_D(AbstractItem);
    for (int i = d->_inputs.count() - 1; i >= 0; i--) {
        ItemInput* input = d->_inputs.at(i);
        input->disconnectOutput();
    }

    for (int i = d->_outputs.count() - 1; i >= 0; i--) {
        ItemOutput* output = d->_outputs.at(i);
        output->disconnectInputs();
    }
}

AbstractItem::~AbstractItem()
{
    Q_D(AbstractItem);

    bool changes = false;

    for (int i = d->_inputs.count() - 1; i >= 0; i--) {
        ItemInput* input = d->_inputs.at(i);
        disconnect(input,0,this,0);
        input->disconnectOutput();
        d->_inputs.removeAt(i);
        delete input;
        changes = true;
    }

    for (int i = d->_outputs.count() - 1; i >= 0; i--) {
        ItemOutput* output = d->_outputs.at(i);
        disconnect(output,0,this,0);
        output->disconnectInputs();
        d->_outputs.removeAt(i);
        delete output;
        changes = true;
    }

    if (scene() != NULL) {
        scene()->removeItem(this);
        changes = true;
    }

    if (changes) {
        emit changed();
    }
}

const QImage AbstractItem::image() const
{
    Q_D(const AbstractItem);

    return d->_image;
}

void AbstractItem::setImage(QImage const& image)
{
    Q_D(AbstractItem);

    d->_image = image;

    update();
}

void AbstractItem::paintItemBox(QPainter* painter)
{
    Q_D(AbstractItem);

    // Draw box
    QColor bgcolor = Resource::get("item_box_bgcolor", QColor(0xF5, 0xF5, 0xF5));
    QColor bcolor = Resource::get("item_box_bordercolor", QColor(Qt::gray));
    painter->fillRect(d->_shape, bgcolor);
    painter->setPen(bcolor);
    painter->drawRect(d->_shape);

    //Draw Icon
    QRectF rec_image;

    if (d->_progress == -1) {
        rec_image = QRectF(d->_shape.x() + 5, d->_shape.y() + 5, d->_shape.width() - 10, d->_shape.height() - 10);
    } else {
        rec_image = QRectF(d->_shape.x() + 10, d->_shape.y() + 5, d->_shape.width() - 20, d->_shape.height() - 20);
    }

    painter->drawImage(rec_image, d->_image);

    if (d->_progress != -1) {
        QRectF rec_pb(d->_shape.x() + 5, d->_shape.bottom() - 10 , d->_shape.width() - 10, 5);
        painter->drawRect(rec_pb);
        rec_pb.adjust(1, 1, 0, 0);
        rec_pb.setWidth(rec_pb.width() * (d->_progress) / 100);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Resource::get("item_box_progresscolor", QColor(Qt::green)));
        painter->drawRect(rec_pb);
    }
}

void AbstractItem::paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget* widget)
{
    Q_D(AbstractItem);

    // paint inputs and output
    for (ItemInput* e : d->_inputs) {
        e->paint(painter, option, widget);
    }

    for (ItemOutput* e : d->_outputs) {
        e->paint(painter, option, widget);
    }

    //Paint the item with background and content
    paintItemBox(painter);

    // If plugin is selected draw surrounding box
    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Resource::get("item_box_selected_pen", QPen(Qt::black, 1, Qt::DashLine)));
        painter->drawRect(d->_boundingRect);
    }
}

bool AbstractItem::load(QDomElement& element)
{
    Q_D(AbstractItem);

    bool success = true;

    // Load settings scope first
    if (d->_settingsScope != nullptr) {
        if (!d->_settingsScope->load(element)) {
            qDebug("Failed to load item settings scope!");
            success = false;
        }
    }

    // Load all properties that are marked as USER properties from this item
    if (!DomHelper::loadUserProperties(this, element)) {
        qDebug("Failed to load all properties!");
        success = false;
    }

    return success;
}

bool AbstractItem::save(QDomDocument& doc, QDomElement& element) const
{
    Q_D(const AbstractItem);

    bool success = true;

    // First persist the attached settings scope
    if (d->_settingsScope != nullptr) {
        if (!d->_settingsScope->save(doc, element)) {
            qDebug("Failed to save item settings scope!");
            success = false;
        }
    }

    // Next persist the user properties
    if (!DomHelper::saveUserProperties(this, element, doc)) {
        qDebug("Failed to save all properties!");
        success = false;
    }

    return success;
}

SettingsScope* AbstractItem::settingsScope() const
{
    Q_D(const AbstractItem);
    return d->_settingsScope.data();
}

int AbstractItem::connectorHeight()
{
    return AbstractItemPrivate::_connectorHeight;
}

int AbstractItem::connectorWidth()
{
    return AbstractItemPrivate::_connectorWidth;
}

