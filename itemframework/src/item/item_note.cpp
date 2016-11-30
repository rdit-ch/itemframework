#include "item_note.h"
#include <QPainter>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QGraphicsSceneMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QDebug>
#include <QDomElement>
#include <QGraphicsScene>
#include <QTextCursor>
#include <QColorDialog>
#include "res/resource.h"

#define BOUNDING_SPACE 10
#define TEXT_MARGIN QPointF(5,5)


ItemNote::ItemNote() : QGraphicsObject()
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setAcceptHoverEvents(true);
    _resizeMode = false;
    _editMode = false;
    _color = Resource::get("item_note_bgcolor", QColor(249, 238, 122));

    _textItem = new QGraphicsTextItem("This is a Note.\nDouble-Click to edit.", this);
    _textItem->setDefaultTextColor(Qt::black);
    _textItem->setPos(_shape.topLeft() + TEXT_MARGIN);
    setWidth(180);
    connect(_textItem->document(), SIGNAL(contentsChanged()), this, SLOT(updateSize()));
}

ItemNote::~ItemNote()
{
    scene()->removeItem(this);
    emit changed();
}

QVariant ItemNote::itemChange(GraphicsItemChange change, QVariant const& value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        if (!isSelected()) {
            stopEdit();
        }

    } else if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        emit changed();
    }

    return value;
}


void ItemNote::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    startEdit();
}

void ItemNote::startEdit()
{
    _textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    _textItem->setFocus();
    _editMode = true;
}

void ItemNote::stopEdit()
{
    QTextCursor c(_textItem->textCursor());  //Copy current cursor
    c.clearSelection(); //clear selection
    _textItem->setTextCursor(c); //apply cursor
    _textItem->clearFocus();
    _textItem->setTextInteractionFlags(Qt::NoTextInteraction);
    _editMode = false;
}

void ItemNote::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    if (isSelected()) {
        if ((_shape.right() - event->pos().x()) < 4 &&  _shape.right() != event->pos().x()) {
            setCursor(QCursor(Qt::SizeHorCursor));
        } else {
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }

    QGraphicsItem::hoverMoveEvent(event);
}

void ItemNote::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(QCursor(Qt::ArrowCursor));
    QGraphicsItem::hoverLeaveEvent(event);
}


void ItemNote::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (isSelected() && event->button() == Qt::LeftButton && ((_shape.right() - event->pos().x()) < 4)) {
        _resizeMode = true;
    }

    QGraphicsItem::mousePressEvent(event);
}

void ItemNote::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (_resizeMode) {
        int x = event->pos().x();
        x -= _shape.left() + 2 * TEXT_MARGIN.x() - 1;
        setWidth(x);
        emit changed();
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void ItemNote::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (_resizeMode) {
        _resizeMode = false;
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

qreal ItemNote::width() const
{
    return _textItem->textWidth();

}

void ItemNote::setWidth(qreal r)
{
    if (r >= 40) {
        _textItem->setTextWidth(r);
        updateSize();
    }
}

void ItemNote::updateSize()
{
    prepareGeometryChange();
    _shape.setHeight(_textItem->boundingRect().height() + 2 * TEXT_MARGIN.y());
    _shape.setWidth(_textItem->boundingRect().width() + 2 * TEXT_MARGIN.x());
    _boundingRect = _shape.adjusted(-BOUNDING_SPACE, -BOUNDING_SPACE, BOUNDING_SPACE, BOUNDING_SPACE);
    emit changed();
}

QRectF ItemNote::boundingRect() const
{
    return _boundingRect;
}


QPainterPath ItemNote::shape() const
{
    QPainterPath path;
    path.addRect(_shape);
    return path;
}

bool ItemNote::inEditMode() const
{
    return _editMode;
}

void ItemNote::paint(QPainter* painter, QStyleOptionGraphicsItem const*, QWidget*)
{
    painter->setBrush(_color);
    painter->drawRect(_shape.adjusted(0, 0, -1, -1)); // this is bad practice, but since we can't get mouse events outside the shape in a easy way, we need to do it that way
    //because the rect is painted 1px smaller, we'll catch events which are exatly on the right line too

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Resource::get("item_box_selected_pen", QPen(Qt::black, 1, Qt::DashLine)));
        painter->drawRect(_boundingRect);

    }
}


bool ItemNote::save(QDomDocument&, QDomElement& elm) const
{
    elm.setAttribute("content", _textItem->toPlainText());
    elm.setAttribute("width", width());
    elm.setAttribute("color", QString::number((uint)_color.rgb(), 16));
    return true;
}

bool ItemNote::load(QDomElement& elm)
{
    if (elm.hasAttribute("content")) {
        QString f = elm.attribute("content");
        _textItem->setPlainText(f);
        prepareGeometryChange();
        _shape.setHeight(_textItem->boundingRect().height() + 2 * TEXT_MARGIN.y());
        _boundingRect = _shape.adjusted(-BOUNDING_SPACE, -BOUNDING_SPACE, BOUNDING_SPACE, BOUNDING_SPACE);
    }

    if (elm.hasAttribute("width")) {
        int width = elm.attribute("width").toInt();
        setWidth(width);
    }

    if (elm.hasAttribute("color")) {
        // load color
        int color_rgb = elm.attribute("color").toLong(0, 16);
        _color = QColor((QRgb)color_rgb);
    }

    return true;
}

void ItemNote::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    event->accept();
    QMenu menu;

    QAction* act_edit = menu.addAction("Edit Note");
    QFont font = QFont(act_edit->font());
    font.setWeight(70);
    font.setStyleStrategy(QFont::OpenGLCompatible);
    act_edit->setFont(font);

    QAction* act_select_color = menu.addAction("Select background");
    menu.addSeparator();
    QAction* act_remove = menu.addAction("Remove");
    QAction* sel_action = menu.exec(event->screenPos());

    ungrabMouse(); //Important line! Otherwise the element will be moved after you close the context menu and draw a selection rectangle somewhere.

    if (sel_action == act_remove) {
        delete this;
    } else if (sel_action == act_edit) {
        setSelected(true);
        startEdit();
    } else if (sel_action == act_select_color) {
        selectColor();
    }

}

void ItemNote::selectColor()
{
    // pick a new color
    QColor color_new = QColorDialog::getColor(_color);

    // has color changed?
    if (_color != color_new) {
        _color = color_new;
        emit changed();
    }
}


