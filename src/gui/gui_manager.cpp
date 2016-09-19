#include "gui/gui_manager.h"
#include "gui_main_window.h"
#include "helper/startup_helper.h"
#include "error/console_widget.h"
#include "error/console_model.h"
#include "project/project_manager_gui.h"
#include "gui/gui_plugin_manager.h"
#include "gui_manager_p.h"
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QPointer>
#include <QDesktopWidget>
#include <QSettings>
#include <QToolBar>
#include <QStatusBar>


STARTUP_ADD_SINGLETON(GuiManager)

GuiManager::GuiManager(): d_ptr(new GuiManagerPrivate(this))
{
    Q_D(GuiManager);
    d->_mainWindow = new Gui_Main_Window();
    d->_mainWindow->installEventFilter(this);

    d->_menuViewList << "&View";

    d->_menuBar = d->_mainWindow->menuBar();

    d->_mainWindow->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    d->_mainWindow->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    d->_mainWindow->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    d->_mainWindow->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
    ConsoleWidget* localConsoleWidget = new ConsoleWidget(ConsoleModel::instance());

    //localConsoleWidget->setVisible(false);
    if (!addWidget(localConsoleWidget, "Console", WidgetArea::Bottom, WidgetType::DockWidget)) {
        qWarning() << "failed to insert Console Widget";
    }
    addWidget(static_cast<Gui_Main_Window*>(d->_mainWindow)->toolBar(),"ToolBar",WidgetArea::Left,WidgetType::ToolBar);
    addWidget(static_cast<Gui_Main_Window*>(d->_mainWindow)->statusBar(),"StatusBar",WidgetArea::NoArea,WidgetType::MenuBar);

    // plugin manager
    d->_uiPluginManager = new GuiPluginManager(d->_mainWindow);


}

GuiManager::~GuiManager()
{
    Q_D(GuiManager);
    delete d->_uiPluginManager;
}

bool GuiManager::postInit()
{
    Q_D(GuiManager);
    showMainWindow();
    d->_mainWindow->setDisabled(true);

    if (!ProjectManagerGui::instance()->start()) {
        d->_mainWindow->close();
    }

    d->_mainWindow->setDisabled(false);

    QSettings settings("Ruag", "traviz");
    settings.sync();
    d->_mainWindow->restoreState(settings.value("windowState").toByteArray());
    //_mainWindow->restoreGeometry(settings.value("geometry").toByteArray());
    d->_initialized = true;
    return true;
}

bool GuiManager::preDestroy()
{
    Q_D(GuiManager);
    QSettings settings("Ruag", "traviz");
    //settings.setValue("geometry", _mainWindow->saveGeometry());
    settings.setValue("windowState", d->_mainWindow->saveState());
    settings.sync();
    return true;
}

bool GuiManager::eventFilter(QObject* o, QEvent* e)
{
    Q_D(GuiManager);
    if (o == d->_mainWindow && e->type() == QEvent::Close) {
        QCloseEvent* ce = (QCloseEvent*) e;

        // Go through all callbacks in callbacklist
        for (int i = 0; i < d->_closeApplicationCallbacks.count(); i++) {
            if (!(d->_closeApplicationCallbacks.at(i))()) {
                // If a callback return false - interrup close procedure and
                // ignore the close event
                ce->ignore();
                // Filter the event out of the eventsystem
                return true;
            }
        }

        // Close all registred windows
        for (int i = 0; i < d->_closeWindows.count(); i++) {
            // If (Q)Pointer is still valid (Window not deleted yet)
            if (d->_closeWindows.at(i)) {
                // Hide window (deletion should be handled by owner)
                d->_closeWindows.at(i)->hide();
            }
        }
    }

    if (o == d->_mainWindow && e->type() == QEvent::ActivationChange) {
        emit mainWindowActivationChange();
    }

    // Execute event and return success state
    return QObject::eventFilter(o, e);
}

void GuiManager::saveState()
{
    Q_D(GuiManager);
    if (d->_initialized) {
        QSettings settings("Ruag", "traviz");
        settings.setValue("windowState", d->_mainWindow->saveState());
    }
}

void GuiManager::registerCloseHandler(std::function<bool()> callback)
{
    Q_D(GuiManager);
    d->_closeApplicationCallbacks.append(callback);
}

QT_DEPRECATED void GuiManager::registerCloseWindow(QMainWindow* window)
{
    Q_D(GuiManager);
    //destroy the window in the destructor of the owner when possible!
    d->_closeWindows.append(QPointer<QMainWindow>(window));
}

bool GuiManager::mainWindowIsActive() const
{
    Q_D(const GuiManager);
    return d->_mainWindow->isActiveWindow();
}
void GuiManager::includeInMainmenue(QStringList menu, QAction* action)
{
    Q_D(GuiManager);
    if (action == NULL || menu.isEmpty()) {
        return;
    }

    QString rootMenu = menu.takeFirst();
    QMenu*  tmpMenue = NULL;

    foreach (QAction* a, d->_menuBar->actions()) {
        if (rootMenu == a->text()) {
            tmpMenue = a->menu();
            break;
        }
    }

    if (tmpMenue == NULL) {
        tmpMenue = d->_menuBar->addMenu(rootMenu);
    }
    for (int i = 0; i < menu.size(); ++i) {
        bool findItem = false;
        QString text = menu.at(i);

        foreach (QAction* a, tmpMenue->actions()) {
            if (text == a->text()) {
                findItem = true;
                tmpMenue = a->menu();
                break;
            }
        }

        if (!findItem) {
            tmpMenue = d->_menuBar->addMenu(text);
        }
    }

    tmpMenue->addAction(action);
}

