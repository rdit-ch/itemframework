#ifndef ITEM_OUTPUT_P_H
#define ITEM_OUTPUT_P_H

#include <QObject>
#include <QList>

class ItemInput;
class ItemOutput;

class ItemOutputPrivate
{
public:
    explicit ItemOutputPrivate(ItemOutput* parent);

    ItemOutput* const q_ptr;
    Q_DECLARE_PUBLIC(ItemOutput)

    void disconnectInput(ItemInput* input);
    void connectInput(ItemInput* input);

    QList<ItemInput*> _inputs;
    QObject* _data = nullptr;
};

#endif // ITEM_OUTPUT_P_H
