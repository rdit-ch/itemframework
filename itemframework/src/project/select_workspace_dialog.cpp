#include "select_workspace_dialog.h"
#include "ui_select_workspace_dialog.h"
#include "file_workspace_gui.h"
#include "sql_workspace_gui.h"
#include "plugin/plugin_manager.h"
#include "project_manager_gui.h"
#include "project_manager_config.h"
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QCheckBox>

SelectWorkspaceDialog::SelectWorkspaceDialog(ProjectManager* projectManager) : QDialog(), _ui(new Ui::SelectWorkspaceDialog)
{
    // Setup UI
    _ui->setupUi(this);

    // Set project manager
    _projectManager = projectManager;

    // Set the recent used workspaces and prepare the treewidget.
    setRecentUsedWorkspaces();

    // Connect UI elements
    _ui->treeWidgetRecentWorkspace->hideColumn(TreeWidgetColumn::Workspace);
    _ui->treeWidgetRecentWorkspace->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_ui->treeWidgetRecentWorkspace, &QTreeWidget::customContextMenuRequested, this, &SelectWorkspaceDialog::customContextMenuRequested);
    connect(_ui->treeWidgetRecentWorkspace, &QTreeWidget::itemClicked, this, &SelectWorkspaceDialog::recentWorkspaceSelected);
    connect(_ui->treeWidgetRecentWorkspace, &QTreeWidget::itemDoubleClicked, this, &SelectWorkspaceDialog::loadRecentUsedWorkspace);
    connect(_ui->treeWidgetRecentWorkspace, &QTreeWidget::itemSelectionChanged, this, &SelectWorkspaceDialog::recentUsedWorkspaceSelectionChanged);
}

SelectWorkspaceDialog::~SelectWorkspaceDialog()
{
     // Disconnect workspace change signals
    for (const auto workspace : _projectManager->recentWorkspaces()) {
        disconnect(workspace.data(), 0, this, 0);
    }

    // Clear QTreeWidget
    _ui->treeWidgetRecentWorkspace->clear();

    // Delete SelectWorkspaceDialog UI
    delete _ui;
}

void SelectWorkspaceDialog::setRecentUsedWorkspaces()
{
    for (const auto workspace : _projectManager->recentWorkspaces()) {

        // Disconnect signal to prevent workspace change loop
        disconnect(workspace.data(), &AbstractWorkspace::workspaceUpdated, this, &SelectWorkspaceDialog::onWorkspaceUpdated);

        QTreeWidgetItem* item = nullptr;

        // Find QTreeWidgetItem which matches to this workspace
        for(QTreeWidgetItem* topLevelItem : treeWidgetItems()){
            if(workspace->compare(topLevelItem->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>())){
                item = topLevelItem;
                break;
            }
        }

        // If no QTreeWidgetItem was found create a new one
        if(item == nullptr){
            item = new QTreeWidgetItem(_ui->treeWidgetRecentWorkspace);
            item->setData(TreeWidgetColumn::Workspace, Qt::UserRole, QVariant::fromValue(workspace));
        }

        // Set QTreeWidgetItem properties
        item->setText(TreeWidgetColumn::LastUsed, workspace->lastUsedDateTime());
        item->setText(TreeWidgetColumn::Name, workspace->name());
        item->setText(TreeWidgetColumn::Type, workspace->typeString());
        item->setText(TreeWidgetColumn::Connection, workspace->connectionString());
        item->setToolTip(TreeWidgetColumn::Name, workspace->description());

        // Show an error icon (connection string) if workspace fail the test
        if (!workspace->test()) {
            item->setIcon(TreeWidgetColumn::Connection, QIcon(":/core/projectmanager/workspace_not_valid.png"));
            item->setToolTip(TreeWidgetColumn::Connection, workspace->lastError());
        } else if(!item->icon(TreeWidgetColumn::Connection).isNull()){
            // Remove the error icon (connection string) if workspace had an error and pass now the test.
            item->setIcon(TreeWidgetColumn::Connection, QIcon());
            item->setToolTip(TreeWidgetColumn::Connection, QString(""));
        }

        // Check if workspace is default property is set
        if (workspace->isDefault()) {
            item->setText(TreeWidgetColumn::Default, QString("x"));
            item->setTextAlignment(TreeWidgetColumn::Default, Qt::AlignHCenter);
        }

        // Check if workspace is current workspace
        const auto currentWorkspace = _projectManager->currentWorkspace();
        if (!currentWorkspace.isNull() && currentWorkspace->compare(workspace)) {
            setSelectedWorkspace(workspace);
            item->setText(TreeWidgetColumn::Open, QString("x"));
            item->setTextAlignment(TreeWidgetColumn::Open, Qt::AlignHCenter);
            _ui->treeWidgetRecentWorkspace->setCurrentItem(item);
        }

        // Connect workspace change signal
        connect(workspace.data(), &AbstractWorkspace::workspaceUpdated, this, &SelectWorkspaceDialog::onWorkspaceUpdated);
    }

    // Config QTreeWidget (recent used workspace list) layout settings
    _ui->treeWidgetRecentWorkspace->header()->resizeSection(TreeWidgetColumn::Open, 55);
    _ui->treeWidgetRecentWorkspace->header()->resizeSection(TreeWidgetColumn::Default, 55);
    _ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::LastUsed);
    _ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Name);
    _ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Type);
    _ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Connection);
    _ui->treeWidgetRecentWorkspace->sortByColumn(TreeWidgetColumn::LastUsed, Qt::DescendingOrder);
}

