#ifndef ITEM_INPUT_OUTPUT_BASE_P_H
#define ITEM_INPUT_OUTPUT_BASE_P_H

#include "item/abstract_item.h"

class AbstractItemInputOutputBase;

class AbstractItemInputOutputBasePrivate
{
public:
    explicit AbstractItemInputOutputBasePrivate(AbstractItemInputOutputBase* parent,
            AbstractItem* parentItem, QRectF const& shape,
            int transportType);

    AbstractItemInputOutputBase* const q_ptr;
    Q_DECLARE_PUBLIC(AbstractItemInputOutputBase)

    AbstractItem* _parent;
    QRectF _shape;
    int _transportType;
};


#endif // ITEM_INPUT_OUTPUT_BASE_P_H

