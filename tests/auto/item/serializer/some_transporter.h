#ifndef SOME_TRANSPORTER_H
#define SOME_TRANSPORTER_H

#include "appcore.h"
#include <QObject>

class SomeTransporter : public QObject
{
    Q_OBJECT
};

Q_DECLARE_METATYPE(SomeTransporter*)

#endif // SOME_TRANSPORTER_H
