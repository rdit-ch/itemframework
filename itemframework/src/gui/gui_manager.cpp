#include "gui/gui_manager.h"
#include "gui_main_window.h"
#include "helper/startup_helper.h"
#include "error/console_widget.h"
#include "error/console_model.h"
#include "project/project_manager_gui.h"
#include "gui/gui_plugin_manager.h"
#include "gui_manager_p.h"
#include "src/project/project_gui.h"
#include "src/project/project_manager.h"
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
    Gui_Main_Window* guiWindow = static_cast<Gui_Main_Window*>(d->_mainWindow);
    d->_mainWindow->installEventFilter(d);
    d->_mainWindow->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    d->_mainWindow->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    d->_mainWindow->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    d->_mainWindow->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
    ConsoleWidget* localConsoleWidget = new ConsoleWidget(ConsoleModel::instance());

    //localConsoleWidget->setVisible(false);
    if (!addWidget(localConsoleWidget, "Console", WidgetArea::Bottom, WidgetType::DockWidget)) {
        qWarning() << "failed to insert Console Widget";
        delete localConsoleWidget;
    }

    QToolBar* toolBar = guiWindow->toolBar();
    QStatusBar* statusBar = guiWindow->statusBar();
    QMenuBar* menuBar = guiWindow->menuBar();
    d->_widgets.insert("ToolBar", Widget(WidgetType::ToolBar, toolBar));

    d->_widgets.insert("StatusBar", Widget(WidgetType::StatusBar, statusBar));
    d->_widgets.insert("MenuBar", Widget(WidgetType::MenuBar, menuBar));

    for (QAction* action : toolBar->actions()) {
        d->registerAction(action, "ToolBar");
    }

    for (QAction* action : menuBar->actions()) {
        d->registerAction(action, "MenuBar");
    }

    addAction(toolBar->toggleViewAction(), "menuView");
    addAction(localConsoleWidget->toggleViewAction(), "menuView");

    // plugin manager
    d->_uiPluginManager = new GuiPluginManager(d->_mainWindow);

}

GuiManager::~GuiManager()
{
    Q_D(GuiManager);
    delete d->_uiPluginManager;
    delete d->_mainWindow;
}

bool GuiManager::postInit()
{
    Q_D(GuiManager);
    d->connectProjectManager();
    d->setupGui();
    d->_initialized = true;
    return true;
}

void GuiManager::setMode(GuiMode mode)
{
    Q_D(GuiManager);
    d->_mode = mode;
}

void GuiManagerPrivate::connectProjectManager()
{
    ProjectManager* projectManager = ProjectManagerGui::instance()->projectManager();
    connect(projectManager, &ProjectManager::workspaceChanged,
            this, &GuiManagerPrivate::loadMainWindowState);
}

void GuiManagerPrivate::setupGui()
{
    if (_mode == GuiMode::Headless) {
        QApplication::exit();
    }

    _mainWindow->setGeometry(QStyle::alignedRect(
                                    Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    _mainWindow->size(),
                                    qApp->desktop()->availableGeometry()));

    _mainWindow->show();
    _mainWindow->setDisabled(true);

    if (!ProjectManagerGui::instance()->start()) {
        _mainWindow->close();
    }

    _mainWindow->setDisabled(false);

    loadMainWindowState();
}

bool GuiManager::preDestroy()
{
    Q_D(GuiManager);
    d->saveState();
    return true;
}

bool GuiManagerPrivate::eventFilter(QObject* o, QEvent* e)
{
    Q_Q(GuiManager);

    if (o == _mainWindow && e->type() == QEvent::Close) {
        QCloseEvent* ce = static_cast<QCloseEvent*>(e);

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

    }

    if (o == _mainWindow && e->type() == QEvent::ActivationChange) {
        emit q->mainWindowActivationChange();
    }

    // Execute event and return success state
    return QObject::eventFilter(o, e);
}

void GuiManagerPrivate::saveState()
{
    if (_initialized) {
        ProjectManager* projectManager = ProjectManagerGui::instance()->projectManager();

        if (projectManager &&
                projectManager->currentWorkspace() &&
                projectManager->currentWorkspace()->settingsScope()) {
            projectManager->currentWorkspace()->settingsScope()->setValue("GuiWindowLayout", _mainWindow->saveState());
        }
    }
}

