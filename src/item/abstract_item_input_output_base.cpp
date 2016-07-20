#include "item/abstract_item_input_output_base.h"
#include "item/abstract_item_input_output_base_p.h"
#include "item/abstract_item.h"
#include "res/resource.h"
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QDebug>

AbstractItemInputOutputBase::AbstractItemInputOutputBase(AbstractItem* parent, int type,
        QString const& description,
        QRectF const& shape) :
    QGraphicsObject(parent), d_ptr(new AbstractItemInputOutputBasePrivate(this, parent,
                                   shape, type))
{
    setToolTip(description);
}

AbstractItemInputOutputBase::~AbstractItemInputOutputBase()
{
}

AbstractItemInputOutputBasePrivate::AbstractItemInputOutputBasePrivate(AbstractItemInputOutputBase* parent,
        AbstractItem* parentItem,
        QRectF const& shape,
        int type) :
    q_ptr(parent), _parent(parentItem), _shape(shape), _transportType(type)
{
}

int AbstractItemInputOutputBase::transportType() const
{
    Q_D(const AbstractItemInputOutputBase);

    return d->_transportType;
}

AbstractItem* AbstractItemInputOutputBase::owner() const
{
    Q_D(const AbstractItemInputOutputBase);

    return d->_parent;
}

QPointF AbstractItemInputOutputBase::localPosition() const
{
    Q_D(const AbstractItemInputOutputBase);

    return d->_shape.center();
}

QPointF AbstractItemInputOutputBase::scenePosition() const
{
    Q_D(const AbstractItemInputOutputBase);

    return d->_parent->mapToScene(localPosition());
}

void AbstractItemInputOutputBase::setLocalPosition(QPointF const& position)
{
    Q_D(AbstractItemInputOutputBase);

    d->_shape.moveCenter(position);

    emit positionChanged();
}

void AbstractItemInputOutputBase::updateScenePosition()
{
    emit positionChanged();
}

QRectF AbstractItemInputOutputBase::boundingRect() const
{
    Q_D(const AbstractItemInputOutputBase);

    QRectF r(d->_shape.x(), d->_shape.y(), d->_shape.width(), d->_shape.height());

    return r;
}

void AbstractItemInputOutputBase::paint(QPainter* painter, QStyleOptionGraphicsItem const*, QWidget*)
{
    Q_D(AbstractItemInputOutputBase);

    // fill shape
    painter->fillRect(d->_shape, AbstractItem::connectorStyle(transportType()).color());

    // overlay pattern
    if (data() == nullptr) {
        painter->fillRect(d->_shape, QBrush{Qt::white, Qt::BDiagPattern});
    }
}
