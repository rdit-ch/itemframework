#ifndef ITEM_TEMPLATES_WIDGET_H
#define ITEM_TEMPLATES_WIDGET_H

#include <QDockWidget>
#include <QDomDocument>
#include <QModelIndexList>

namespace Ui {
class ItemTemplatesWidget;
}

class ItemTemplatesWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ItemTemplatesWidget(QWidget *parent = 0);
    ~ItemTemplatesWidget();
    void createTemplate(QDomDocument const& items, QPixmap const& pixmap);

private slots:
    void handleSelectionChanged(QModelIndex const& current, QModelIndex const&);

private:
    bool eventFilter(QObject* obj, QEvent* event);
    void saveTemplate(QString const& name, QDomDocument const& items, QPixmap const& pixmap);
    QString promptUserForTemplateName();
    QString askTemplateName(bool* shouldCancel);
    void deleteTemplates(QModelIndexList const& indexes);
    void updatePreview(const QModelIndex& current);
    void updateZoom();

    Ui::ItemTemplatesWidget *ui;
    class ItemTemplatesModel* const _model;
    int const _sliderValuesFactor = 10;
};

#endif // ITEM_TEMPLATES_WIDGET_H