void GuiManagerPrivate::loadMainWindowState()
{
    ProjectManager* projectManager = ProjectManagerGui::instance()->projectManager();

    if (projectManager &&
            projectManager->currentWorkspace().data() &&
            projectManager->currentWorkspace()->settingsScope()) {
        QVariant layout = projectManager->currentWorkspace()->settingsScope()->value("GuiWindowLayout");

        if (layout.isValid()) {
            _mainWindow->restoreState(layout.toByteArray());
        }
    }
}

void GuiManager::registerCloseHandler(std::function<bool()> callback)
{
    Q_D(GuiManager);
    d->_closeApplicationCallbacks.append(callback);
}

QStringList GuiManager::parents(QString const& name)
{
    Q_D(const GuiManager);
    return d->_actions.value(name).parents;
}

QStringList GuiManager::children(QString const& name) const
{
    Q_D(const GuiManager);
    return d->_widgets.value(name).children + d->_actions.value(name).children;
}

QStringList GuiManager::registeredActions() const
{
    Q_D(const GuiManager);
    return d->_actions.keys();
}

QStringList GuiManager::registeredWidgets() const
{
    Q_D(const GuiManager);
    return d->_widgets.keys();
}

QStringList GuiManager::registeredWidgets(WidgetType type) const
{
    Q_D(const GuiManager);
    QStringList names;

    for (auto name : d->_widgets.keys()) {
        if (d->_widgets.value(name).type == type) {
            names.append(name);
        }
    }

    return names;
}

bool GuiManager::mainWindowIsActive() const
{
    Q_D(const GuiManager);
    return d->_mainWindow->isActiveWindow();
}

bool GuiManager::addAction(QAction* action, QString const& name, QString const& parent)
{
    Q_D(GuiManager);

    if (!d->validParent(parent, action) || action == NULL) {
        return false;
    }

    //name has to be free or action must be the same as name
    if (d->nameUsed(name) && !(d->_actions.value(name).action == action)) {
        return false;
    }

    if (d->_widgets.contains(parent)) {
        if (action->menu()) {
            qobject_cast<QMenuBar*>(d->_widgets.value(parent).widget.data())->addMenu(action->menu());
        } else  {
            d->_widgets.value(parent).widget->addAction(action);
        }

        d->_widgets[parent].children.append(name);
    } else {
        d->_actions.value(parent).action->menu()->addAction(action);
        d->_actions[parent].children.append(name);
    }

    if (d->_actions.contains(name)) {
        d->_actions[name].parents.append(parent);
    } else {
        d->_actions.insert(name, Action(parent, action));
    }

    return true;
}

bool GuiManager::addActionToParent(QString const& name, QString const& parent)
{
    Q_D(GuiManager);

    QAction* action = d->_actions.value(name).action;

    //parent needs to ba a valid parent to name
    if (!d->validParent(parent, action)) {
        return false;
    }

    if (d->_widgets.contains(parent)) {
        if (action->menu()) {
            qobject_cast<QMenuBar*>(d->_widgets.value(parent).widget.data())->addMenu(action->menu());
        } else  {
            d->_widgets.value(parent).widget->addAction(action);
        }

        d->_widgets[parent].children.append(name);
    } else {
        d->_actions.value(parent).action->menu()->addAction(action);
        d->_actions[parent].children.append(name);
    }

    d->_actions[name].parents.append(parent);
    return true;
}

QString GuiManager::addAction(QAction* action, QString const& parent)
{
    Q_D(GuiManager);

    //Check if action can be added
    if (!d->validParent(parent, action) || action == NULL) {
        return QString();
    }

    QString name = action->objectName();

    if (name.isEmpty()) {
        name = QString(action->metaObject()->className());
    }

    if (addAction(action, name, parent)) {
        return name;
    } else {
        int i = 1;

        //iterate until a custom name is found
        while (!addAction(action, name + QString("_%1").arg(i), parent)) {
            i++;
        }

        return name + QString("_%1").arg(i);
    }
}

QAction* GuiManager::action(QString const& name) const
{
    Q_D(const GuiManager);
    return d->_actions.value(name).action;
}

