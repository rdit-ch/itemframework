#include "sql_workspace_gui.h"

SqlWorkspaceGui::SqlWorkspaceGui()
{
}

void SqlWorkspaceGui::addListWidgetItem(QListWidget* parentListWidget)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem(parentListWidget);
    listWidgetItem->setText(QStringLiteral("Database SQL"));
    listWidgetItem->setIcon(QIcon(":/core/projectmanager/database"));
}

QString SqlWorkspaceGui::workspaceTypeName() const
{
    return QStringLiteral("Sql Workspace");
}

QDialog* SqlWorkspaceGui::dialogNewWorkspace(QDialog* parent) const
{
    Q_UNUSED(parent);
    return new QDialog();
}

QDialog* SqlWorkspaceGui::dialogLoadWorkspace(QDialog* parent) const
{
    Q_UNUSED(parent);
    return new QDialog();
}

QDialog* SqlWorkspaceGui::dialogEditWorkspace(QDialog* parent, QSharedPointer<AbstractWorkspace> workspace) const
{
    Q_UNUSED(parent);
    Q_UNUSED(workspace);
    return new QDialog();
}

bool SqlWorkspaceGui::isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const
{
    QSharedPointer<SqlWorkspace> sqlWorkspace = qSharedPointerCast<SqlWorkspace>(workspace);

    if (sqlWorkspace.isNull()) {
        return false;
    }

    return true;
}

bool SqlWorkspaceGui::removeProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox)
{
    Q_UNUSED(projectGui);
    Q_UNUSED(showMessagebox);
    return true;
}

bool SqlWorkspaceGui::deleteProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox)
{
    Q_UNUSED(projectGui);
    Q_UNUSED(showMessagebox);
    return true;
}

bool SqlWorkspaceGui::saveExternChangedProjects()
{
    return true;
}

void SqlWorkspaceGui::resetExternChangedProjects()
{

}

void SqlWorkspaceGui::editProject(QSharedPointer<ProjectGui> projectGui)
{
    Q_UNUSED(projectGui);
}

void SqlWorkspaceGui::showChangeSourceDialog(const QSharedPointer<AbstractWorkspace> &workspace)
{
    Q_UNUSED(workspace);
}

bool SqlWorkspaceGui::exportToFileSystem(QString exportPath, ExportOptions options, QVector<QSharedPointer<AbstractProject>> projects)
{
    Q_UNUSED(exportPath);
    Q_UNUSED(options);
    Q_UNUSED(projects);
    return true;
}

void SqlWorkspaceGui::searchProjectSource(QSharedPointer<ProjectGui> projectGui)
{
    Q_UNUSED(projectGui);
}

void SqlWorkspaceGui::createProjectsFromImport(const QStringList &projectPaths, bool overwrite)
{
    Q_UNUSED(projectPaths);
    Q_UNUSED(overwrite);
}
