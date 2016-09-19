#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

#include <QMainWindow>

namespace Ui
{
class Gui_Main_Window;
}

class Gui_Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Gui_Main_Window(QWidget* parent = 0);
    QToolBar* toolBar();
    QMenuBar* menuBar();
    ~Gui_Main_Window();

private slots:
    void on_actionAboutQt_triggered();

    void on_actionAbout_Application_triggered();

private:
    Ui::Gui_Main_Window* ui;
};

#endif // GUI_MAIN_WINDOW_H
