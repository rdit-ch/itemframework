#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include "abstract_project.h"
#include "project_gui.h"
#include "gui/gui_manager.h"
#include "item/item_view.h"
#include "abstract_workspace_gui.h"
#include "project_changed_extern_dialog.h"
#include "project_manager_config.h"

ProjectGui::ProjectGui(AbstractWorkspaceGui* parent, QSharedPointer<AbstractProject> project)
{
    _autosaveTimer = new QTimer(this);
    _autosaveTimer->setInterval(AutosaveInerval);
    _projectChangedExternDialog = new ProjectChangedExternDialog();
    _abstractWorkspaceGui = parent;
    _project = project;
    _projectGuiLabel = _project->name();
    _projectName = _project->name();
    connect(Gui_Manager::instance(), &Gui_Manager::mainWindowActivationChange, this, &ProjectGui::onMainWindowActivationChanged);
    connect(_project.data(), &AbstractProject::stateChange, this, &ProjectGui::onStateChanged);
    connect(_project.data(), &AbstractProject::externDomChange, this, &ProjectGui::onExternDomChanged);
    connect(_project.data(), &AbstractProject::internDomChanged, this, &ProjectGui::reloadDomDocument);
    connect(_autosaveTimer, &QTimer::timeout, this, &ProjectGui::onAutosaveTimeout);
}

ProjectGui::~ProjectGui()
{
    if (_projectChangedExternDialog != nullptr) {
        delete _projectChangedExternDialog;
    }

    if (_autosaveTimer->isActive()) {
        _autosaveTimer->stop();
    }

    delete _autosaveTimer;
}

void ProjectGui::showProjectContextMenue(const QPoint& globalPosition) const
{
    QMenu projectContextMenue;
    QAction* loadProject = projectContextMenue.addAction(QString("Open \"%1\"").arg(_projectGuiLabel));
    QAction* fastLoadProject = projectContextMenue.addAction(QString("Open \"%1\" directly").arg(_projectGuiLabel));
    fastLoadProject->setCheckable(true);
    projectContextMenue.addAction(projectContextMenue.addSeparator());

    QAction* saveProject = projectContextMenue.addAction(QString("Save \"%1\"").arg(_projectGuiLabel));
    QAction* saveAllProjects = projectContextMenue.addAction(QLatin1String("Save All"));
    projectContextMenue.addAction(projectContextMenue.addSeparator());

    QAction* unloadProject = projectContextMenue.addAction(QString("Close \"%1\"").arg(_project->name()));
    QAction* unloadAllExceptThisProject = projectContextMenue.addAction(QString("Close All Except \"%1\"").arg(_project->name()));
    QAction* unloadAllExceptVisibleProjects = projectContextMenue.addAction(QString("Close All Except Visible"));
    QAction* unloadAllProjects = projectContextMenue.addAction(QLatin1String("Close All"));
    projectContextMenue.addAction(projectContextMenue.addSeparator());

    QAction* removeProject = projectContextMenue.addAction(QString("Remove Project \"%1\" from Workspace").arg(_project->name()));
    QAction* deleteProject = projectContextMenue.addAction(QString("Delete Project \"%1\"").arg(_project->name()));
    projectContextMenue.addAction(projectContextMenue.addSeparator());

    QAction* editProject = projectContextMenue.addAction(QString("Edit Project..."));
    QAction* infoProject = projectContextMenue.addAction(QString("Project Info"));

    connect(fastLoadProject, &QAction::toggled, this, &ProjectGui::onFastLoad);
    connect(saveAllProjects, &QAction::triggered, this, &ProjectGui::onSaveAllTrigger);
    connect(unloadAllProjects, &QAction::triggered, this, &ProjectGui::onUnloadAllTrigger);
    connect(infoProject, &QAction::triggered, this, &ProjectGui::showProjectInfo);
    connect(saveProject, &QAction::triggered, this, &ProjectGui::onSaveTrigger);
    connect(loadProject, &QAction::triggered, this, &ProjectGui::onLoadTrigger);
    connect(unloadProject, &QAction::triggered, this, &ProjectGui::onUnloadTrigger);

    connect(unloadAllExceptThisProject, &QAction::triggered, this, &ProjectGui::onUnloadAllExceptThisTrigger);
    connect(unloadAllExceptVisibleProjects, &QAction::triggered, this, &ProjectGui::onUnloadAllExceptVisibleTrigger);

    connect(removeProject, &QAction::triggered, this, &ProjectGui::onRemoveTrigger);
    connect(deleteProject, &QAction::triggered, this, &ProjectGui::onDeleteTrigger);
    connect(editProject, &QAction::triggered, this, &ProjectGui::onEditTrigger);

    if (_isLoaded) {
        loadProject->setEnabled(false);
    } else {
        unloadProject->setEnabled(false);
    }

    if (_project->isFastLoad()) {
        fastLoadProject->setChecked(true);
    }

    projectContextMenue.exec(globalPosition);
}

QString ProjectGui::lastError() const
{
    return _lastError;
}

