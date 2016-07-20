#include "livedoc_widget.h"
#include "ui_livedoc_widget.h"

Livedoc_Widget::Livedoc_Widget(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Livedoc_Widget)
{

    setWindowFlags(windowFlags() | Qt::WindowDoesNotAcceptFocus);
    ui->setupUi(this);
    ui->noticeWidget->setVisible(false);
}

void Livedoc_Widget::navigate(QUrl doc)
{
    if (ui->actionAutoNavigate->isChecked() && isVisible()) {
        ui->textBrowser->setSource(doc);
    } else if (isVisible()) {
        lastUrl = doc;
        ui->noticeWidget->setVisible(doc != ui->textBrowser->source());
        ui->lblChangeTo->setText(QString("Change to %1 ?").arg(doc.toString()));
    }
}

void Livedoc_Widget::force_navigate(QUrl doc)
{
    ui->textBrowser->setSource(doc);
}

void Livedoc_Widget::closeEvent(QCloseEvent* e)
{
    emit signal_close();
    QMainWindow::closeEvent(e);
}


Livedoc_Widget::~Livedoc_Widget()
{
    delete ui;
}

void Livedoc_Widget::showEvent(QShowEvent* ev)
{
    force_navigate(QUrl("help:home"));
    QWidget::showEvent(ev);
}

void Livedoc_Widget::on_btnChange_clicked()
{
    force_navigate(lastUrl);
    ui->noticeWidget->setVisible(false);
}

void Livedoc_Widget::on_btnNoChange_clicked()
{
    ui->noticeWidget->setVisible(false);
}
