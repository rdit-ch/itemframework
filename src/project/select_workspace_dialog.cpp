#include "select_workspace_dialog.h"
#include "ui_select_workspace_dialog.h"
#include "file_workspace_gui.h"
#include "sql_workspace_gui.h"
#include "plugin/plugin_manager.h"
#include "project_manager_gui.h"
#include "project_manager_config.h"

SelectWorkspaceDialog::SelectWorkspaceDialog(ProjectManager* projectManager) : QDialog(), ui(new Ui::SelectWorkspaceDialog)
{
    // Setup UI
    ui->setupUi(this);

    // Set project manager
    _projectManager = projectManager;

    // Set the recent used workspaces and prepare the treewidget.
    setRecentUsedWorkspaces();

    // Connect UI elements
    ui->treeWidgetRecentWorkspace->hideColumn(TreeWidgetColumn::Workspace);
    ui->treeWidgetRecentWorkspace->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidgetRecentWorkspace, &QTreeWidget::customContextMenuRequested, this, &SelectWorkspaceDialog::customContextMenuRequested);
    connect(ui->treeWidgetRecentWorkspace, &QTreeWidget::itemClicked, this, &SelectWorkspaceDialog::recentWorkspaceSelected);
    connect(ui->treeWidgetRecentWorkspace, &QTreeWidget::itemDoubleClicked, this, &SelectWorkspaceDialog::loadRecentUsedWorkspace);
    connect(ui->treeWidgetRecentWorkspace, &QTreeWidget::itemSelectionChanged, this, &SelectWorkspaceDialog::recentUsedWorkspaceSelectionChanged);

    for (const QSharedPointer<AbstractWorkspace> abstractWorkspace : _projectManager->recentWorkspaces()) {
        connect(abstractWorkspace.data(), &AbstractWorkspace::workspaceNameChanged, this, &SelectWorkspaceDialog::onWorkspaceNameChanged);
        connect(abstractWorkspace.data(), &AbstractWorkspace::workspaceDescriptionChanged, this, &SelectWorkspaceDialog::onWorkspaceDescriptionChanged);
        connect(abstractWorkspace.data(), &AbstractWorkspace::workspaceConnectionChanged, this, &SelectWorkspaceDialog::onWorkspaceConnectionChanged);
    }
}

SelectWorkspaceDialog::~SelectWorkspaceDialog()
{
    for (const QSharedPointer<AbstractWorkspace> abstractWorkspace : _projectManager->recentWorkspaces()) {
        disconnect(abstractWorkspace.data(), 0, this, 0);
    }

    // Delete SelectWorkspaceDialog UI
    delete ui;
}

void SelectWorkspaceDialog::setRecentUsedWorkspaces()
{
    ui->treeWidgetRecentWorkspace->clear();
    QSharedPointer<AbstractWorkspace> currentWorkspace = _projectManager->currentWorkspace();

    for (const QSharedPointer<AbstractWorkspace>& abstractWorkspace : _projectManager->recentWorkspaces()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidgetRecentWorkspace);
        item->setData(TreeWidgetColumn::Workspace, Qt::UserRole, QVariant::fromValue(abstractWorkspace));
        item->setText(TreeWidgetColumn::Open, QString());
        item->setText(TreeWidgetColumn::Default, QString());
        item->setText(TreeWidgetColumn::LastUsed, abstractWorkspace->lastUsedDateTime());
        item->setText(TreeWidgetColumn::Name, abstractWorkspace->name());
        item->setText(TreeWidgetColumn::Type, abstractWorkspace->typeString());
        item->setText(TreeWidgetColumn::Connection, abstractWorkspace->connectionString());
        item->setToolTip(TreeWidgetColumn::Name, abstractWorkspace->description());

        if (!abstractWorkspace->isValid()) {
            item->setIcon(TreeWidgetColumn::Connection, QIcon(":/core/projectmanager/workspace_not_valid.png"));
            item->setToolTip(TreeWidgetColumn::Connection, abstractWorkspace->lastError());
        }

        if (abstractWorkspace->isDefault()) {
            item->setText(TreeWidgetColumn::Default, QString("x"));
            item->setTextAlignment(TreeWidgetColumn::Default, Qt::AlignHCenter);
        }

        if (!currentWorkspace.isNull() && currentWorkspace->compare(abstractWorkspace)) {
            setSelectedWorkspace(abstractWorkspace);
            item->setText(TreeWidgetColumn::Open, QString("x"));
            item->setTextAlignment(TreeWidgetColumn::Open, Qt::AlignHCenter);
            ui->treeWidgetRecentWorkspace->setCurrentItem(item);
        }
    }

    ui->treeWidgetRecentWorkspace->header()->resizeSection(TreeWidgetColumn::Open, 55);
    ui->treeWidgetRecentWorkspace->header()->resizeSection(TreeWidgetColumn::Default, 55);
    ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::LastUsed);
    ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Name);
    ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Type);
    ui->treeWidgetRecentWorkspace->resizeColumnToContents(TreeWidgetColumn::Connection);
    ui->treeWidgetRecentWorkspace->sortByColumn(TreeWidgetColumn::LastUsed, Qt::DescendingOrder);

    _projectManager->saveRecentWorkspacesSettings();
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

