#ifndef GUI_MANAGER_P_H
#define GUI_MANAGER_P_H
#include <gui/gui_manager.h>

#include <QPointer>

struct Widget {
    WidgetType type;
    QPointer<QWidget> widget;
    QStringList children;
    Widget(WidgetType type = WidgetType::DockWidget, QWidget* widget = NULL) : type(type), widget(widget) {}
};
struct Action {
    QStringList parents;
    QAction* action;
    QStringList children;
    Action(QString parent = QString(), QAction* action = NULL) : parents(parent), action(action) {}
};
class GuiManagerPrivate : public QObject
{
    Q_OBJECT
public:

    explicit GuiManagerPrivate(GuiManager* parent);

    GuiManager* const q_ptr;
    Q_DECLARE_PUBLIC(GuiManager)

    QMainWindow* _mainWindow;
    QStringList _menuViewList;

    QList<std::function<bool()>> _closeApplicationCallbacks;
    QList<QPointer<QMainWindow>> _closeWindows;
    class GuiPluginManager* _uiPluginManager;
    QHash<QString, Widget> _widgets;
    QHash<QString, Action> _actions;
    bool _initialized = false;

    QStringList findTypes(WidgetType type) const;
    static bool isType(QWidget* widget, WidgetType type, QString const& name);
    bool nameUsed(QString const& name);
    /**
     * @brief Checks if the name is a valid parent to action
     * @param name Name of the Parent, must be an action that is a menu,
     * a menubar when the action is a menu or a toolbar when the action isn't a menu
     * @param action mustn't be NULL
     * @return true when
     */
    bool validParent(QString const& name, QAction *action);
    QString registerAction(QAction* action, QString const& parent, bool recursive=true);
    /**
     * @brief unregisterAction removes an action from the internal QHash
     * @param name name of the action
     * @param parent parent of the action
     */
    void unregisterAction(QString const& name, QString const& parent);
public slots:
    void saveState();
protected:
    bool eventFilter(QObject*, QEvent*);

};
#endif // GUI_MANAGER_P_H
