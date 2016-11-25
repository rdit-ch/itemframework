#include <QToolBar>
#include <QToolButton>
#include "projectlist_dockwidget.h"
#include "ui_projectlist_dockwidget.h"
#include "abstract_project.h"
#include "project/project_manager.h"

ProjectListDockWidget::ProjectListDockWidget() : QDockWidget(), _ui(new Ui::ProjectListDockWidget)
{
    _ui->setupUi(this);
    _ui->listWidgetProjects->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_ui->listWidgetProjects, &QListWidget::customContextMenuRequested, this, &ProjectListDockWidget::customContextMenuRequested);
}

ProjectListDockWidget::~ProjectListDockWidget()
{
    delete _ui;
}

QVector<QSharedPointer<ProjectGui>> ProjectListDockWidget::projectGuis() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;

    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        QListWidgetItem* item = _ui->listWidgetProjects->item(i);
        projectGuis.append(item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
    }

    return projectGuis;
}

QVector<QSharedPointer<ProjectGui> > ProjectListDockWidget::dirtyProjects() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;

    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        QListWidgetItem* item = _ui->listWidgetProjects->item(i);
        QSharedPointer<ProjectGui> projectGui = item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();

        if (projectGui->project()->isDirty()) {
            projectGuis.append(projectGui);
        }

    }

    return projectGuis;
}

QVector<QListWidgetItem*> ProjectListDockWidget::listWidgetItems() const
{
    QVector<QListWidgetItem*> listWidgetItems;

    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        listWidgetItems.append(_ui->listWidgetProjects->item(i));
    }

    return listWidgetItems;
}

void ProjectListDockWidget::addProject(const QSharedPointer<ProjectGui>& projectGui)
{
    QListWidgetItem* item = new QListWidgetItem(projectGui->project()->name());
    item->setData(Qt::UserRole, QVariant::fromValue(projectGui));
    item->setToolTip(projectGui->project()->connectionString());
    _ui->listWidgetProjects->addItem(item);

}

void ProjectListDockWidget::addProjects(const QVector<QSharedPointer<ProjectGui> >& projectGuis)
{
    for (QSharedPointer<ProjectGui> projectGui : projectGuis) {
        addProject(projectGui);
    }
}

void ProjectListDockWidget::removeAllProjects()
{
    QSharedPointer<ProjectGui> project;

    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        project = _ui->listWidgetProjects->item(i)->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();
        disconnect(project.data());
        delete _ui->listWidgetProjects->takeItem(i);
    }

    _ui->listWidgetProjects->clear();
}

bool ProjectListDockWidget::removeProject(const QSharedPointer<ProjectGui>& projectGui)
{
    bool removed = false;
    QSharedPointer<ProjectGui> project;

    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        project = _ui->listWidgetProjects->item(i)->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();

        if (project == projectGui) {
            disconnect(project.data());
            delete _ui->listWidgetProjects->takeItem(i);
            removed = true;
            break;
        }
    }

    return removed;
}

bool ProjectListDockWidget::contains(const QSharedPointer<ProjectGui> &projectGui)
{
    for (int i = 0; i < _ui->listWidgetProjects->count(); ++i) {
        auto project = _ui->listWidgetProjects->item(i)->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();

        if (project == projectGui) {
            return true;
        }
    }

    return false;
}

void ProjectListDockWidget::customContextMenuRequested(const QPoint& position)
{
    QList<QListWidgetItem*> selectedItems = _ui->listWidgetProjects->selectedItems();
    QListWidgetItem* selectedItem = _ui->listWidgetProjects->itemAt(position);
    QPoint globalPosition = _ui->listWidgetProjects->mapToGlobal(position);

    // Set specific menue request (workspace = 0, project = 1, multi project = >1).
    int menuRequest = 0;
    if(selectedItem != nullptr){
        menuRequest = selectedItems.count();
    }

    switch (menuRequest) {
    case 0:
        // No Listwidget items are selected
        for(QListWidgetItem* item : _ui->listWidgetProjects->selectedItems()) {
            item->setSelected(false);
        }
        emit showWorkspaceContextMenu(globalPosition);
        break;
    case 1:
        // 1 Listwidget item is selected
        showProjectContextMenu(globalPosition, selectedItem->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
        break;
    default:
        // >1 Listwidget items are selected
        QList<QSharedPointer<ProjectGui>> projectGuis;
        for(QListWidgetItem* item : selectedItems){
            projectGuis.append(item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
        }
        emit showMultiProjectContextMenu(globalPosition, projectGuis);
        break;
    }
}

void ProjectListDockWidget::listWidgetClicked(QListWidgetItem* item)
{
    QSharedPointer<ProjectGui> projectGui = item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();
    emit loadProject(projectGui);
}

void ProjectListDockWidget::showProjectContextMenu(const QPoint& globalPosition, const QSharedPointer<ProjectGui>& projectGui)
{
    projectGui->showProjectContextMenue(globalPosition);
}
