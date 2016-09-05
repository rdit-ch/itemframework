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


STARTUP_ADD_SINGLETON(GuiManager)


GuiManager::GuiManager()
{
    _mainWindow        = new Gui_Main_Window();
    _mainWindow->installEventFilter(this);

    _menuViewList << "&View";

    _menuBar = _mainWindow->menuBar();

    _mainWindow->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    _mainWindow->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    _mainWindow->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    _mainWindow->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);

    includeInLayout(new ConsoleWidget(ConsoleModel::instance()), Window_Layout::Bottom_Area);

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
    return true;
}

bool GuiManager::preDestroy()
{
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


void GuiManager::includeInMainmenue(QStringList menuItemList, QAction* menuAction)
{
    if (menuAction == NULL || menuItemList.isEmpty()) {
        return;
    }

    QString rootMenu = menuItemList.takeFirst();
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


    for (int i = 0; i < menuItemList.size(); ++i) {
        bool findItem = false;
        QString text = menuItemList.at(i);

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

    tmpMenue->addAction(menuAction);

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


void GuiManager::includeInLayout(QDockWidget* widget, Window_Layout::Area area)
{
    widget->setWindowTitle(widget->windowTitle());
    includeInMainmenue(_menuViewList, widget->toggleViewAction());


    switch (area) {
    case Window_Layout::Left_Area:
        _mainWindow->addDockWidget(Qt::LeftDockWidgetArea, widget);
        break;

    case Window_Layout::Right_Area:
        _mainWindow->addDockWidget(Qt::RightDockWidgetArea, widget);
        break;

    case Window_Layout::Bottom_Area:
        _mainWindow->addDockWidget(Qt::BottomDockWidgetArea, widget);
        break;

    }
}

bool GuiManager::removeFromLayout(QDockWidget* dockWidget)
{
    for (QDockWidget* dw : _mainWindow->findChildren<QDockWidget*>()) {
        if (dw == dockWidget) {
            _mainWindow->removeDockWidget(dockWidget);
            return true;
        }
    }

    return false;
}


void GuiManager::setCentralWidget(QWidget* w)
{
    _mainWindow->setCentralWidget(w);

}
void GuiManager::showMainWindow()
{
    _mainWindow->setGeometry(QStyle::alignedRect(
                                 Qt::LeftToRight,
                                 Qt::AlignCenter,
                                 _mainWindow->size(),
                                 qApp->desktop()->availableGeometry()));
    _mainWindow->showMaximized();
}


QWidget* GuiManager::widgetReference()
{
    return _mainWindow;
}

void GuiManager::showWidget(QWidget* widget)
{
    widget->show();
}

void GuiManager::showWidget(QWidget* widget, QPoint position)
{
    widget->move(position);
    widget->show();
}

void GuiManager::showDialogModal(QDialog* dialog)
{
    dialog->exec();
}

void GuiManager::showDialogModal(QDialog* dialog, QPoint widget_position)
{
    dialog->move(widget_position);
    dialog->exec();
}
