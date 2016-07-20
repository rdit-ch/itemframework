#ifndef LIVEDOC_H
#define LIVEDOC_H

#include <QMutex>
#include <QObject>
#include <QUrl>

#include "appcore.h"
#include "helper/singleton.h"

class Interface_Livedoc_Provider;
class Livedoc_Widget;
class CORE_EXPORT Livedoc : public QObject, public Singleton<Livedoc>
{
    //------------------Singleton Stuff---------------------------
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)

    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "Gui_Manager")
    Q_CLASSINFO("dependsOn", "PluginManager")

public:
    Q_INVOKABLE Livedoc();
    ~Livedoc();

protected:
    bool postInit();
    bool preDestroy();
    //------------------End Singleton Stuff---------------------------

public:
    void add_provider(Interface_Livedoc_Provider* p);
    void remove_provider(Interface_Livedoc_Provider* p);
    const QList<Interface_Livedoc_Provider*>& get_providers() const;
    QString get_doc(QUrl doc);

public slots:
    void navigate(QUrl doc);
    void show();
    void hide();
    void toggle(bool visible);
private slots:
    void widget_closed();

private:
    Livedoc_Widget* browser;
    QList<Interface_Livedoc_Provider*> lis_providers;

};

#endif // LIVEDOC_H
