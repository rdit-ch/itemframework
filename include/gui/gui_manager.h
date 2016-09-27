#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <QObject>
#include <QMainWindow>

#include <functional>
#include "appcore.h"
#include "helper/singleton.h"

/** @file */

///Information about the type of a widget
enum class WidgetType {
    DockWidget, ///< Widget that can be casted to a QDockwidget
    ToolBar, ///< Widget that can be casted into a Toolbar
    CentralWidget, ///< Widget that is placed in the center, only one can exist within a QMainWindow
    MenuBar, ///< Widget that can be casted into a QMenuBar, only one can exist within a QMainWindow
    StatusBar ///< Widget that can be casted into a QStatusBar, only one can exist within a QMainWindow
};
///Information about the area where a widget should be placed, Compatible with Qt::DockWidgetArea and Qt::ToolBarArea
enum class WidgetArea {
    NoArea = 0, ///< No area, use for widget that don't need an area
    Left = 0x1, ///< Left Area
    Right = 0x2, ///<Right Area
    Top = 0x4, ///<Top Area
    Bottom = 0x8, ///<Bottom Area
    AllAreas = 0xf ///< Mask for all areas

};

/**
 * @brief The GuiManager class handels widgets and actions on the main window
 */
class ITEMFRAMEWORK_EXPORT GuiManager : public QObject, public Singleton<GuiManager>
{
    //------------------Singleton Stuff---------------------------
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)
    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "PluginManager")

public:
    Q_INVOKABLE GuiManager();
    ~GuiManager();

protected:
    bool postInit();
    bool preDestroy();
    //------------------End Singleton Stuff---------------------------

public:
    /**
     * @brief removeFromLayout removes widget from the GUI.
     * Caller takes ownership of the widget
     * @param widget id that is removed
     * @return Pointer to QDockWidget. Null if nothing has been removed.
     */
    QWidget* removeWidget(QString const& name);
    /**
     * @brief Adds a widget to the mainwindow.
     * @param widget pointer to the widget, action that are already within a Toolbar or MenuBar are also registered whe
     * @param name name of the specified widget, name can be used to access the widget later, must be unique
     * @param area dock- or toolbox-area where the widget will be placed, use WidgetType::NoArea for other widgets
     * @param type what type of widget it is. Note that if a widget with a type that occurs only once
     * (CentralWidget, MenuBar, StatusBar) is added, the old widget will be deleted.
     * @return true if the widget could be added
     */
    bool addWidget(QWidget* widget, QString const& name, WidgetArea area, WidgetType type = WidgetType::DockWidget);
    /**
     * @brief Adds a widget to the mainwindow. \n Uses the objectName or className as name. \n
     * Appends a number if a a widget is already stored with that name
     * @param widget pointer to the widget, action that are already within a Toolbar or MenuBar are also registered
     * @param area dock- or toolbox-area where the widget will be placed, use WidgetType::NoArea for other widgets
     * @param type what type of widget it is. Note that if a widget with a type that occurs only once
     * (CentralWidget, MenuBar, StatusBar) is added, the old widget will be deleted.
     * @return name under which the widget was added, is empty when the widget couldn't be added
     */
    QString addWidget(QWidget* widget, WidgetArea area, WidgetType type = WidgetType::DockWidget);
    /**
     * @brief moveWidget change the position of a widget
     * @param name name of the widget that should be moved
     * @param area area where the widget will be moved to
     * @return true if the action was successful
     */
    bool moveWidget(QString const& name, WidgetArea area);
    /**
     * @brief setVisible changes the visibility of a widget
     * @param name name of the widget
     * @param visible value that the visibility is set to
     */
    void setVisible(QString const& name, bool visible);
    /**
     * @brief showMainWindow makes tha main window visible
     */
    void showMainWindow();
    /**
     * @brief widgetReference method to access the main window
     * @return  refrence to the mainwindow
     */
    QWidget* widgetReference();
    /**
     * @brief  adds an action a menu or a toolbar
     * @param Action The action that is added, GuiManager doesn't takes ownership, can't be NULL
     * @param Name The name of the action, is used to access the action later on, must be unique or
     * @param parent The name of the parent under which the action is added, parent must be an action that is a menu,
     * a menubar when the action is a menu or a toolbar when the action isn't a menu
     * @return True when the action could be added
     */
    bool addAction(QAction* action, QString const& name, QString const& parent);
    /**
     * @brief Adds the action name to the action or widget parent
     * @param name Name of the action or widget, must be an action
     * @param parent, The name of the parent under which the action is added, parent must be an action that is a menu,
     * a menubar when the action is a menu or a toolbar when the action isn't a menu
     * @return True when the action could be added to parent
     */
    bool addActionToParent(QString const& name, QString const& parent);
    /**
     * @brief Adds an action to a menu or a toolbar \n Uses the objectName or className as name. \n
     * Appends a number if something is already stored with that name
     * @param action Pointer to the action, GuiManager doesn't takes ownership
     * @param parent The name of the parent under which the action is added, parent must be an action that is a menu,
     * a menubar when the action is a menu or a toolbar when the action isn't a menu
     * @return name under which the action was added, is empty when the widget couldn't be added
     */
    QString addAction(QAction* action, QString const& parent);
    /**
     * @brief Access to reference of an action
     * @param name Name of the action
     * @return Reference to the action, NULL when it doesn't exist
     */
    QAction* action(QString const& name) const;
    /**
     * @brief Removes an action from the gui and returns it.
     * @param name name of the action
     * @param parent parent of the action, will remove all occurences if string is empty. None when parent doesn't exist
     * @return removed action, NULL when name or parent doesn't exist
     */
    QAction* removeAction(QString const& name, QString const& parent = QString());
    /**
     * @brief This callback will be executed by an application close event.
     * If the callback returns true the close process will be continue -> shutdown application.
     * If the callback returns false the close process will be interrupted.
     * @param std::function<bool> callback
     */
    void registerCloseHandler(std::function<bool()> callback);
    /**
     * @brief Access to the names of the parents of an registered action
     * @param name Name of the action
     * @return List of the names of all parents of name
     */
    QStringList parents(QString const& name);
    /**
     * @brief Access to the names of the children of an registered action or widget
     * @param name Name of the action or widget
     * @return List of the names of all children of name
     */
    QStringList children(QString const& name) const;
    /**
     * @brief Access to all the names of the registered actions
     * @return List containing all names of the registered actions
     */
    QStringList registeredActions()const;
    /**
     * @brief Access to all the names of the registered widgets
     * @return List containing all names of the registered widgets
     */
    QStringList registeredWidgets() const;
    /**
     * @brief registeredWidgets return all registered widgets of the given type
     * @param type Type to be returned
     * @return Names of widgets
     */
    QStringList registeredWidgets(WidgetType type) const;
    /**
     * @brief mainWindowIsActive
     * @return State of the main window
     */
    bool mainWindowIsActive() const;

signals:
    /**
     * @brief is emited when the activation of the main window changed
     */
    void mainWindowActivationChange();

private:
    QScopedPointer<class GuiManagerPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(GuiManager)

};
#endif // GUI_MANAGER_H