QSharedPointer<AbstractWorkspace> SelectWorkspaceDialog::selectedWorkspace() const
{
    return _selectedWorkspace;
}

QDialog* SelectWorkspaceDialog::createDialogWorkspaceGui(const WorkspaceGuiType workspaceGuiType,
        QVector<AbstractWorkspaceGui*> const workspaceGuiVector)
{
    // Create a new QDialog to provide a workspace UI interaction
    QDialog* dialogWorkspaceGui = new QDialog(this);
    dialogWorkspaceGui->setFixedSize(680, 400);

    // Set default gridlayout
    QGridLayout* layoutWorkspaceDialog = new QGridLayout(dialogWorkspaceGui);

    // QListWidget listWidgetWorkspaceType selects the workspace type
    QListWidget* listWidgetWorkspaceType = new QListWidget();
    listWidgetWorkspaceType->setMaximumWidth(150);
    // Header of current workspace type
    QLabel* labelHeaderWorkspaceType = new QLabel();
    QFont font;
    font.setBold(true);
    font.setPixelSize(14);
    labelHeaderWorkspaceType->setFont(font);

    // The QStackedWidget holds all available (different workspace types - file,sql) workspaceWidget UIs
    QStackedWidget* stackWidgetWorkspaceGui = new QStackedWidget();
    // Setup QGridLayout
    layoutWorkspaceDialog->addWidget(listWidgetWorkspaceType, 0, 0, 2, 1);
    layoutWorkspaceDialog->addWidget(labelHeaderWorkspaceType, 0, 1, 1, 1);
    layoutWorkspaceDialog->addWidget(stackWidgetWorkspaceGui, 1, 1, 1, 1);
    layoutWorkspaceDialog->setColumnMinimumWidth(0, 160);

    // Connect listWidgetWorkspaceType with loadWorkspaceGuiStackWidget to control
    // QStackedWidget over QListWidget selection index
    connect(listWidgetWorkspaceType, &QListWidget::currentRowChanged, stackWidgetWorkspaceGui, &QStackedWidget::setCurrentIndex);
    connect(listWidgetWorkspaceType, &QListWidget::currentTextChanged, labelHeaderWorkspaceType, &QLabel::setText);

    // Add all WorkspaceGui Widgets to QStackedWidget
    for (AbstractWorkspaceGui* abstractWorkspaceGui : workspaceGuiVector) {
        // Function to grep the required workspace UI depends on workspaceGuiType
        switch (workspaceGuiType) {
        case WorkspaceGuiType::newWorkspace:
            // Add workspace widget which is dedicated to create a new workspace
            stackWidgetWorkspaceGui->addWidget(abstractWorkspaceGui->dialogNewWorkspace(dialogWorkspaceGui));
            break;

        case WorkspaceGuiType::loadWorkspace:
            // Add workspace widget which is dedicated to load a workspace
            stackWidgetWorkspaceGui->addWidget(abstractWorkspaceGui->dialogLoadWorkspace(dialogWorkspaceGui));
            break;

        case WorkspaceGuiType::editWorkspace:
            // Add workspace widget which is dedicated to load a workspace
            stackWidgetWorkspaceGui->addWidget(abstractWorkspaceGui->dialogEditWorkspace(dialogWorkspaceGui));
            break;

        default:
            qWarning() << QString("WorkspaceGuiType enum %1 not handled.").arg(workspaceGuiType);
            break;
        }

        // Add WorkspaceGui listwidgetitem to listWidgetWorkspaceType
        abstractWorkspaceGui->addListWidgetItem(listWidgetWorkspaceType);
        // Connect QDialog accept an reject signals
        // Send created workspace to workspaceAccepted(QSharedPointer<AbstractWorkspace> workspace) function
        connect(abstractWorkspaceGui, &AbstractWorkspaceGui::acceptWorkspace, this, &SelectWorkspaceDialog::acceptWorkspace);
    }

    if (stackWidgetWorkspaceGui->count() > 0) {
        stackWidgetWorkspaceGui->setCurrentIndex(0);
    }

    // Activate first entry
    if (listWidgetWorkspaceType->count() > 0) {
        listWidgetWorkspaceType->setCurrentRow(0);
    }

    // Return final QDialog
    return dialogWorkspaceGui;
}

