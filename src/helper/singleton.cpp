#include "helper/singleton.h"
#include <QHash>

static QHash<QString, AbstractSingleton* > instances;

AbstractSingleton* SingletonStorage::getInstance(const char* name)
{
    return instances[name];
}

void SingletonStorage::storeInstance(const char* name, AbstractSingleton* inst)
{
    if (inst != nullptr && instances.contains(name)) {
        qCritical() << "Singleton" << name << "was already created. Ignoring new instance";
    } else {
        instances[name] = inst;
    }
}
