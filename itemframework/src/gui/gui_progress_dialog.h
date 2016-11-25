#ifndef GUI_PROGRESS_DIALOG_H
#define GUI_PROGRESS_DIALOG_H

#include <QDialog>

namespace Ui
{
class Gui_Progress_Dialog;
}

class Gui_Progress_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Gui_Progress_Dialog(QWidget* parent = 0);
    ~Gui_Progress_Dialog();

    void set_autoclose(bool close);

private:
    Ui::Gui_Progress_Dialog* ui;
    bool autoclose;
protected:
    void timerEvent(QTimerEvent* ev);

public slots:
    void progress(const int value);
    void progress(const int value, const QString& statusinfo);
    void progress(const int value, const QString& statusinfo, const QString& titleinfo);

private slots:
    void on_progressBar_valueChanged(int value);

};

#endif // GUI_PROGRESS_DIALOG_H
