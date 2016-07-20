#ifndef INTERFACE_LIVEDOC_PROVIDER_H
#define INTERFACE_LIVEDOC_PROVIDER_H

#include "appcore.h"

#include <QUrl>
#include <QString>
#include <QObject>
#include <QMap>

class CORE_EXPORT Interface_Livedoc_Provider : public QObject
{
    Q_GADGET
public:
    /**
     * @brief provide_livedoc returns the html content of the requested document or a null-string when no such document is available
     * @param doc
     * @return
     */
    virtual QString provide_livedoc(QUrl doc) = 0;
    /**
     * @brief get_listed_contents returns a map of contents which should be listed in a Index (e.g. Table of Contents)
     * @return a map with the Title as Key and the Links as value of the listed contents
     */
    virtual const QMap<QString, QString>& get_listed_contents() = 0;
};

Q_DECLARE_INTERFACE(Interface_Livedoc_Provider, "Itemframework.Interface.Livedoc_Provider/1.0")


#endif // INTERFACE_LIVEDOC_PROVIDER_H
