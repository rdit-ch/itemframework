#ifndef PROJECT_LIST_DOCK_WIDGET_H
#define PROJECT_LIST_DOCK_WIDGET_H

#include <QtWidgets/QDockWidget>
#include <QPoint>
#include <QListWidget>

#include "gui/gui_manager.h"
#include "project_gui.h"

namespace Ui
{
class ProjectListDockWidget;
}

class ProjectListDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ProjectListDockWidget();
    ~ProjectListDockWidget();
    QVector<QSharedPointer<ProjectGui>> projectGuis() const;
    QVector<QSharedPointer<ProjectGui>> dirtyProjects() const;
    QVector<QListWidgetItem*> listWidgetItems() const;
    void addProject(const QSharedPointer<ProjectGui>& projectGui);
    void addProjects(const QVector<QSharedPointer<ProjectGui>>& projectGui);
    void removeAllProjects();
    bool removeProject(const QSharedPointer<ProjectGui>& projectGui);
    bool contains(const QSharedPointer<ProjectGui>& projectGui);

private slots:
    void customContextMenuRequested(const QPoint& position);
    void listWidgetClicked(QListWidgetItem* item);

signals:
    void loadProject(QSharedPointer<ProjectGui> projectGui);

private:
    Ui::ProjectListDockWidget* _ui;
    void showProjectContextMenu(const QPoint& globalPosition, const QSharedPointer<ProjectGui> &projectGui);

signals:
    void showWorkspaceContextMenu(const QPoint& position);
    void showMultiProjectContextMenu(const QPoint& position, const QList<QSharedPointer<ProjectGui>> projectGuis);
};

#endif // PROJECT_LIST_DOCK_WIDGET_H
