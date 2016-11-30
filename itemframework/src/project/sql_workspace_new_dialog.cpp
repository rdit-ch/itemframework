#include "sql_workspace_new_dialog.h"
#include "ui_sql_workspace_new_dialog.h"

SqlWorkspaceNewDialog::SqlWorkspaceNewDialog(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SqlWorkspaceNewDialog)
{
    ui->setupUi(this);
}

SqlWorkspaceNewDialog::~SqlWorkspaceNewDialog()
{
    delete ui;
}
