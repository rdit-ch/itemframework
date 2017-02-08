#ifndef SQL_WORKSPACE_NEW_DIALOG_H
#define SQL_WORKSPACE_NEW_DIALOG_H

#include <QWidget>

namespace Ui
{
class SqlWorkspaceNewDialog;
}

class SqlWorkspaceNewDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SqlWorkspaceNewDialog(QWidget* parent = 0);
    ~SqlWorkspaceNewDialog();

private:
    Ui::SqlWorkspaceNewDialog* ui;
};

#endif // SQL_WORKSPACE_NEW_DIALOG_H
