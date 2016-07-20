#include "item/abstract_window_item.h"
#include "abstract_window_item_p.h"

#include <QMenu>
#include <QDebug>

AbstractWindowItem::AbstractWindowItem(QString const& typeName) :
    AbstractItem(typeName), d_ptr(new AbstractWindowItemPrivate(this))
{
}

AbstractWindowItem::~AbstractWindowItem()
{
    Q_D(AbstractWindowItem);

    if (d->_window != nullptr) {
        delete d->_window;
    }

    for (auto entry : d->_contextMenuEntries) {
        delete entry;
    }
}

AbstractWindowItemPrivate::AbstractWindowItemPrivate(AbstractWindowItem* parent) :
    q_ptr(parent)
{
}

void AbstractWindowItem::setWindow(QMainWindow* window)
{
    Q_D(AbstractWindowItem);

    d->_window = window;

    d->setupWindow();
}

void AbstractWindowItemPrivate::setupWindow()
{
    Q_Q(AbstractWindowItem);

    if (_window != nullptr) {
        _window->setWindowTitle(q->name());
    }
}

QMainWindow* AbstractWindowItem::window() const
{
    Q_D(const AbstractWindowItem);

    return d->_window;
}

void AbstractWindowItem::deleteWindow()
{
    Q_D(AbstractWindowItem);

    if (d->_window != nullptr) {
        delete d->_window;
        d->_window = nullptr;
    }
}

void AbstractWindowItem::hideWindow()
{
    Q_D(AbstractWindowItem);

    if (d->_window != nullptr) {
        d->_window->hide();
    }
}

void AbstractWindowItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    AbstractItem::mouseDoubleClickEvent(event);

    openWindow();
}

void AbstractWindowItem::setOpenWindowContextActionEnabled(bool enabled) const
{
    Q_D(const AbstractWindowItem);

    d->_openWindowContextActionEnabled = enabled;
}

void AbstractWindowItem::contextMenuPrepare(QMenu& menu) const
{
    Q_D(const AbstractWindowItem);

    // Add "Open Window" entry
    QAction* action = menu.addAction("Open Window");
    QFont font = QFont(action->font());
    font.setWeight(70);
    action->setFont(font);
    action->setEnabled(d->_openWindowContextActionEnabled);

    connect(action, SIGNAL(triggered()), this, SLOT(openWindow()));

    d->_openWindowContextActionEnabled = true;

    // Separate custom entries
    if (d->_contextMenuEntries.count() > 0) {
        menu.addSeparator();
    }

    // Add custom entries
    for (auto entry : d->_contextMenuEntries) {
        menu.addAction(entry);
    }
}

QAction* AbstractWindowItem::addContextMenuEntry(QString const& text, std::function<void()> f)
{
    Q_D(AbstractWindowItem);

    QAction* action = new QAction(text, this);

    if (f != nullptr) {
        connect(action, &QAction::triggered, f);
    }

    d->_contextMenuEntries.append(action);

    return action;
}

void AbstractWindowItem::openWindow()
{
    Q_D(AbstractWindowItem);

    if (d->_window == nullptr) {
        d->_window = allocateWindow();

        if (d->_window == nullptr) {
            qCritical() << "BasicItem: Failed to allocate window!";
        } else {
            d->setupWindow();
        }
    }

    if (d->_window != nullptr) {
        if (! d->_window->isVisible()) {
            d->_window->show();
        } else {
            d->_window->activateWindow();
        }
    }
}