QAction* GuiManager::action(QString name)
{
    Q_D(GuiManager);
    foreach (QAction* a, d->_menuBar->actions()) {
        QMenu* submenu = a->menu();

        if (submenu == NULL) {
            continue;
        }

        QAction* sub = d->action(submenu, name);

        if (sub != NULL) {
            return sub;
        }
    }

    return NULL;
}

QAction* GuiManagerPrivate::action(QMenu* parent, QString name)
{
    foreach (QAction* a, parent->actions()) {
        if (a->text().replace("&", "") == name) {
            return a;
        }

        QMenu* submenu = a->menu();

        if (submenu == NULL) {
            continue;
        }

        QAction* sub = action(submenu, name);

        if (sub != NULL) {
            return sub;
        }
    }

    return NULL;
}


bool GuiManager::addWidget(QWidget* widget, QString name, WidgetArea area, WidgetType type)
{
    Q_D(GuiManager);
    if (d->_widgets.contains(name) || widget == NULL) {
        return false;
    }

    switch (type) {
    case WidgetType::DockWidget: {

        QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(widget);

        if (dockWidget == NULL) {
            return false;
        }

        includeInMainmenue(d->_menuViewList, dockWidget->toggleViewAction());
        d->_mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area), dockWidget);
        connect(dockWidget, &QDockWidget::dockLocationChanged, this, &GuiManager::saveState);
        connect(dockWidget, &QDockWidget::visibilityChanged, this, &GuiManager::saveState);
        break;
    }

    case WidgetType::ToolBar: {
        QToolBar* toolBar = dynamic_cast<QToolBar*>(widget);
        connect(toolBar, &QToolBar::topLevelChanged, this, &GuiManager::saveState);
        connect(toolBar, &QToolBar::visibilityChanged, this, &GuiManager::saveState);

        if (toolBar == NULL) {
            return false;
        }

        includeInMainmenue(d->_menuViewList, toolBar->toggleViewAction());
        d->_mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area), toolBar);
        break;
    }

    case WidgetType::CentralWidget:
        //includeInMainmenue(_menuViewList, widget->);
        d->_mainWindow->setCentralWidget(widget);
        break;

    case WidgetType::MenuBar: {
        QMenuBar* menuBar = dynamic_cast<QMenuBar*>(widget);

        if (menuBar == NULL) {
            return false;
        }

        d->_mainWindow->setMenuBar(menuBar);
        break;
    }

    case WidgetType::StatusBar: {
        QStatusBar* statusBar = dynamic_cast<QStatusBar*>(widget);

        if (statusBar == NULL) {
            return false;
        }

        d->_mainWindow->setStatusBar(statusBar);
        break;
    }
    }

    d->_widgets.insert(name, qMakePair<WidgetType, QWidget*>(type, widget));
    return true;
}

bool GuiManager::moveWidget(QString name, WidgetArea area)
{
    Q_D(GuiManager);
    if (!d->_widgets.contains(name)) {
        return false;
    }

    QWidget* widget = d->_widgets[name].second;
    WidgetType type = d->_widgets[name].first;

    switch (type) {
    case WidgetType::DockWidget:
        //_mainWindow->removeDockWidget(dynamic_cast<QDockWidget*>(widget));
        d->_mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area),
                                   dynamic_cast<QDockWidget*>(widget));
        break;

    case WidgetType::ToolBar:
        d->_mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area),
                                dynamic_cast<QToolBar*>(widget));
        break;

    default:
        return false;
    }

    return true;
}

void GuiManager::setVisible(QString name, bool visible)
{
    Q_D(GuiManager);
    d->_widgets[name].second->setVisible(visible);
}

QWidget* GuiManager::removeWidget(QString name)
{
    Q_D(GuiManager);
    QWidget* widget = d->_widgets.take(name).second;
    WidgetType type = d->_widgets.take(name).first;

    switch (type) {
    case WidgetType::CentralWidget:
        if (widget == d->_mainWindow->centralWidget()) {
            d->_mainWindow->takeCentralWidget();
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::MenuBar:
        if (widget == d->_mainWindow->menuBar()) {
            d->_mainWindow->setMenuBar(NULL);
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::StatusBar:
        if (widget == d->_mainWindow->statusBar()) {
            d->_mainWindow->setStatusBar(NULL);
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::DockWidget:
        d->_mainWindow->removeDockWidget(dynamic_cast<QDockWidget*>(widget));
        disconnect(dynamic_cast<QDockWidget*>(widget), NULL, this, NULL);
        break;

    case WidgetType::ToolBar:
        d->_mainWindow->removeToolBar(dynamic_cast<QToolBar*>(widget));
        disconnect(dynamic_cast<QToolBar*>(widget), NULL, this, NULL);
        break;
    }

    return widget;
}

void GuiManager::showMainWindow()
{
    Q_D(GuiManager);
    d->_mainWindow->setGeometry(QStyle::alignedRect(
                                 Qt::LeftToRight,
                                 Qt::AlignCenter,
                                 d->_mainWindow->size(),
                                 qApp->desktop()->availableGeometry()));
    d->_mainWindow->show();
    //showMaximized causes problems with awesome-wm.
}


QWidget* GuiManager::widgetReference()
{
    Q_D(GuiManager);
    return d->_mainWindow;
}

GuiManagerPrivate::GuiManagerPrivate(GuiManager* parent) :
    q_ptr(parent)
{
}
