#ifndef PROJECT_SAVE_REMINDER_DIALOG_H
#define PROJECT_SAVE_REMINDER_DIALOG_H

#include <QDialog>
#include "project_gui.h"

namespace Ui
{
class ProjectSaveReminderDialog;
}

class ProjectSaveReminderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSaveReminderDialog(const QVector<QSharedPointer<ProjectGui> >& dirtyProjectGuis, QWidget* parent = 0);
    ~ProjectSaveReminderDialog();

    QVector<QSharedPointer<ProjectGui> > projectGuiSaveSelection() const;
    QVector<QSharedPointer<ProjectGui> > projectGuiDiscardSelection() const;

private:
    Ui::ProjectSaveReminderDialog* ui;

private slots:
    void listSelectionChanged();
};

#endif // PROJECT_SAVE_REMINDER_DIALOG_H
