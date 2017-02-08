#ifndef ITEMTEMPLATESVIEW_H
#define ITEMTEMPLATESVIEW_H

#include <QListView>

class ItemTemplatesView : public QListView
{
    Q_OBJECT

public:
    explicit ItemTemplatesView(QWidget* parent = nullptr);

protected:
    virtual void startDrag(Qt::DropActions supportedActions);
};

#endif // ITEMTEMPLATESVIEW_H
