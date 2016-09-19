#ifndef GUI_MANAGER_P_H
#define GUI_MANAGER_P_H
#include <gui/gui_manager.h>
class GuiManagerPrivate
{
public:
    explicit GuiManagerPrivate(GuiManager* parent);

    GuiManager* const q_ptr;
    Q_DECLARE_PUBLIC(GuiManager)



    QMainWindow* _mainWindow;
    QMenuBar* _menuBar;
    QStringList _menuViewList;
    QList<std::function<bool()>> _closeApplicationCallbacks;
    QList<QPointer<QMainWindow>> _closeWindows;
    class GuiPluginManager* _uiPluginManager;
    QHash<QString, QPair<WidgetType, QWidget*>> _widgets;
    bool _initialized = false;

    QAction* action(QMenu* parent, QString name);

};
#endif // GUI_MANAGER_P_H
