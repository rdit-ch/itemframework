#ifndef ABSTRACT_WINDOW_ITEM_H
#define ABSTRACT_WINDOW_ITEM_H

#include "appcore.h"
#include "abstract_item.h"

#include <QObject>
#include <QMainWindow>
#include <functional>

/**
 * @brief The AbstractWindowItem class provides basic standard functionality for a typical
 * item that uses a window.
 *
 * It has a pure virtual function, \a allocateWindow. This function needs to be
 * implemented by the derived class and is used by this base class to construct
 * a window for this item.
 *
 * \see AbstractItem
 */
class CORE_EXPORT AbstractWindowItem : public AbstractItem
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a AbstractWindowItem with the given name
     * @param typeName The name of the item to construct
     */
    explicit AbstractWindowItem(QString const& typeName);

    virtual ~AbstractWindowItem() = 0; // prevent this class from being instantiated directly, enforce deriving this class

protected:
    /**
     * @brief This is a pure virtual function. The implementation has to construct
     * and return a QMainWindow which this base class can then manage
     * @return The window that belongs to this item
     */
    virtual QMainWindow* allocateWindow() = 0;

    /**
     * @return The window of this item
     *
     * \sa setWindow
     * \sa deleteWindow
     * \sa hideWindow
     */
    QMainWindow* window() const;

    /**
     * @param window The window to couple this item to
     *
     * \sa window
     * \sa deleteWindow
     * \sa hideWindow
     */
    void setWindow(QMainWindow* window);

    /**
     * @brief Deletes the window of this item
     *
     * \sa window
     * \sa setWindow
     * \sa hideWindow
     */
    void deleteWindow();

    /**
     * @brief Hides the window of this item
     *
     * \sa window
     * \sa setWindow
     * \sa deleteWindow
     */
    void hideWindow();

    /**
     * @brief This function allows the derived class to enable or disable the
     * context menu option to open this items window for the current context menu.
     * It is typically called in \a contextMenuPrepare, if needed. The value is
     * reset to its default (true) whenever the user opens a new context menu.
     * @param enabled Whether or not to enable the context menu option to open
     * the window
     *
     * \sa contextMenuPrepare
     */
    void setOpenWindowContextActionEnabled(bool enabled) const;

    /**
     * @brief This function adds a new context menu entry
     * @param text The text to display for the context menu entry
     * @param f The function to call when the user clicks the context menu entry
     * @return A pointer to the newly created context menu action
     *
     * \sa contextMenuPrepare
     */
    QAction* addContextMenuEntry(QString const& text, std::function<void()> f = nullptr);

    virtual void contextMenuPrepare(QMenu& menu) const;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

protected slots:
    /**
     * @brief This is a default implementation of a slot which opens this items
     * window, if any. If the item is not allocated yet, it uses \a allocateWindow
     * to request one from the derived class. Override this function for custom
     * behavior
     */
    virtual void openWindow();

private:
    QScopedPointer<class AbstractWindowItemPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(AbstractWindowItem)
};

#endif // ABSTRACT_WINDOW_ITEM_H
