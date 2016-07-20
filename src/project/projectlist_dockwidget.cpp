#include <QToolBar>
#include <QToolButton>
#include "projectlist_dockwidget.h"
#include "ui_projectlist_dockwidget.h"
#include "abstract_project.h"
#include "project/project_manager.h"

ProjectListDockWidget::ProjectListDockWidget() : QDockWidget(), ui(new Ui::ProjectListDockWidget)
{
    ui->setupUi(this);
    ui->listWidgetProjects->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidgetProjects, &QListWidget::customContextMenuRequested, this, &ProjectListDockWidget::customContextMenuRequested);
}

ProjectListDockWidget::~ProjectListDockWidget()
{
    delete ui;
}

QVector<QSharedPointer<ProjectGui>> ProjectListDockWidget::projectGuis() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;

    for (int i = 0; i < ui->listWidgetProjects->count(); ++i) {
        QListWidgetItem* item = ui->listWidgetProjects->item(i);
        projectGuis.append(item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
    }

    return projectGuis;
}

QVector<QSharedPointer<ProjectGui> > ProjectListDockWidget::dirtyProjects() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;

    for (int i = 0; i < ui->listWidgetProjects->count(); ++i) {
        QListWidgetItem* item = ui->listWidgetProjects->item(i);
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

    for (int i = 0; i < ui->listWidgetProjects->count(); ++i) {
        listWidgetItems.append(ui->listWidgetProjects->item(i));
    }

    return listWidgetItems;
}

void ProjectListDockWidget::addProject(const QSharedPointer<ProjectGui>& projectGui)
{
    QListWidgetItem* item = new QListWidgetItem(projectGui->project()->name());
    item->setData(Qt::UserRole, QVariant::fromValue(projectGui));
    item->setToolTip(projectGui->project()->connectionString());
    ui->listWidgetProjects->addItem(item);

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

    for (int i = 0; i < ui->listWidgetProjects->count(); ++i) {
        project = ui->listWidgetProjects->item(i)->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();
        disconnect(project.data());
        delete ui->listWidgetProjects->takeItem(i);
    }

    ui->listWidgetProjects->clear();
}

bool ProjectListDockWidget::removeProject(const QSharedPointer<ProjectGui>& projectGui)
{
    bool removed = false;
    QSharedPointer<ProjectGui> project;

    for (int i = 0; i < ui->listWidgetProjects->count(); ++i) {
        project = ui->listWidgetProjects->item(i)->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();

        if (project == projectGui) {
            disconnect(project.data());
            delete ui->listWidgetProjects->takeItem(i);
            removed = true;
            break;
        }
    }

    return removed;
}

void ProjectListDockWidget::customContextMenuRequested(const QPoint& clickPosition)
{
    // Set listwidget item on clicked position
    QListWidgetItem* selectedItem = ui->listWidgetProjects->itemAt(clickPosition);
    QPoint globalClickPosition = ui->listWidgetProjects->mapToGlobal(clickPosition);

    // Check if a listwidget item is selected if not show the workspace context menue
    if (selectedItem != nullptr) {
        // Get project from QVariant Data property - Qt::UserRole
        const QSharedPointer<ProjectGui> project = selectedItem->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();
        // Show project context menue
        showProjectContextMenu(globalClickPosition, project);
    } else {
        emit showWorkspaceContextMenu(globalClickPosition);
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
