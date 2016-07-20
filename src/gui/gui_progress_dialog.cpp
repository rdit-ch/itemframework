#include <QDialog>

#include "gui_progress_dialog.h"
#include "ui_gui_progress_dialog.h"
#include "gui/gui_manager.h"

Gui_Progress_Dialog::Gui_Progress_Dialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Gui_Progress_Dialog)
{
    ui->setupUi(this);
    autoclose = true;

    //prevent the widget from starting before a selected time is over
    startTimer(100);
}

void Gui_Progress_Dialog::progress(const int value)
{
    setWindowTitle("Loading...");
    ui->lbl_status->setText("");
    ui->progressBar->setValue(value);
}

void Gui_Progress_Dialog::progress(const int value, const QString& statusinfo)
{
    setWindowTitle("Loading...");
    ui->lbl_status->setText(statusinfo);
    ui->progressBar->setValue(value);
}

void Gui_Progress_Dialog::progress(const int value, const QString& statusinfo, const QString& titleinfo)
{
    if (titleinfo != "") {
        setWindowTitle(titleinfo);
    }

    ui->lbl_status->setText(statusinfo);
    ui->progressBar->setValue(value);
}

void Gui_Progress_Dialog::on_progressBar_valueChanged(int value)
{
    if ((value >= 100) && autoclose) {
        this->close();
    }
}

void Gui_Progress_Dialog::set_autoclose(bool close)
{
    autoclose = close;
}

void Gui_Progress_Dialog::timerEvent(QTimerEvent* ev)
{
    if (ui->progressBar->value() < 100) {
        show();
    }

    killTimer(ev->timerId());
}

Gui_Progress_Dialog::~Gui_Progress_Dialog()
{
    delete ui;
}


