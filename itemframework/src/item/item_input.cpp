#include "item/item_input.h"
#include "item_input_p.h"
#include "item/item_output.h"
#include "item_output_p.h"
#include "item/abstract_item.h"

ItemInput::ItemInput(AbstractItem* owner, int type, QString const& description, QRectF const& shape)
    : AbstractItemInputOutputBase(owner, type, description, shape), d_ptr(new ItemInputPrivate(this))
{
}

ItemInput::~ItemInput()
{
}

ItemInputPrivate::ItemInputPrivate(ItemInput* parent) :
    q_ptr(parent)
{
}

bool ItemInput::isConnected() const
{
    Q_D(const ItemInput);

    return (d->_output != NULL);
}

ItemOutput* ItemInput::output() const
{
    Q_D(const ItemInput);

    return d->_output;
}

void ItemInput::disconnectOutput()
{
    Q_D(ItemInput);

    if (isConnected()) {
        d->_output->d_ptr->disconnectInput(this);
        d->disconnectOutput();
    }
}

void ItemInputPrivate::disconnectOutput()
{
    Q_Q(ItemInput);

    _output = NULL;

    emit q->outputDisconnected();

    updateData();
}

bool ItemInput::connectOutput(ItemOutput* output)
{
    Q_D(ItemInput);

    if (isConnected() || output == NULL) {
        return false;
    }

    if (transportType() != output->transportType()) {
        return false;
    }

    if (output->d_ptr->_inputs.contains(this)) {
        return false;
    }

    output->d_ptr->connectInput(this);
    d->connectOutput(output);

    return true;
}

void ItemInputPrivate::connectOutput(ItemOutput* output)
{
    Q_Q(ItemInput);

    _output = output;

    emit q->outputConnected();

    updateData();
}

QObject* ItemInput::data() const
{
    Q_D(const ItemInput);

    if (isConnected()) {
        return d->_output->data();
    }

    return NULL;
}

void ItemInputPrivate::updateData()
{
    Q_Q(ItemInput);

    auto newData = q->data();

    if (newData != _data) {
        _data = newData;

        emit q->dataChanged();

        // redraw connector as internal state has changed
        q->update();
    }
}

QPointF ItemInput::scenePosition() const
{
    auto pos = AbstractItemInputOutputBase::scenePosition();
    pos.setX(pos.x() - (boundingRect().width() / 2));

    return pos;
}