QVector<QTreeWidgetItem *> SelectWorkspaceDialog::treeWidgetItems() const
{
    QVector<QTreeWidgetItem*> items;

    for (int i = 0; i < _ui->treeWidgetRecentWorkspace->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = _ui->treeWidgetRecentWorkspace->topLevelItem(i);
        if (item != nullptr) {
            items.append(item);
        }
    }

    return items;
}

void SelectWorkspaceDialog::customContextMenuRequested(const QPoint& position)
{
    QTreeWidgetItem* item = _ui->treeWidgetRecentWorkspace->itemAt(position);

    if (item != nullptr) {
        auto workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

        if (!workspace.isNull()) {
            QPoint menuePosition = _ui->treeWidgetRecentWorkspace->mapToGlobal(position);
            menuePosition.setY(menuePosition.y() + 15);
            showContextMenu(menuePosition, workspace);
        }
    }
}

void SelectWorkspaceDialog::showContextMenu(const QPoint& position, const QSharedPointer<AbstractWorkspace>& workspace)
{
    QMenu menu;
    QAction* actionSetDefault = menu.addAction(QLatin1String("Set as default Workspace"));
    menu.addAction(menu.addSeparator());
    QAction* actionEditWorkspace = menu.addAction(QString("Edit '%1'").arg(workspace->name()));
    QAction* actionDeleteWorkspace = menu.addAction(QString("Delete '%1'").arg(workspace->name()));
    menu.addAction(menu.addSeparator());
    QAction* actionRemoveFromRecentList = menu.addAction(QString("Remove '%1' from List").arg(workspace->name()));
    actionSetDefault->setCheckable(true);
    actionSetDefault->setChecked(workspace->isDefault());
    int selectedItemCount = _ui->treeWidgetRecentWorkspace->selectedItems().count();

    if (selectedItemCount > 1) {
        actionSetDefault->setEnabled(false);
        actionSetDefault->setChecked(false);
        actionEditWorkspace->setEnabled(false);
        actionEditWorkspace->setText(QLatin1String("Edit"));
        actionDeleteWorkspace->setText(QString("Delete all selected."));
        actionRemoveFromRecentList->setText(QString("Remove all selected from List."));
    }

    if(workspace->isValid()){
        connect(actionEditWorkspace, &QAction::triggered, [this, workspace](){
            AbstractWorkspaceGui* workspaceGui = ProjectManagerGui::abstractWorkspaceGuiClass(workspace, _workspaceGuiVector);
            QDialog* editWorkspaceDialog = workspaceGui->dialogEditWorkspace(0, workspace);
            editWorkspaceDialog->exec();
        });
        connect(actionDeleteWorkspace, &QAction::triggered, this, &SelectWorkspaceDialog::deleteWorkspace);
        connect(actionSetDefault, &QAction::toggled, this, &SelectWorkspaceDialog::defaultWorkspaceChange);
    } else {
        actionSetDefault->setEnabled(false);
        actionDeleteWorkspace->setEnabled(false);
        actionSetDefault->setChecked(false);
        actionEditWorkspace->setEnabled(false);

        menu.addAction(menu.addSeparator());
        QAction* actionSearchWorkspace = menu.addAction(QString("Search '%1'...").arg(workspace->name()));
        connect(actionSearchWorkspace, &QAction::triggered, this, &SelectWorkspaceDialog::searchRecentUsedWorkspaces);
    }

    connect(actionRemoveFromRecentList, &QAction::triggered, this, &SelectWorkspaceDialog::removeFromRecentList);

    menu.exec(position);
}

