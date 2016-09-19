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
 * @brief The GuiManager class handels widgets on the main window and
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
     * @brief removeFromLayout removes dock widget from the GUI.
     * Caller takes ownership of the widget
     * @param widget id that is removed
     * @return Pointer to QDockWidget. Null if nothing has been removed.
     */
    QWidget* removeWidget(QString name);
    /**
     * @brief addWidget Adds a widget to the mainwindow.
     * @param widget pointer to the widger
     * @param name name of the specified widget, name can be used to access the widget later, must be unique
     * @param area dock- or toolbox-area where the widget will be placed
     * @param type what type of widget it is. Note that if a widget with a type that occurs only once
     * (CentralWidget, MenuBar, StatusBar) is added, the old widget will be deleted.
     * @return true if the widget could be added
     */
    bool addWidget(QWidget* widget, QString name, WidgetArea area, WidgetType type = WidgetType::DockWidget);
    /**
     * @brief moveWidget change the position of a widget
     * @param name name of the widget that should be moved
     * @param area area where the widget will be moved to
     * @return true if the action was successful
     */
    bool moveWidget(QString name, WidgetArea area);
    /**
     * @brief setVisible changes the visibility of a widget
     * @param name name of the widget
     * @param visible value that the visibility is set to
     */
    void setVisible(QString name, bool visible);
    /**
     * @brief includeInMainmenue Includes an action in a menu
     * @param menu string of the menu
     * @param action action that is added
     */
    void includeInMainmenue(QStringList menu, QAction* action);
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
     * @brief action Access to an action
     * @param name Name of the action
     * @return Reference to the action
     */
    QAction* action(QString name);
    /**
     * @brief This callback will be executed by an application close event.
     * If the callback returns true the close process will be continue -> shutdown application.
     * If the callback returns false the close process will be interrupted.
     * @param std::function<bool> callback
     */
    void registerCloseHandler(std::function<bool()> callback);
    void registerCloseWindow(QMainWindow* window);
    /**
     * @brief mainWindowIsActive
     * @return State of the main window
     */
    bool mainWindowIsActive() const;
private:
    QScopedPointer<class GuiManagerPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(GuiManager)

protected:
    bool eventFilter(QObject*, QEvent*);
public slots:

private slots:
    void saveState();
signals:
    void mainWindowActivationChange();
};
#endif // GUI_MANAGER_H
