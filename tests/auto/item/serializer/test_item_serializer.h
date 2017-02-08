#ifndef TEST_ITEM_SCENE_H
#define TEST_ITEM_SCENE_H

#include <QObject>
#include <QtTest/QTest>
#include <functional> // std::function

struct SingleItemResult
{
    bool                        saveSuccess_{false};
    bool                        loadSuccess_{false};
    QList<class QGraphicsItem*> itemsBeforeSave_{};
    QList<class QGraphicsItem*> itemsAfterLoad_{};
};

struct MultipleItemsResult
{
    bool                        saveSuccess_{false};
    bool                        loadSuccess_{false};
    QList<class QGraphicsItem*> itemsBeforeSave_{};
    QList<class QGraphicsItem*> itemsAfterLoad_{};

    class SomeItem*             itemA;
    class SomeItem*             itemB;
    class Item_Connector*       connector;
    class ItemNote*             note;
};

class test_ItemScene : public QObject
{
    Q_OBJECT

public:

private slots:
    void initTestCase();
    SingleItemResult    initSingleItemTestCase();
    MultipleItemsResult initMultipleItemsTestCase();

    // single item
    void testSingleItemSaveSucceeds();
    void testSingleItemLoadSucceeds();
    void testSingleItemSameCount();
    void testSingleItemEqual();

    // multiple items
    void testMultipleItemsSaveSucceeds();
    void testMultipleItemsLoadSucceeds();
    void testMultipleItemsSameCount();
    void testMultipleItemsSameCountPerType();
    void testMultipleItemsConnectionsRestored();

private:
    QString getName    (class QGraphicsItem *graphicsItem);
    int     getNumber  (class QGraphicsItem *graphicsItem);
    void    maybeProcessItem(class QGraphicsItem *graphicsItem, std::function<void (class SomeItem *)> f);

    SingleItemResult    singleItem_;
    MultipleItemsResult multipleItems_;
};

#endif // TEST_ITEM_SCENE_H