void SelectWorkspaceDialog::acceptWorkspace(QSharedPointer<AbstractWorkspace> workspace)
{
    // Add current workspace into recent used workspace vector.
    _projectManager->addRecentWorkspace(workspace);
    // Set selected workspace and save it
    setSelectedWorkspace(workspace);

    if (workspace->isDefault()) {
        defaultWorkspaceChange(true);
    }

    accept();
}

void SelectWorkspaceDialog::recentWorkspaceSelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    setSelectedWorkspace(item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>());
}

void SelectWorkspaceDialog::loadRecentUsedWorkspace(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if(item == nullptr){
        return;
    }

    auto workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

    if (!workspace.isNull()) {
        if(!workspace->test()){
            QMessageBox::warning(this, tr("Load Workspace."),
                                 tr("Workspace is not valid.\n\n%1").arg(workspace->lastError()),
                                 QMessageBox::Ok);

            item->setIcon(TreeWidgetColumn::Connection, QIcon(":/core/projectmanager/workspace_not_valid.png"));
            item->setToolTip(TreeWidgetColumn::Connection, workspace->lastError());
            return;
        }

        if(!item->icon(TreeWidgetColumn::Connection).isNull()){
            item->setIcon(TreeWidgetColumn::Connection, QIcon());
            item->setToolTip(TreeWidgetColumn::Connection, QString(""));
        }

        setSelectedWorkspace(workspace);
        accept();
    }
}

void SelectWorkspaceDialog::setSelectedWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    _selectedWorkspace = workspace;
    _ui->buttonAcceptDialog->setEnabled(true);
}

void SelectWorkspaceDialog::clearSelectedWorkspace()
{
    _selectedWorkspace.clear();
    _ui->buttonAcceptDialog->setEnabled(false);
}

QVector<AbstractWorkspaceGui*> SelectWorkspaceDialog::workspaceGuiVector() const
{
    return _workspaceGuiVector;
}

void SelectWorkspaceDialog::setWorkspaceGuiVector(const QVector<AbstractWorkspaceGui*>& workspaceGuiVector)
{
    _workspaceGuiVector = workspaceGuiVector;
}

void SelectWorkspaceDialog::recentUsedWorkspaceSelectionChanged()
{
    const QList<QTreeWidgetItem*> items = _ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        setSelectedWorkspace(items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>());
        return;
    }

    clearSelectedWorkspace();
}

void SelectWorkspaceDialog::defaultWorkspaceChange(bool checked)
{
    for (QTreeWidgetItem* item : treeWidgetItems()) {
        auto workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

        if (item->isSelected() && checked) {
            item->setText(TreeWidgetColumn::Default, QString("x"));
            item->setTextAlignment(TreeWidgetColumn::Default, Qt::AlignHCenter);
            workspace->setDefault(true);
        } else {
            item->setText(TreeWidgetColumn::Default, QString(""));
            workspace->setDefault(false);
        }
    }

    _projectManager->saveRecentWorkspacesSettings();
}

