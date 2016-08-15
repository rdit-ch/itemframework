#ifndef PROJECT_GUI_H
#define PROJECT_GUI_H

#include <QObject>
#include <QPoint>
#include <QDialog>
#include <QPointer>
#include <QEnableSharedFromThis>

class ItemView;
class AbstractProject;
class AbstractWorkspaceGui;
class ProjectChangedExternDialog;
class QTimer;
class QMenu;

namespace Ui
{
class ProjectInfoDialog;
}

class ProjectGui : public QObject, public QEnableSharedFromThis<ProjectGui>
{
    Q_OBJECT
public:
    ProjectGui(AbstractWorkspaceGui* parent, QSharedPointer<AbstractProject> project);
    ~ProjectGui();

    bool isValid() const;
    bool isLoaded() const;
    bool isFastLoaded() const;
    QSharedPointer<AbstractProject> project() const;
    ItemView* itemView() const;
    void reset();
    bool save(bool autosave = false);
    bool load();
    bool unload();
    int tabWidgetIndex() const;
    void setTabWidgetIndex(int tabWidgetIndex);
    QString projectGuiLabel() const;
    QString projectName() const;
    void setLoaded(bool isLoaded);
    QString lastError() const;
    int autosaveTimerInterval() const;
    void setAutosaveTimerInterval(int value);
    void showProjectContextMenue(const QPoint& globalPosition) const;
    void setDialogPositionOffset(const QPoint &dialogPoistion);

private:
    QTimer* _autosaveTimer = nullptr;
    int _autosaveTimerInterval;
    bool saveReminder();
    void showProjectChangedByExternalDialog();
    ProjectChangedExternDialog* _projectChangedExternDialog = nullptr;
    AbstractWorkspaceGui* _parent = nullptr;
    QSharedPointer<AbstractProject> _project;
    QMenu* _contextMenu = nullptr;
    ItemView* _itemView = nullptr;
    bool _isLoaded = false;
    int _tabWidgetIndex = -1;
    QString _projectGuiLabel;
    QString _projectName;
    QString _description;
    bool _domChanged = false;
    QString _lastError;
    QPoint _dialogPositionOffset;
    QPointer<QDialog> _projectInfoDialog;

signals:
    void projectGuiLabelChanged(QSharedPointer<ProjectGui> projectGui);
    void projectLoaded(QSharedPointer<ProjectGui> projectGui);
    void projectUnloaded(QSharedPointer<ProjectGui> projectGui);

public slots:
    void onLoadTrigger();
    void onLoadAllTrigger();
    void onUnloadTrigger();
    void onUnloadAllTrigger();
    void onSaveTrigger();
    void onSaveAllTrigger();
    void onRemoveTrigger();
    void onDeleteTrigger();
    void onFastLoad(bool state);
    void showProjectInfo();
    void onEditTrigger();  
    void onUnloadAllExceptThisTrigger();
    void onUnloadAllExceptVisibleTrigger();

private slots:
    void onAutosaveTimeout();
    void onMainWindowActivationChanged();
    bool reloadDomDocument();
    void onItemViewSceneChanged();
    void onExternDomChanged();
    void onStateChanged();
    void onSearchProject();
};
#endif // PROJECT_GUI_H
