#ifndef PROJECT_CHANGED_EXTERN_DIALOG_H
#define PROJECT_CHANGED_EXTERN_DIALOG_H

#include <QDialog>

class QTextEdit;

enum ProjectChangedAction {
    Save,
    SaveAll,
    Discard,
    DiscardAll,
    NoAction
};

namespace Ui
{
class ProjectChangedExternDialog;
}

class ProjectChangedExternDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectChangedExternDialog(QWidget* parent = 0);
    ~ProjectChangedExternDialog();

    ProjectChangedAction projectChangedAction() const;

    void setText(const QString& text);
    void setDetails(const QString& text);

private:
    Ui::ProjectChangedExternDialog* ui;
    QTextEdit* textEditDetails = nullptr;
    ProjectChangedAction _projectChangedAction;
    QString _textDetails;

private slots:
    void onShowDetails(bool toggle);

protected slots:
    void accept();
};

#endif // PROJECT_CHANGED_EXTERN_DIALOG_H
