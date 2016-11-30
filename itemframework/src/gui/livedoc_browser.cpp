#include "livedoc_browser.h"
#include "gui/livedoc.h"

Livedoc_Browser::Livedoc_Browser(QWidget* parent) :
    QTextBrowser(parent)
{
}


QVariant  Livedoc_Browser::loadResource(int type, const QUrl& name)
{

    if (type == QTextDocument::HtmlResource) {
        return Livedoc::instance()->get_doc(name);
    }

    return QTextBrowser::loadResource(type, name);
}

