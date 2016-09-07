#include "gui/gui_manager.h"
#include "gui_main_window.h"
#include "helper/startup_helper.h"
#include "error/console_widget.h"
#include "error/console_model.h"
#include "project/project_manager_gui.h"
#include "gui/gui_plugin_manager.h"
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QPointer>
#include <QDesktopWidget>
#include <QSettings>
#include <QToolBar>
#include <QStatusBar>


STARTUP_ADD_SINGLETON(GuiManager)


GuiManager::GuiManager()
{
    _mainWindow = new Gui_Main_Window();
    _mainWindow->installEventFilter(this);

    _menuViewList << "&View";

    _menuBar = _mainWindow->menuBar();

    _mainWindow->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    _mainWindow->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    _mainWindow->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    _mainWindow->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
    ConsoleWidget* localConsoleWidget = new ConsoleWidget(ConsoleModel::instance());

    //localConsoleWidget->setVisible(false);
    if (!addWidget(localConsoleWidget, WidgetName::Console, WidgetArea::Bottom, WidgetType::DockWidget)) {
        qWarning() << "failed to insert Console Widget";
    }

    // plugin manager
    _uiPluginManager = new GuiPluginManager(_mainWindow);

}

GuiManager::~GuiManager()
{
    delete _uiPluginManager;
}

bool GuiManager::postInit()
{
    showMainWindow();
    _mainWindow->setDisabled(true);

    if (!ProjectManagerGui::instance()->start()) {
        _mainWindow->close();
    }

    _mainWindow->setDisabled(false);

    QSettings settings("Ruag", "traviz");
    settings.sync();
    _mainWindow->restoreState(settings.value("windowState").toByteArray());
    //_mainWindow->restoreGeometry(settings.value("geometry").toByteArray());
    _initialized = true;
    return true;
}

bool GuiManager::preDestroy()
{
    QSettings settings("Ruag", "traviz");
    //settings.setValue("geometry", _mainWindow->saveGeometry());
    settings.setValue("windowState", _mainWindow->saveState());
    settings.sync();
    return true;
}

bool GuiManager::eventFilter(QObject* o, QEvent* e)
{
    if (o == _mainWindow && e->type() == QEvent::Close) {
        QCloseEvent* ce = (QCloseEvent*) e;

        // Go through all callbacks in callbacklist
        for (int i = 0; i < _closeApplicationCallbacks.count(); i++) {
            if (!(_closeApplicationCallbacks.at(i))()) {
                // If a callback return false - interrup close procedure and
                // ignore the close event
                ce->ignore();
                // Filter the event out of the eventsystem
                return true;
            }
        }

        // Close all registred windows
        for (int i = 0; i < _closeWindows.count(); i++) {
            // If (Q)Pointer is still valid (Window not deleted yet)
            if (_closeWindows.at(i)) {
                // Hide window (deletion should be handled by owner)
                _closeWindows.at(i)->hide();
            }
        }
    }

    if (o == _mainWindow && e->type() == QEvent::ActivationChange) {
        emit mainWindowActivationChange();
    }

    // Execute event and return success state
    return QObject::eventFilter(o, e);
}

void GuiManager::saveState()
{
    if (_initialized) {
        QSettings settings("Ruag", "traviz");
        settings.setValue("windowState", _mainWindow->saveState());
    }
}

void GuiManager::registerCloseHandler(std::function<bool()> callback)
{
    _closeApplicationCallbacks.append(callback);

}

QT_DEPRECATED void GuiManager::registerCloseWindow(QMainWindow* window)
{
    //destroy the window in the destructor of the owner when possible!
    _closeWindows.append(QPointer<QMainWindow>(window));

}

bool GuiManager::mainWindowIsActive() const
{
    return _mainWindow->isActiveWindow();
}
void GuiManager::includeInMainmenue(QStringList menu, QAction* action)
{
    if (action == NULL || menu.isEmpty()) {
        return;
    }

    QString rootMenu = menu.takeFirst();
    QMenu*  tmpMenue = NULL;

    foreach (QAction* a, _menuBar->actions()) {
        if (rootMenu == a->text()) {
            tmpMenue = a->menu();
            break;
        }
    }

    if (tmpMenue == NULL) {
        tmpMenue = _menuBar->addMenu(rootMenu);
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
            tmpMenue = _menuBar->addMenu(text);
        }
    }

    tmpMenue->addAction(action);

}

