#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <QObject>
#include <QMainWindow>

#include <functional>
#include "appcore.h"
#include "helper/singleton.h"

namespace Window_Layout
{
enum Area {
    Left_Area,
    Right_Area,
    Bottom_Area//,
    // Center_Area
};
}

class CORE_EXPORT Gui_Manager : public QObject, public Singleton<Gui_Manager>
{
    //------------------Singleton Stuff---------------------------
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)
    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "PluginManager")

public:
    Q_INVOKABLE Gui_Manager();
    ~Gui_Manager();

protected:
    bool postInit();
    bool preDestroy();
    //------------------End Singleton Stuff---------------------------

public:

    bool removeFromLayout(QDockWidget* dockWidget);
    void        include_In_Layout(QDockWidget*, Window_Layout::Area);
    void        include_In_Mainmenue(QStringList, QAction*);
    void        set_central_widget(QWidget* w);
    void        show_mainwindow();
    void        show_widget(QWidget* widget);
    void        show_widget(QWidget* widget, QPoint position);
    void        show_dialog_modal(QDialog* dialog);
    void        show_dialog_modal(QDialog* dialog, QPoint position);
    QWidget*    get_widget_reference();
    QAction*    get_action(QString name);
    /**
     * @brief This callback will be executed by an application close event.
     * If the callback returns true the close process will be continue -> shutdown application.
     * If the callback returns false the close process will be interrupted.
     * @param std::function<bool> callback
     */
    void        register_close_handler(std::function<bool()> callback);
    void        register_close_window(QMainWindow* window);
    bool mainWindowIsActive() const;
private:

    QMainWindow*                main_window;
    QMenuBar*                    menubar;
    QStringList                 menu_view_list;
    QList<std::function<bool()>>  _closeApplicationCallbacks;
    QList<QPointer<QMainWindow>>  lis_close_window;
    QAction* get_action(QMenu* parent, QString name);
    class GuiPluginManager* _uiPluginManager;


protected:
    bool eventFilter(QObject*, QEvent*);
public slots:
private slots:
signals:
    void mainWindowActivationChange();
};

#endif // GUI_MANAGER_H
