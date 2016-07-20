#ifndef CONSOLE_WIDGET_H
#define CONSOLE_WIDGET_H

#include <QDockWidget>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

namespace Ui
{
class ConsoleWidget;
}

/**
 * @brief The ConsoleWidget shows the contents of ConsoleModel::instance() (an QAbstractTableModel) in a QTableView.
 * It also provides a lineedit to allow fulltext filtering.
 */
class ConsoleWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ConsoleWidget(QAbstractTableModel* model);
    ~ConsoleWidget();
private slots:
    void filterChanged(QString text);
    void clearPressed();
    void tableContextMenu(QPoint pos);

private:
    Ui::ConsoleWidget* ui;

    /**
     * @brief Proxy Model used for fulltext filter
     */
    QSortFilterProxyModel _proxyModel;
};

#endif // CONSOLE_WIDGET_H
