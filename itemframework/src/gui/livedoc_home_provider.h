#ifndef LIVEDOC_HOME_PROVIDER_H
#define LIVEDOC_HOME_PROVIDER_H
#include "gui/interface_livedoc_provider.h"
#include <QMap>

class Livedoc_Home_Provider : public Interface_Livedoc_Provider
{
public:
    virtual QString provide_livedoc(QUrl doc);
    const QMap<QString, QString>& get_listed_contents();
private:
    static const QMap<QString, QString> map_contents;
};

#endif // LIVEDOC_HOME_PROVIDER_H