QList<QTreeWidgetItem*> SelectWorkspaceDialog::treeWidgetItems() const
{
    QList<QTreeWidgetItem*> items;

    for (int i = 0; i < ui->treeWidgetRecentWorkspace->topLevelItemCount(); ++i) {
        if (ui->treeWidgetRecentWorkspace->topLevelItem(i) != nullptr) {
            items.append(ui->treeWidgetRecentWorkspace->topLevelItem(i));
        }\
    }

    return items;
}

void SelectWorkspaceDialog::customContextMenuRequested(const QPoint& position)
{
    QTreeWidgetItem* item = ui->treeWidgetRecentWorkspace->itemAt(position);

    if (item != nullptr) {
        QSharedPointer<AbstractWorkspace> workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

        if (!workspace.isNull()) {
            QPoint menuePosition = ui->treeWidgetRecentWorkspace->mapToGlobal(position);
            menuePosition.setY(menuePosition.y() + 15);
            showContextMenuRecentWorkspace(menuePosition, workspace);
        }
    }
}

void SelectWorkspaceDialog::showContextMenuRecentWorkspace(const QPoint& position, const QSharedPointer<AbstractWorkspace>& workspace)
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
    int selectedItemCount = ui->treeWidgetRecentWorkspace->selectedItems().count();

    if (selectedItemCount > 1) {
        actionSetDefault->setEnabled(false);
        actionSetDefault->setChecked(false);
        actionEditWorkspace->setEnabled(false);
        actionEditWorkspace->setText(QLatin1String("Edit"));
        actionDeleteWorkspace->setText(QString("Delete all selected."));
        actionRemoveFromRecentList->setText(QString("Remove all selected from List."));
    }

    connect(actionEditWorkspace, &QAction::triggered, this, &SelectWorkspaceDialog::showDialogEditWorkspace);
    connect(actionDeleteWorkspace, &QAction::triggered, this, &SelectWorkspaceDialog::deleteWorkspace);
    connect(actionRemoveFromRecentList, &QAction::triggered, this, &SelectWorkspaceDialog::removeFromRecentList);
    connect(actionSetDefault, &QAction::toggled, this, &SelectWorkspaceDialog::defaultWorkspaceChange);
    menu.exec(position);
}

void SelectWorkspaceDialog::acceptWorkspace(QSharedPointer<AbstractWorkspace> workspace)
{
    if (!workspace.isNull()) {
        // Add current workspace into recent used workspace vector.
        _projectManager->addRecentWorkspace(workspace);
        // Set selected workspace and save it
        setSelectedWorkspace(workspace);

        if (workspace->isDefault()) {
            defaultWorkspaceChange(true);
        }

        accept();
    }
}

void SelectWorkspaceDialog::recentWorkspaceSelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    QSharedPointer<AbstractWorkspace> workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();
    setSelectedWorkspace(workspace);
}

void SelectWorkspaceDialog::loadRecentUsedWorkspace(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    QSharedPointer<AbstractWorkspace> workspace;
    workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

    if (!workspace.isNull()) {
        if (!workspace->test()) {
            QMessageBox::warning(this, tr("Load Workspace."),
                                 tr("This Workspace is invalid.\n\n%1").arg(workspace->lastError()),
                                 QMessageBox::Ok);
            return;
        }

        setSelectedWorkspace(workspace);
        accept();
    }
}

void SelectWorkspaceDialog::setSelectedWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    clearSelectedWorkspace();
    _selectedWorkspace = workspace;

    if (!_selectedWorkspace.isNull()) {
        ui->buttonAcceptDialog->setEnabled(true);
    } else {
        ui->buttonAcceptDialog->setEnabled(false);
    }
}

void SelectWorkspaceDialog::clearSelectedWorkspace()
{
    if (!_selectedWorkspace.isNull()) {
        disconnect(_selectedWorkspace.data());
        _selectedWorkspace.clear();
    }

    ui->buttonAcceptDialog->setEnabled(false);
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
    const QList<QTreeWidgetItem*> items = ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        QSharedPointer<AbstractWorkspace> workspace = items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();
        setSelectedWorkspace(workspace);
        return;
    }

    clearSelectedWorkspace();
}

