#ifndef LIVEDOC_WIDGET_H
#define LIVEDOC_WIDGET_H

#include <QMainWindow>
#include <QUrl>

namespace Ui
{
class Livedoc_Widget;
}

class Livedoc_Widget : public QMainWindow
{
    Q_OBJECT

public:
    explicit Livedoc_Widget(QWidget* parent = 0);
    ~Livedoc_Widget();

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

public slots:
    void navigate(QUrl doc);
    void force_navigate(QUrl doc);
signals:
    void signal_close();


private slots:
    void on_btnChange_clicked();

    void on_btnNoChange_clicked();

private:
    Ui::Livedoc_Widget* ui;
    QUrl lastUrl;
};

#endif // LIVEDOC_WIDGET_H
