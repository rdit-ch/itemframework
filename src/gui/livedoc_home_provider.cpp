#include "livedoc_home_provider.h"
#include "gui/livedoc.h"
#include <QFile>
#include <QTextStream>


const QMap<QString, QString> Livedoc_Home_Provider::map_contents = {
    {"Projects and Items", "help:items" }

};

QString Livedoc_Home_Provider::provide_livedoc(QUrl doc)
{
    if (doc.scheme() != "help") {
        return QString();
    }

    if (doc.path() == "home") {
        QString html = "<html><body>";
        html += "<h1>Home</h1>";
        html += "<h2>Table of Contents</h2>";
        html += "<ul>";
        const QList<Interface_Livedoc_Provider*>&  providers = Livedoc::instance()->get_providers();
        QListIterator<Interface_Livedoc_Provider*> it(providers);

        while (it.hasNext()) {
            const QMap<QString, QString>& contents = it.next()->get_listed_contents();
            QMapIterator<QString, QString> it2(contents);

            while (it2.hasNext()) {
                it2.next();
                html += QString("<li><a href=\"%1\">%2</a></li>").arg(it2.value()).arg(it2.key());
            }
        }

        html += "</ul>";
        html += "</body></html>";
        return html;
    }

    QMapIterator<QString, QString> it(map_contents);

    while (it.hasNext()) {
        it.next();

        if (it.value() == doc.toString()) {

            QFile f(QString(":/core/livedoc_%1.html").arg(doc.toString().replace("help:", "")));

            if (!f.open(QIODevice::ReadOnly)) {
                return QString();
            }

            QTextStream stream(&f);
            QString contents = stream.readAll();
            f.close();
            return contents;
        }
    }

    return QString();
}


const QMap<QString, QString>& Livedoc_Home_Provider::get_listed_contents()
{
    return map_contents;
}
