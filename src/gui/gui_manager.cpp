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


STARTUP_ADD_SINGLETON(Gui_Manager)


Gui_Manager::Gui_Manager()
{
    main_window        = new Gui_Main_Window();
    main_window->installEventFilter(this);

    menu_view_list << "&View";

    menubar = main_window->menuBar();

    main_window->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    main_window->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    main_window->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    main_window->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);

    include_In_Layout(new ConsoleWidget(ConsoleModel::instance()), Window_Layout::Bottom_Area);

    // plugin manager
    _uiPluginManager = new GuiPluginManager(main_window);

}

Gui_Manager::~Gui_Manager()
{
    delete _uiPluginManager;
}

bool Gui_Manager::postInit()
{
    show_mainwindow();
    main_window->setDisabled(true);

    if (!ProjectManagerGui::instance()->start()) {
        main_window->close();
    }

    main_window->setDisabled(false);
    return true;
}

bool Gui_Manager::preDestroy()
{
    return true;
}

bool Gui_Manager::eventFilter(QObject* o, QEvent* e)
{
    if (o == main_window && e->type() == QEvent::Close) {
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
        for (int i = 0; i < lis_close_window.count(); i++) {
            // If (Q)Pointer is still valid (Window not deleted yet)
            if (lis_close_window.at(i)) {
                // Hide window (deletion should be handled by owner)
                lis_close_window.at(i)->hide();
            }
        }
    }

    if (o == main_window && e->type() == QEvent::ActivationChange) {
        emit mainWindowActivationChange();
    }

    // Execute event and return success state
    return QObject::eventFilter(o, e);
}



void Gui_Manager::register_close_handler(std::function<bool()> callback)
{
    _closeApplicationCallbacks.append(callback);

}

QT_DEPRECATED void Gui_Manager::register_close_window(QMainWindow* window)
{
    //destroy the window in the destructor of the owner when possible!
    lis_close_window.append(QPointer<QMainWindow>(window));

}

bool Gui_Manager::mainWindowIsActive() const
{
    return main_window->isActiveWindow();
}


void Gui_Manager::include_In_Mainmenue(QStringList menu_item_list, QAction* menu_action)
{
    if (menu_action == NULL || menu_item_list.isEmpty()) {
        return;
    }

    QString root_menu = menu_item_list.takeFirst();
    QMenu*  tmp_Menue = NULL;

    foreach (QAction* a, menubar->actions()) {
        if (root_menu == a->text()) {
            tmp_Menue = a->menu();
            break;
        }
    }

    if (tmp_Menue == NULL) {
        tmp_Menue = menubar->addMenu(root_menu);
    }


    for (int i = 0; i < menu_item_list.size(); ++i) {
        bool find_item = false;
        QString text = menu_item_list.at(i);

        foreach (QAction* a, tmp_Menue->actions()) {
            if (text == a->text()) {
                find_item = true;
                tmp_Menue = a->menu();
                break;
            }
        }

        if (!find_item) {
            tmp_Menue = menubar->addMenu(text);
        }
    }

    tmp_Menue->addAction(menu_action);

}

QAction* Gui_Manager::get_action(QString name)
{
    foreach (QAction* a, menubar->actions()) {
        QMenu* submenu = a->menu();

        if (submenu == NULL) {
            continue;
        }

        QAction* sub = get_action(submenu, name);

        if (sub != NULL) {
            return sub;
        }
    }

    return NULL;
}

QAction* Gui_Manager::get_action(QMenu* parent, QString name)
{
    foreach (QAction* a, parent->actions()) {
        if (a->text().replace("&", "") == name) {
            return a;
        }

        QMenu* submenu = a->menu();

        if (submenu == NULL) {
            continue;
        }

        QAction* sub = get_action(submenu, name);

        if (sub != NULL) {
            return sub;
        }
    }

    return NULL;
}


void Gui_Manager::include_In_Layout(QDockWidget* widget, Window_Layout::Area area)
{
    widget->setWindowTitle(widget->windowTitle());
    include_In_Mainmenue(menu_view_list, widget->toggleViewAction());


    switch (area) {
    case Window_Layout::Left_Area:
        main_window->addDockWidget(Qt::LeftDockWidgetArea, widget);
        break;

    case Window_Layout::Right_Area:
        main_window->addDockWidget(Qt::RightDockWidgetArea, widget);
        break;

    case Window_Layout::Bottom_Area:
        main_window->addDockWidget(Qt::BottomDockWidgetArea, widget);
        break;

    }
}

bool Gui_Manager::removeFromLayout(QDockWidget* dockWidget)
{
    for (QDockWidget* dw : main_window->findChildren<QDockWidget*>()) {
        if (dw == dockWidget) {
            main_window->removeDockWidget(dockWidget);
            return true;
        }
    }

    return false;
}


void Gui_Manager::set_central_widget(QWidget* w)
{
    main_window->setCentralWidget(w);

}
void Gui_Manager::show_mainwindow()
{
    main_window->setGeometry(QStyle::alignedRect(
                                 Qt::LeftToRight,
                                 Qt::AlignCenter,
                                 main_window->size(),
                                 qApp->desktop()->availableGeometry()));
    main_window->showMaximized();
}


QWidget* Gui_Manager::get_widget_reference()
{
    return main_window;
}

void Gui_Manager::show_widget(QWidget* widget)
{
    widget->show();
}

void Gui_Manager::show_widget(QWidget* widget, QPoint position)
{
    widget->move(position);
    widget->show();
}

void Gui_Manager::show_dialog_modal(QDialog* dialog)
{
    dialog->exec();
}

void Gui_Manager::show_dialog_modal(QDialog* dialog, QPoint widget_position)
{
    dialog->move(widget_position);
    dialog->exec();
}
