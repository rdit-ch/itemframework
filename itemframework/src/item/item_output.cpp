#include "item/item_output.h"
#include "item_output_p.h"
#include "item/item_input.h"
#include "item_input_p.h"
#include "item/abstract_item.h"

ItemOutput::ItemOutput(AbstractItem* owner, int type, QString const& description, QRectF const& shape)
    : AbstractItemInputOutputBase(owner, type, description, shape), d_ptr(new ItemOutputPrivate(this))
{
}

ItemOutput::~ItemOutput()
{

}

ItemOutputPrivate::ItemOutputPrivate(ItemOutput* parent) :
    q_ptr(parent)
{
}



bool ItemOutput::isConnected() const
{
    Q_D(const ItemOutput);

    return (d->_inputs.count() > 0);
}

QList<ItemInput*> ItemOutput::inputs() const
{
    Q_D(const ItemOutput);

    return d->_inputs;
}

QObject* ItemOutput::data() const
{
    Q_D(const ItemOutput);

    return d->_data;
}

void ItemOutput::disconnectInput(ItemInput* input)
{
    Q_D(ItemOutput);

    if (d->_inputs.contains(input)) {
        input->d_ptr->disconnectOutput();
        d->disconnectInput(input);
    }
}

void ItemOutput::disconnectInputs()
{
    Q_D(ItemOutput);

    for (auto input : d->_inputs) {
        input->d_ptr->disconnectOutput();
        d->disconnectInput(input);
    }
}

void ItemOutputPrivate::disconnectInput(ItemInput* input)
{
    Q_Q(ItemOutput);

    _inputs.removeOne(input);
    emit q->inputDisconnected();
}

bool ItemOutput::connectInput(ItemInput* input)
{
    Q_D(ItemOutput);

    if (input == NULL) {
        return false;
    }

    if (transportType() != input->transportType()) {
        return false;
    }

    if (input->isConnected()) {
        return false;
    }

    if (d->_inputs.contains(input)) {
        return false;
    }

    input->d_ptr->connectOutput(this);
    d->connectInput(input);

    return true;
}

void ItemOutputPrivate::connectInput(ItemInput* input)
{
    Q_Q(ItemOutput);

    _inputs.append(input);

    emit q->inputConnected();
}

void ItemOutput::setData(QObject* data)
{
    Q_D(ItemOutput);

    bool hasChanged = (d->_data != data);
    d->_data = data;

    if (hasChanged) {
        emit dataChanged();

        // redraw connector as internal state has changed
        update();
    }

    for (auto input : d->_inputs) {
        input->d_ptr->updateData();
    }
}

QPointF ItemOutput::scenePosition() const
{
    auto pos = AbstractItemInputOutputBase::scenePosition();
    pos.setX(pos.x() + (boundingRect().width() / 2));

    return pos;
}