QAction* GuiManager::action(QString name)
{
    foreach (QAction* a, _menuBar->actions()) {
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

QAction* GuiManager::action(QMenu* parent, QString name)
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


bool GuiManager::addWidget(QWidget* widget, WidgetName name, WidgetArea area, WidgetType type)
{
    if (_widgets.contains(static_cast<quint32>(name)) || widget == NULL) {
        return false;
    }

    switch (type) {
    case WidgetType::DockWidget: {

        QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(widget);

        if (dockWidget == NULL) {
            return false;
        }

        includeInMainmenue(_menuViewList, dockWidget->toggleViewAction());
        _mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area), dockWidget);
        connect(dockWidget, &QDockWidget::dockLocationChanged, this, &GuiManager::saveState);
        connect(dockWidget, &QDockWidget::visibilityChanged, this, &GuiManager::saveState);
        break;
    }

    case WidgetType::ToolBar: {
        QToolBar* toolBar = dynamic_cast<QToolBar*>(widget);

        if (toolBar == NULL) {
            return false;
        }

        includeInMainmenue(_menuViewList, toolBar->toggleViewAction());
        _mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area), toolBar);
        break;
    }

    case WidgetType::CentralWidget:
        //includeInMainmenue(_menuViewList, widget->);
        _mainWindow->setCentralWidget(widget);
        break;

    case WidgetType::MenuBar: {
        QMenuBar* menuBar = dynamic_cast<QMenuBar*>(widget);

        if (menuBar == NULL) {
            return false;
        }

        _mainWindow->setMenuBar(menuBar);
        break;
    }

    case WidgetType::StatusBar: {
        QStatusBar* statusBar = dynamic_cast<QStatusBar*>(widget);

        if (statusBar == NULL) {
            return false;
        }

        _mainWindow->setStatusBar(statusBar);
        break;
    }
    }

    _widgets.insert(static_cast<quint32>(name), qMakePair<WidgetType, QWidget*>(type, widget));
    return true;
}

bool GuiManager::moveWidget(WidgetName name, WidgetArea area)
{
    if (!_widgets.contains(static_cast<quint32>(name))) {
        return false;
    }

    QWidget* widget = _widgets[static_cast<quint32>(name)].second;
    WidgetType type = _widgets[static_cast<quint32>(name)].first;

    switch (type) {
    case WidgetType::DockWidget:
        //_mainWindow->removeDockWidget(dynamic_cast<QDockWidget*>(widget));
        _mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area),
                                   dynamic_cast<QDockWidget*>(widget));
        break;

    case WidgetType::ToolBar:
        _mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area),
                                dynamic_cast<QToolBar*>(widget));
        break;

    default:
        return false;
    }

    return true;
}

void GuiManager::setVisible(WidgetName name, bool visible)
{
    _widgets[static_cast<quint32>(name)].second->setVisible(visible);
}

QWidget* GuiManager::removeWidget(WidgetName name)
{
    QWidget* widget = _widgets.take(static_cast<quint32>(name)).second;
    WidgetType type = _widgets.take(static_cast<quint32>(name)).first;

    switch (type) {
    case WidgetType::CentralWidget:
        if (widget == _mainWindow->centralWidget()) {
            _mainWindow->takeCentralWidget();
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::MenuBar:
        if (widget == _mainWindow->menuBar()) {
            _mainWindow->setMenuBar(NULL);
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::StatusBar:
        if (widget == _mainWindow->statusBar()) {
            _mainWindow->setStatusBar(NULL);
        } else {
            widget = NULL;
        }

        break;

    case WidgetType::DockWidget:
        _mainWindow->removeDockWidget(dynamic_cast<QDockWidget*>(widget));
        disconnect(dynamic_cast<QDockWidget*>(widget), NULL, this, NULL);
        break;

    case WidgetType::ToolBar:
        _mainWindow->removeToolBar(dynamic_cast<QToolBar*>(widget));
        break;
    }

    return widget;
}

void GuiManager::showMainWindow()
{
    _mainWindow->setGeometry(QStyle::alignedRect(
                                 Qt::LeftToRight,
                                 Qt::AlignCenter,
                                 _mainWindow->size(),
                                 qApp->desktop()->availableGeometry()));
    _mainWindow->show();
    //showMaximized causes problems with awesome-wm.
}


QWidget* GuiManager::widgetReference()
{
    return _mainWindow;
}
