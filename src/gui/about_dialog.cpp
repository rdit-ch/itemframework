#include "about_dialog.h"
#include "ui_about_dialog.h"
#include "appcore.h"

AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // TODO: fix

    // itemframework
    ui->lblApiVersion->setText(QString("%1 (%2)").arg(ApiVersion).arg(SourceVersion));

    // user application
    ui->lblProgrVersion->setText(QApplication::applicationVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