void SelectWorkspaceDialog::defaultWorkspaceChange(bool checked)
{
    for (QTreeWidgetItem* item : treeWidgetItems()) {
        QSharedPointer<AbstractWorkspace> workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

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
    QList<QTreeWidgetItem*> items = ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        QString questionRemove = QStringLiteral("Do you want to remove all selected workspaces from list?");

        if (items.count() == 1) {
            QSharedPointer<AbstractWorkspace> workspace = items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();
            questionRemove = QStringLiteral("Do you want to remove workspace '%1' from list?").arg(workspace->name());
        }

        int ret = QMessageBox::question(this, tr("Remove Workspace."),
                                        questionRemove,
                                        QMessageBox::Ok | QMessageBox::Cancel);

        if (ret == QMessageBox::Ok) {
            bool recentUsedWorkspaceVectorChanged = false;

            QVector<QSharedPointer<AbstractWorkspace>> recentUsedWorkspaces = _projectManager->recentWorkspaces();

            for (QTreeWidgetItem* item : items) {
                QSharedPointer<AbstractWorkspace> workspace;
                workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

                if (!workspace.isNull() && recentUsedWorkspaces.contains(workspace)) {
                    int indexItem = ui->treeWidgetRecentWorkspace->indexOfTopLevelItem(item);
                    recentUsedWorkspaces.removeOne(workspace);
                    delete ui->treeWidgetRecentWorkspace->takeTopLevelItem(indexItem);
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
    QList<QTreeWidgetItem*> items = ui->treeWidgetRecentWorkspace->selectedItems();

    if (!items.isEmpty()) {
        QString questionRemove = QStringLiteral("Do you want to delete all selected workspaces ?\n");

        if (items.count() == 1) {
            questionRemove = QStringLiteral("Do you want to delete workspace '%1'' ?\n").
                             arg((items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>())->name());
        }

        QCheckBox* deleteProjectsCheckBox = new QCheckBox(tr("Delete all Projects in this Workspace."));
        QMessageBox deleteWorkspaceMessageBox;
        deleteWorkspaceMessageBox.setText(questionRemove);
        deleteWorkspaceMessageBox.setCheckBox(deleteProjectsCheckBox);
        deleteWorkspaceMessageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        deleteWorkspaceMessageBox.setIcon(QMessageBox::Question);

        if (deleteWorkspaceMessageBox.exec() == QMessageBox::Ok) {
            QSharedPointer<AbstractWorkspace> workspace;

            for (QTreeWidgetItem* item : treeWidgetItems()) {
                if (item->isSelected()) {
                    workspace = item->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

                    if (workspace->isOpen()) {
                        QMessageBox::warning(this, tr("Delete Workspace '%1' failed.").arg(workspace->name()),
                                             tr("Could not delete an open Workspace.\nSwitch to a different workspace first."), QMessageBox::Ok);
                        continue;
                    }

                    _projectManager->removeRecentWorkspaces(workspace);
                    delete ui->treeWidgetRecentWorkspace->takeTopLevelItem(ui->treeWidgetRecentWorkspace->indexOfTopLevelItem(item));
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
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    if (button == nullptr) {
        QList<QTreeWidgetItem*> items = ui->treeWidgetRecentWorkspace->selectedItems();

        if (!items.isEmpty()) {
            QSharedPointer<AbstractWorkspace> workspace = items.first()->data(TreeWidgetColumn::Workspace, Qt::UserRole).value<QSharedPointer<AbstractWorkspace>>();

            if (!workspace.isNull()) {
                AbstractWorkspaceGui* workspaceGui = ProjectManagerGui::abstractWorkspaceGuiClass(workspace, _workspaceGuiVector);
                QDialog* editWorkspaceDialog = workspaceGui->dialogEditWorkspace(0, workspace);
                editWorkspaceDialog->exec();
                return;
            }
        }
    } else {
        QDialog* dialogEditWorkspace = createDialogWorkspaceGui(WorkspaceGuiType::editWorkspace, _workspaceGuiVector);
        dialogEditWorkspace->exec();
    }
}

void SelectWorkspaceDialog::onWorkspaceNameChanged(const QString& workspaceName)
{
    setRecentUsedWorkspaces();
}

void SelectWorkspaceDialog::onWorkspaceConnectionChanged(const QString& workspaceConnection)
{
    setRecentUsedWorkspaces();
}

void SelectWorkspaceDialog::onWorkspaceDescriptionChanged(const QString& workspaceName)
{
    setRecentUsedWorkspaces();
}