int ProjectGui::autosaveTimerInterval() const
{
    return _autosaveTimerInterval;
}

void ProjectGui::setAutosaveTimerInterval(int value)
{
    _autosaveTimerInterval = value;

    if (_autosaveTimer->isActive()) {
        _autosaveTimer->stop();
    }

    _autosaveTimer->setInterval(AutosaveInerval);
    _autosaveTimer->start();
}

QString ProjectGui::projectGuiLabel() const
{
    return _projectGuiLabel;
}

int ProjectGui::tabWidgetIndex() const
{
    return _tabWidgetIndex;
}

void ProjectGui::setTabWidgetIndex(int tabWidgetIndex)
{
    _tabWidgetIndex = tabWidgetIndex;
}

bool ProjectGui::saveReminder()
{
    if (_project->isDirty()) {
        QMessageBox saveReminder;
        saveReminder.setIcon(QMessageBox::Question);
        saveReminder.setText("The project " + _project->name() + " has been modified.");
        saveReminder.setInformativeText("Do you want to save your changes?");
        saveReminder.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        saveReminder.setDefaultButton(QMessageBox::Save);
        int ret = saveReminder.exec();

        switch (ret) {
        case QMessageBox::Discard:
            reset();
            break;

        case QMessageBox::Cancel:
            return false;

        case QMessageBox::Save:
            save();
            break;
        }
    }

    return true;
}

void ProjectGui::reset()
{
    _project->reset();

    if (isLoaded()) {
        if (!reloadDomDocument() || !isValid()) {
            QMessageBox::warning(0, tr("Error loading Project"), tr("Project is invalid."));
            unload();
        }
    }
}

bool ProjectGui::save(bool autosave)
{
    if (isLoaded()) {
        QDomDocument projectDomDocumentTemplate = _project->projectDomDocumentTemplate(_project->name(),
                _project->version(),
                _project->description());
        QDomElement projectRootDomElement =  projectDomDocumentTemplate.documentElement();
        _itemView->save(projectDomDocumentTemplate, projectRootDomElement);


        if (_project->setDomDocument(projectDomDocumentTemplate)) {
            if (autosave) {
                return _project->autosave();
            } else {
                return _project->save();
            }

        }
    }

    return false;
}

bool ProjectGui::load()
{
    reset();

    if (!isValid()) {
        _lastError = _project->lastError();
        return false;
    }

    // Check version
    if (_project->majorProjectVersion() < MajorProVersion) {
        // Incompatible version
        _lastError = QString("Invalid project version %1 -> expect 2.0.").arg(_project->version().toStdString().c_str());
        return false;
    }

    _itemView = new ItemView(sharedFromThis());
    connect(_itemView->itemScene(), &ItemScene::sceneRealChanged, this, &ProjectGui::onItemViewSceneChanged);

    QDomElement projectDomElement = _project->domDocument().documentElement();
    QMessageBox recoverProjectDialog;
    recoverProjectDialog.setText(_project->autosaveInfo());
    recoverProjectDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    recoverProjectDialog.setButtonText(QMessageBox::Yes, trUtf8("&Recover"));
    recoverProjectDialog.setButtonText(QMessageBox::No, trUtf8("&Delete"));
    recoverProjectDialog.setButtonText(QMessageBox::Abort, trUtf8("&Cancel"));
    recoverProjectDialog.setIcon(QMessageBox::Warning);

    if (_project->autosaveExists()) {
        switch (recoverProjectDialog.exec()) {
        case QMessageBox::Yes:
            projectDomElement = _project->autosaveDomDocument().documentElement();
            _project->setDomDocument(_project->autosaveDomDocument());
            _project->setDirty(true);
            break;

        case QMessageBox::No:
            _project->cleanAutosave();
            break;

        default:
            break;
        }
    }

    if (!_itemView->load(projectDomElement)) {
        delete _itemView;
        _itemView = nullptr;
        return false;
    }

    setLoaded(true);
    emit projectLoaded(sharedFromThis());
    return true;
}

bool ProjectGui::unload()
{
    // Check projectGui is loaded.
    if (_isLoaded) {
        // Disconnect projectGui from itemView scene.
        if (!_itemView->scene()->disconnect(this)) {
            // Error disconnect procedure.
            // Return unload projectGui procedure failed.
            return false;
        }

        // Delete itemView and set it to null.
        delete _itemView;
        _itemView = nullptr;
    }

    // Set projectGui loaded to false.
    setLoaded(false);
    emit projectUnloaded(sharedFromThis());

    // Return unload projectGui procedure is successfull.
    return true;
}

bool ProjectGui::isLoaded() const
{
    return _isLoaded;
}

void ProjectGui::setLoaded(bool isLoaded)
{
    _project->setLoaded(isLoaded);
    _isLoaded = isLoaded;

    if (_isLoaded) {
        _autosaveTimer->start();
    } else {
        if (_autosaveTimer->isActive()) {
            _autosaveTimer->stop();
        }
    }
}

