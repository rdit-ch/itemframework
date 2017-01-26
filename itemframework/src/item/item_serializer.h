#ifndef ITEM_SERIALIZER_H
#define ITEM_SERIALIZER_H

#include "appcore.h"

#include "helper/progress_helper.h"

#include <QDomDocument>
#include <QDomElement>
#include <QList>

struct ITEMFRAMEWORK_TEST_EXPORT ItemSerializer
{
    static bool saveToXml(QDomDocument& document,
                          QDomElement& xml,
                          QList<class QGraphicsItem const*> items);

    static bool loadFromXml(QDomElement const& xml,
                            QList<class QGraphicsItem*>* itemsOut,
                            ProgressReporter progressReporter,
                            bool shouldConnectIO = true);
};

#endif // ITEM_SERIALIZER_H
