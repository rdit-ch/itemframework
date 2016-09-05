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

class CORE_EXPORT GuiManager : public QObject, public Singleton<GuiManager>
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

    bool removeFromLayout(QDockWidget* dockWidget);
    void        includeInLayout(QDockWidget*, Window_Layout::Area);
    void        includeInMainmenue(QStringList, QAction*);
    void        setCentralWidget(QWidget* w);
    void        showMainWindow();
    void        showWidget(QWidget* widget);
    void        showWidget(QWidget* widget, QPoint position);
    void        showDialogModal(QDialog* dialog);
    void        showDialogModal(QDialog* dialog, QPoint position);
    QWidget*    widgetReference();
    QAction*    action(QString name);
    /**
     * @brief This callback will be executed by an application close event.
     * If the callback returns true the close process will be continue -> shutdown application.
     * If the callback returns false the close process will be interrupted.
     * @param std::function<bool> callback
     */
    void        registerCloseHandler(std::function<bool()> callback);
    void        registerCloseWindow(QMainWindow* window);
    bool mainWindowIsActive() const;
private:

    QMainWindow*                _mainWindow;
    QMenuBar*                    _menuBar;
    QStringList                 _menuViewList;
    QList<std::function<bool()>>  _closeApplicationCallbacks;
    QList<QPointer<QMainWindow>>  _closeWindows;
    QAction* action(QMenu* parent, QString name);
    class GuiPluginManager* _uiPluginManager;


protected:
    bool eventFilter(QObject*, QEvent*);
public slots:
private slots:
signals:
    void mainWindowActivationChange();
};

#endif // GUI_MANAGER_H
