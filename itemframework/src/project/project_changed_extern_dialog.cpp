#include <QDebug>
#include <QTextEdit>
#include "project_changed_extern_dialog.h"
#include "ui_project_changed_extern_dialog.h"

ProjectChangedExternDialog::ProjectChangedExternDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ProjectChangedExternDialog)
{
    ui->setupUi(this);
    _projectChangedAction = NoAction;
}

ProjectChangedExternDialog::~ProjectChangedExternDialog()
{
    delete ui;
}

ProjectChangedAction ProjectChangedExternDialog::projectChangedAction() const
{
    return _projectChangedAction;
}

void ProjectChangedExternDialog::setText(const QString& text)
{
    ui->dialogLabel->setText(text);
}

void ProjectChangedExternDialog::setDetails(const QString& text)
{
    _textDetails = text;
}

void ProjectChangedExternDialog::onShowDetails(bool toggle)
{
    if (toggle) {
        textEditDetails = new QTextEdit(_textDetails, this);
        textEditDetails->setReadOnly(true);
        ui->verticalLayout->addWidget(textEditDetails);
    } else {
        if (textEditDetails != nullptr) {
            delete textEditDetails;
            textEditDetails = nullptr;
            adjustSize();
        }
    }
}

void ProjectChangedExternDialog::accept()
{
    const QString buttonName = QObject::sender()->objectName();

    if (buttonName == QStringLiteral("pushButtonYes")) {
        // Reload the project
        _projectChangedAction = Discard;
    } else if (buttonName == QStringLiteral("pushButtonYesToAll")) {
        // Reload all projects
        _projectChangedAction = DiscardAll;
    } else if (buttonName == QStringLiteral("pushButtonNo")) {
        // Save the project
        _projectChangedAction = Save;
    } else if (buttonName == QStringLiteral("pushButtonNoToAll")) {
        // Save all projects
        _projectChangedAction = SaveAll;
    } else {
        // Should not happen but if we are save
        _projectChangedAction = NoAction;
    }

    QDialog::accept();
}