QAction* GuiManager::removeAction(QString const& name, QString const& parent)
{
    Q_D(GuiManager);
    QStringList parents;

    if (d->_actions.contains(name)) {
        return NULL;
    }

    //Determine from which parents the action should be removed
    if (parents.isEmpty()) {
        parents = d->_actions[name].parents;
    } else if (d->nameUsed(parent)) {
        parents.append(parent);
    } else {
        return NULL;
    }

    //remove relationship with parent(s)
    for (QString const& parentName : parents) {
        if (d->_widgets.contains(parentName)) {
            d->_widgets[parentName].children.removeAll(name);
        }

        if (d->_actions.contains(parentName)) {
            d->_actions[parentName].children.removeAll(name);
        }

        d->_actions[name].parents.removeAll(parentName);
    }

    QAction* action = d->_actions[name].action;

    //Remove action completly including children if it is without a parent
    if (d->_actions.value(name).parents.isEmpty()) {
        for (QString const& child : d->_actions.value(name).children +  d->_widgets.value(name).children) {
            d->unregisterAction(child, name);
        }

        d->_actions.remove(name);
    }

    //remove the action from the menu or widget.
    if (d->_actions.contains(parent)) {
        d->_actions[parent].action->menu()->removeAction(action);
    } else if (d->_widgets.contains(parent)) {
        d->_widgets[parent].widget->removeAction(action);
    }

    return action;
}
bool GuiManagerPrivate::isType(QWidget* widget, WidgetType type, QString const& name)
{
    switch (type) {
    case WidgetType::DockWidget:
        if (qobject_cast<QDockWidget*>(widget) == NULL) {
            qWarning() << name << " can't be casted to a QDockWidget";
            return false;
        }

        break;

    case WidgetType::ToolBar:
        if (qobject_cast<QToolBar*>(widget) == NULL) {
            qWarning() << name << " can't be casted to a QToolBar";
            return false;
        }

        break;

    case WidgetType::MenuBar:

        if (qobject_cast<QMenuBar*>(widget) == NULL) {
            qWarning() << name << " can't be casted to a QMenuBar";
            return false;
        }

        break;

    case WidgetType::StatusBar:
        if (qobject_cast<QStatusBar*>(widget) == NULL) {
            qWarning() << name << " can't be casted to a QStatusBar";
            return false;
        }

        break;

    case WidgetType::CentralWidget:
        break;

    }

    return true;
}

bool GuiManagerPrivate::nameUsed(QString const& name)
{
    bool result = _widgets.contains(name) || _actions.contains(name);
    return result;
}


bool GuiManagerPrivate::validParent(QString const& name, QAction* action)
{
    if (action && ((_actions.contains(name) && _actions.value(name).action->menu()) ||
                   (action->menu() && _widgets.value(name).type == WidgetType::MenuBar) ||
                   (!action->menu() && _widgets.value(name).type == WidgetType::ToolBar))
       ) {
        return true;
    } else {
        return false;
    }
}

QString GuiManagerPrivate::registerAction(QAction* action, QString const& parent, bool recursive)
{
    if (!action || !validParent(parent, action)) {
        return QString();
    }

    QString name = action->objectName();

    if (action->menu()) {
        name = action->menu()->objectName();
    }

    if (name.isEmpty()) {
        name = QString(action->metaObject()->className());
    }

    if (nameUsed(name) && _actions.value(name).action != action) {
        int i = 1;
        QString tmpName = name + QString("_%1").arg(i);

        //iterate until a custom name is found
        while (nameUsed(tmpName) && _actions.value(tmpName).action != action) {
            i++;
            tmpName = name + QString("_%1").arg(i);
        }

        name = tmpName;
    }

    if (_actions.contains(name)) {
        _actions[name].parents.append(parent);
    } else {
        _actions.insert(name, Action(parent, action));
    }

    if (_actions.contains(parent)) {
        _actions[parent].children.append(name);
    } else {
        _widgets[parent].children.append(name);
    }

    if (recursive && action->menu()) {
        for (QAction* subAction : action->menu()->actions()) {
            registerAction(subAction, name);
        }
    }

    return name;
}

void GuiManagerPrivate::unregisterAction(QString const& name, QString const& parent)
{
    if (_actions.contains(parent)) {
        _actions[parent].children.removeAll(name);
    }

    if (_actions.contains(name)) {
        _actions[name].parents.removeAll(parent);
    }

    if (_actions.value(name).parents.isEmpty()) {
        for (QString const& action : _actions.value(name).children) {
            unregisterAction(action, name);
        }

        _actions.remove(name);
    }
}

void GuiManagerPrivate::unregisterActions(QStringList const& parents)
{
    Q_Q(GuiManager);

    for (auto parent : parents) {
        for (QString const& child : _widgets.take(parent).children) {
            q->removeAction(child, parent);
        }
    }
}

