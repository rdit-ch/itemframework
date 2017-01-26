#ifndef SOME_ITEM_H
#define SOME_ITEM_H

#include "item/abstract_item.h"
#include <QObject>

class SomeItem : public AbstractItem
{
    Q_OBJECT

    Q_PROPERTY(int     number MEMBER _number USER true)
    Q_PROPERTY(QString name   MEMBER _name   USER true)

public:
    Q_INVOKABLE SomeItem();
    SomeItem(QString const& name, int number);

    QString _name;
    int     _number;

private:
    ItemInput*  _input {nullptr};
    ItemOutput* _output{nullptr};
};

#endif // SOME_ITEM_H
