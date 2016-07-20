#ifndef LIVEDOC_BROWSER_H
#define LIVEDOC_BROWSER_H

#include <QTextBrowser>

class Livedoc_Browser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit Livedoc_Browser(QWidget* parent = 0);
    virtual QVariant loadResource(int type, const QUrl& name);



};

#endif // LIVEDOC_BROWSER_H