bool ProjectGui::isValid() const
{
    return _project->isValid();
}

ItemView* ProjectGui::itemView() const
{
    return _itemView;
}

QSharedPointer<AbstractProject> ProjectGui::project() const
{
    return _project;
}

void ProjectGui::onStateChanged()
{
    QString newProjectGuiLabel = _projectGuiLabel;

    if (_project->isDirty()) {
        _projectGuiLabel =  _project->name() + QString("*");
    } else {
        _projectGuiLabel = _project->name();
    }

    if (newProjectGuiLabel != _projectGuiLabel) {
        emit projectGuiLabelChanged(sharedFromThis());
    }
}

void ProjectGui::onExternDomChanged()
{
    if (Gui_Manager::instance()->mainWindowIsActive()) {
        showProjectChangedByExternalDialog();
        return;
    }

    _domChanged = true;
}

void ProjectGui::onMainWindowActivationChanged()
{
    if (Gui_Manager::instance()->mainWindowIsActive() && _domChanged) {
        showProjectChangedByExternalDialog();
    }
}

bool ProjectGui::reloadDomDocument()
{
    if (!_project->isValid()) {
        return false;
    }

    if (_itemView == nullptr) {
        return false;
    }

    QDomElement projectDomElement = _project->domDocument().documentElement();
    QDomDocument testProjectDomDocument;
    QString errorSetDomDocument;

    if (!testProjectDomDocument.setContent(_project->domDocument().toByteArray(), &errorSetDomDocument)) {
        //qDebug() << errorSetDomDocument;
        return false;
    }

    if (!_itemView->reload(projectDomElement)) {
        return false;
    }

    _project->setDirty(false);
    return true;
}

void ProjectGui::showProjectChangedByExternalDialog()
{
    if (!_project->isExternChanged()) {
        return;
    }

    QString dialogText = QString("The project %1 has changed outside RTV.Do you want to reload it?\n")
                         .arg(_project->name());
    _domChanged = false;
    _projectChangedExternDialog->setText(dialogText);
    _projectChangedExternDialog->setDetails(_project->connectionString());

    if (_projectChangedExternDialog->exec() == QDialog::Accepted) {
        switch (_projectChangedExternDialog->projectChangedAction()) {
        case ProjectChangedAction::Save:
            save();
            break;

        case ProjectChangedAction::Discard:
            reset();
            break;

        case ProjectChangedAction::SaveAll:
            _abstractWorkspaceGui->saveExternChangedProjects();
            break;

        case ProjectChangedAction::DiscardAll:
            _abstractWorkspaceGui->resetExternChangedProjects();
            break;

        default:
            break;
        };
    }
}

void ProjectGui::onLoadTrigger()
{
    _abstractWorkspaceGui->loadProject(sharedFromThis());
}

void ProjectGui::onUnloadTrigger()
{
    _abstractWorkspaceGui->unloadProject(sharedFromThis());
}

void ProjectGui::onSaveTrigger()
{
    save();
}

void ProjectGui::onSaveAllTrigger()
{
    _abstractWorkspaceGui->onSaveAllProjects();
}

void ProjectGui::onLoadAllTrigger()
{
    _abstractWorkspaceGui->onLoadAllProjects();
}

void ProjectGui::onUnloadAllTrigger()
{
    _abstractWorkspaceGui->onUnloadAllProjects();
}

void ProjectGui::onUnloadAllExceptThisTrigger()
{
    _abstractWorkspaceGui->unloadAllProjectsExceptThis(sharedFromThis());
}

void ProjectGui::onUnloadAllExceptVisibleTrigger()
{
    _abstractWorkspaceGui->unloadAllProjectsExceptVisible();
}

void ProjectGui::onRemoveTrigger()
{
    _abstractWorkspaceGui->removeProject(sharedFromThis());
}

void ProjectGui::onDeleteTrigger()
{
    _abstractWorkspaceGui->deleteProject(sharedFromThis());
}

void ProjectGui::onFastLoad(bool state)
{
    if (_project->isFastLoad() != state) {
        _project->setFastLoad(state);
        _abstractWorkspaceGui->workspace()->save();
    }
}

void ProjectGui::showProjectInfo()
{
    const QString projectInformation = tr("Name: %1\n\nConnection: %2\n\nDescription: \n%3")
                                       .arg(_project->name())
                                       .arg(_project->connectionString())
                                       .arg(_project->description());
    QMessageBox::information(0, tr("Project Information"), projectInformation, QMessageBox::Ok);
}

void ProjectGui::onEditTrigger()
{
    bool descriptionAccepted;
    QString projectDescription = QInputDialog::getMultiLineText(0,
                                 tr("Edit Project"),
                                 tr("Description"),
                                 _project->description(), &descriptionAccepted);

    if (descriptionAccepted) {
        _project->setDescription(projectDescription);
    }
}

void ProjectGui::onItemViewSceneChanged()
{
    _project->setDirty(true);
}

void ProjectGui::onAutosaveTimeout()
{
    save(true);
}
