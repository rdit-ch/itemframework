#include "some_item.h"
#include "some_transporter.h"

SomeItem::SomeItem()
    : SomeItem("", 0)
{}

SomeItem::SomeItem(QString const& name, int number)
    : AbstractItem("SomeItem"),
      _name{name}, _number{number}
{
    _input  = addInput( qMetaTypeId<SomeTransporter*>(), "input transporter");
    _output = addOutput(qMetaTypeId<SomeTransporter*>(), "output transporter");
}
