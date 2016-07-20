#ifndef ITEM_INPUT_P
#define ITEM_INPUT_P

#include <QObject>

class ItemOutput;
class ItemInput;

class ItemInputPrivate
{
public:
    explicit ItemInputPrivate(ItemInput* parent);

    ItemInput* const q_ptr;
    Q_DECLARE_PUBLIC(ItemInput)

    void disconnectOutput();
    void connectOutput(ItemOutput* output);
    void updateData();

    ItemOutput* _output = nullptr;
    QObject* _data = nullptr;
};

#endif // ITEM_INPUT_P