void SelectWorkspaceDialog::removeFromRecentList()
{
    QList<QTreeWidgetItem*> items = _ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        QString questionRemove = tr("Do you want to remove all selected workspaces from list?");

        if (items.count() == 1) {
            auto workspace = items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();
            questionRemove = tr("Do you want to remove workspace '%1' from list?").arg(workspace->name());
        }

        int ret = QMessageBox::question(this, tr("Remove Workspace."),
                                        questionRemove,
                                        QMessageBox::Ok | QMessageBox::Cancel);

        if (ret == QMessageBox::Ok) {
            bool recentUsedWorkspaceVectorChanged = false;
            auto recentUsedWorkspaces = _projectManager->recentWorkspaces();

            for (QTreeWidgetItem* item : items) {
                auto workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

                if (!workspace.isNull() && recentUsedWorkspaces.contains(workspace)) {
                    int indexItem = _ui->treeWidgetRecentWorkspace->indexOfTopLevelItem(item);
                    recentUsedWorkspaces.removeOne(workspace);
                    delete _ui->treeWidgetRecentWorkspace->takeTopLevelItem(indexItem);
                    recentUsedWorkspaceVectorChanged = true;
                }
            }

            if (recentUsedWorkspaceVectorChanged) {
                _projectManager->setRecentWorkspaces(recentUsedWorkspaces);
                _projectManager->saveRecentWorkspacesSettings();
            }

            recentUsedWorkspaces.clear();
        }
    }
}

void SelectWorkspaceDialog::deleteWorkspace()
{
    QList<QTreeWidgetItem*> items = _ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        QString questionRemove = tr("Do you want to delete all selected workspaces ?\n");

        if (items.count() == 1) {
            questionRemove = tr("Do you want to delete workspace '%1'' ?\n").
                             arg((items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>())->name());
        }

        QCheckBox* deleteProjectsCheckBox = new QCheckBox(tr("Delete all Projects in this Workspace."));
        QMessageBox deleteWorkspaceMessageBox;
        deleteWorkspaceMessageBox.setText(questionRemove);
        deleteWorkspaceMessageBox.setCheckBox(deleteProjectsCheckBox);
        deleteWorkspaceMessageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        deleteWorkspaceMessageBox.setIcon(QMessageBox::Question);

        if (deleteWorkspaceMessageBox.exec() == QMessageBox::Ok) {
            for (QTreeWidgetItem* item : treeWidgetItems()) {
                if (item->isSelected()) {
                    auto workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

                    if (workspace->isOpen()) {
                        QMessageBox::warning(this, tr("Delete Workspace '%1' failed.").arg(workspace->name()),
                                             tr("Could not delete an open Workspace.\nSwitch to a different workspace first."), QMessageBox::Ok);
                        continue;
                    }

                    _projectManager->removeRecentWorkspaces(workspace);
                    delete _ui->treeWidgetRecentWorkspace->takeTopLevelItem(_ui->treeWidgetRecentWorkspace->indexOfTopLevelItem(item));
                    workspace->deleteWorkspace(deleteProjectsCheckBox);
                }
            }

            _projectManager->saveRecentWorkspacesSettings();
        }
    }
}

void SelectWorkspaceDialog::showDialogNewWorkspace()
{
    QDialog* dialogNewWorkspace = createDialogWorkspaceGui(WorkspaceGuiType::newWorkspace, _workspaceGuiVector);
    dialogNewWorkspace->exec();
}

void SelectWorkspaceDialog::showDialogLoadWorkspace()
{
    QDialog* dialogLoadWorkspace = createDialogWorkspaceGui(WorkspaceGuiType::loadWorkspace, _workspaceGuiVector);
    dialogLoadWorkspace->exec();
}

void SelectWorkspaceDialog::showDialogEditWorkspace()
{
    QDialog* dialogEditWorkspace = createDialogWorkspaceGui(WorkspaceGuiType::editWorkspace, _workspaceGuiVector);
    dialogEditWorkspace->exec();
}

void SelectWorkspaceDialog::onWorkspaceUpdated()
{
    setRecentUsedWorkspaces();
}

void SelectWorkspaceDialog::searchRecentUsedWorkspaces()
{
    QList<QTreeWidgetItem*> items = _ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        auto workspace = items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

        for(AbstractWorkspaceGui* workspaceGui : _workspaceGuiVector){
            if(workspaceGui->isTypeFriendly(workspace)){
                workspaceGui->showChangeSourceDialog(workspace);
                return;
            }
        }
    }
}
