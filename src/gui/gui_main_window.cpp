#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "appcore.h"
#include <QMessageBox>
#include "about_dialog.h"

Gui_Main_Window::Gui_Main_Window(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Gui_Main_Window)
{
    ui->setupUi(this);

}

QToolBar *Gui_Main_Window::toolBar()
{
    return ui->toolBar;
}

Gui_Main_Window::~Gui_Main_Window()
{
    delete ui;
}

void Gui_Main_Window::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void Gui_Main_Window::on_actionAbout_Application_triggered()
{
    AboutDialog dlg(this);
    dlg.exec(); //Show about dialog modal
}