bool GuiManager::addWidget(QWidget* widget, QString const& name, WidgetArea area, WidgetType type)
{
    Q_D(GuiManager);

    if (d->nameUsed(name) ||
            widget == NULL ||
            !GuiManagerPrivate::isType(widget, type, name)) {
        return false;
    }

    widget->setParent(d->_mainWindow);

    switch (type) {
    case WidgetType::DockWidget: {

        QDockWidget* dockWidget = qobject_cast<QDockWidget*>(widget);
        addAction(dockWidget->toggleViewAction(), "menuView");
        d->_mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area), dockWidget);
        connect(dockWidget, &QDockWidget::dockLocationChanged, d, &GuiManagerPrivate::saveState);
        connect(dockWidget, &QDockWidget::visibilityChanged, d, &GuiManagerPrivate::saveState);
        break;
    }

    case WidgetType::ToolBar: {
        QToolBar* toolBar = qobject_cast<QToolBar*>(widget);

        for (QAction* action : toolBar->actions()) {
            d->registerAction(action, name);
        }

        connect(toolBar, &QToolBar::topLevelChanged, d, &GuiManagerPrivate::saveState);
        connect(toolBar, &QToolBar::visibilityChanged, d, &GuiManagerPrivate::saveState);
        addAction(toolBar->toggleViewAction(), "menuView");
        d->_mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area), toolBar);
        break;
    }

    case WidgetType::CentralWidget:
        d->unregisterActions(registeredWidgets(WidgetType::CentralWidget));
        d->_mainWindow->setCentralWidget(widget);
        break;

    case WidgetType::MenuBar:
        for (QAction* action : qobject_cast<QMenuBar*>(widget)->actions()) {
            d->registerAction(action, name);
        }

        d->unregisterActions(registeredWidgets(WidgetType::MenuBar));
        d->_mainWindow->setMenuBar(qobject_cast<QMenuBar*>(widget));
        break;

    case WidgetType::StatusBar:
        d->_mainWindow->setStatusBar(qobject_cast<QStatusBar*>(widget));
        d->unregisterActions(registeredWidgets(WidgetType::StatusBar));
        break;
    }

    d->_widgets.insert(name, Widget(type, widget));
    return true;
}

QString GuiManager::addWidget(QWidget* widget, WidgetArea area, WidgetType type)
{
    //Check if widget can be added
    if (widget == NULL ||
            !GuiManagerPrivate::isType(widget, type, QString("widget"))) {
        return QString();
    }

    QString name = widget->objectName();

    if (name.isEmpty()) {
        name = QString(widget->metaObject()->className());
    }

    if (addWidget(widget, name, area, type)) {
        return name;
    } else {
        int i = 1;

        //iterate until a custom name is found
        while (!addWidget(widget, name + QString("_%1").arg(i), area, type)) {
            i++;
        }

        return name + QString("_%1").arg(i);
    }
}

bool GuiManager::moveWidget(QString const& name, WidgetArea area)
{
    Q_D(GuiManager);

    if (!d->_widgets.contains(name)) {
        return false;
    }

    QWidget* widget = d->_widgets[name].widget;
    WidgetType type = d->_widgets[name].type;

    switch (type) {
    case WidgetType::DockWidget:
        d->_mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(area),
                                      qobject_cast<QDockWidget*>(widget));
        break;

    case WidgetType::ToolBar:
        d->_mainWindow->addToolBar(static_cast<Qt::ToolBarArea>(area),
                                   qobject_cast<QToolBar*>(widget));
        break;

    default:
        return false;
    }

    return true;
}

void GuiManager::setVisible(QString const& name, bool visible)
{
    Q_D(GuiManager);
    d->_widgets[name].widget->setVisible(visible);
}

QWidget* GuiManager::removeWidget(QString const& name)
{
    Q_D(GuiManager);
    QWidget* widget = d->_widgets.value(name).widget;
    WidgetType type = d->_widgets.value(name).type;
    QStringList children = d->_widgets.take(name).children;

    if (!widget) {
        return NULL;
    }

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
        d->_mainWindow->removeDockWidget(qobject_cast<QDockWidget*>(widget));
        disconnect(qobject_cast<QDockWidget*>(widget), NULL, this, NULL);
        break;

    case WidgetType::ToolBar:
        d->_mainWindow->removeToolBar(qobject_cast<QToolBar*>(widget));
        disconnect(qobject_cast<QToolBar*>(widget), NULL, this, NULL);
        break;
    }

    //if widget was removed, remove it's children also from the hash
    if (widget) {
        for (QString const& child : children) {
            d->unregisterAction(child, name);
        }
    }

    return widget;
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
