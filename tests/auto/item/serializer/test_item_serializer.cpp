#include "test_item_serializer.h"

#include "item/item_serializer.h"
#include "item/item_connector.h"
#include "item/item_input.h"
#include "item/item_output.h"
#include "item/item_note.h"
#include "some_item.h"
#include "some_transporter.h"

#include <QApplication>

template <typename T>
QList<T*> findItems(QList<QGraphicsItem*>& items)
{
    auto toType = [](QGraphicsItem* graphicsItem) {
        return qobject_cast<T*>(graphicsItem->toGraphicsObject());
    };

    QList<T*> itemsOfType{};
    std::for_each(items.cbegin(), items.cend(), [&itemsOfType, toType]
                  (QGraphicsItem* graphicsItem) {
        if (auto item = toType(graphicsItem)) {
            itemsOfType.append(item);
        }
    });

    return itemsOfType;
}

template <typename T>
int countItems(QList<QGraphicsItem*>& items)
{
    return findItems<T>(items).length();
}


////////////////////////////////////////////////////////////////////////////////
// single item
////////////////////////////////////////////////////////////////////////////////
void test_ItemScene::testSingleItemSaveSucceeds()
{
    QCOMPARE(singleItem_.saveSuccess_, true);
}

void test_ItemScene::testSingleItemLoadSucceeds()
{
    QCOMPARE(singleItem_.loadSuccess_, true);
}

void test_ItemScene::testSingleItemSameCount()
{
    QCOMPARE(singleItem_.itemsBeforeSave_.length(),
             singleItem_.itemsAfterLoad_ .length());
}


void test_ItemScene::maybeProcessItem(QGraphicsItem* graphicsItem, std::function<void(SomeItem*)> f) {
    if (auto item = qobject_cast<SomeItem*>(graphicsItem->toGraphicsObject())) {
        f(item);
    }
}

QString test_ItemScene::getName(QGraphicsItem* graphicsItem) {
    QString name{};
    maybeProcessItem(graphicsItem, [&name](SomeItem* item) {
        name = item->_name;
    });
    return name;
}

int test_ItemScene::getNumber(QGraphicsItem* graphicsItem) {
    int number = -1;
    maybeProcessItem(graphicsItem, [&number](SomeItem* item) {
        number = item->_number;
    });
    return number;
}

// Note: the check for multiple items being equal is covered in testMultipleItemsConnectionsRestored.
void test_ItemScene::testSingleItemEqual()
{
    QVERIFY2(singleItem_.itemsBeforeSave_.length() > 0, "items before save is empty!");
    QVERIFY2(singleItem_.itemsAfterLoad_ .length() > 0, "items after load is empty!");

    auto const& nameBeforeSave = getName(singleItem_.itemsBeforeSave_.first());
    auto const& nameAfterLoad  = getName(singleItem_.itemsAfterLoad_ .first());

    auto const numberBeforeSave = getNumber(singleItem_.itemsBeforeSave_.first());
    auto const numberAfterLoad  = getNumber(singleItem_.itemsAfterLoad_ .first());

    QVERIFY2(!nameBeforeSave.isNull(), "name before save is NULL");
    QVERIFY2(!nameAfterLoad .isNull(), "name after load is NULL");

    QVERIFY2(numberBeforeSave != -1, "number before save is -1");
    QVERIFY2(numberAfterLoad  != -1, "number after load is -1");

    QCOMPARE(nameBeforeSave, nameAfterLoad);
    QCOMPARE(numberBeforeSave, numberAfterLoad);
}

////////////////////////////////////////////////////////////////////////////////
// multiple items
////////////////////////////////////////////////////////////////////////////////
void test_ItemScene::testMultipleItemsSaveSucceeds()
{
    QCOMPARE(singleItem_.saveSuccess_, true);
}

void test_ItemScene::testMultipleItemsLoadSucceeds()
{
    QCOMPARE(singleItem_.loadSuccess_, true);
}

void test_ItemScene::testMultipleItemsSameCount()
{
    QCOMPARE(multipleItems_.itemsBeforeSave_.length(),
             multipleItems_.itemsAfterLoad_ .length());
}

void test_ItemScene::testMultipleItemsSameCountPerType()
{
    QCOMPARE(countItems<AbstractItem>(multipleItems_.itemsBeforeSave_),
             countItems<AbstractItem>(multipleItems_.itemsAfterLoad_));

    QCOMPARE(countItems<Item_Connector>(multipleItems_.itemsBeforeSave_),
             countItems<Item_Connector>(multipleItems_.itemsAfterLoad_));

    QCOMPARE(countItems<ItemNote>(multipleItems_.itemsBeforeSave_),
             countItems<ItemNote>(multipleItems_.itemsAfterLoad_));
}

void test_ItemScene::testMultipleItemsConnectionsRestored()
{
    // This code could be much more generic by simply finding all pairs of connected
    // inputs and outputs in the itemsBeforeSave list, and verify they are still
    // connected after saving and loading.
    // At this time though it is not worth the effort.

    auto findFirstItemNamed = [](QString const& name, QList<SomeItem*> items)
    -> SomeItem* {
        auto hasName = [name](SomeItem* item) {
            return item->_name == name;
        };

        auto it = std::find_if(items.cbegin(), items.cend(), hasName);
        if (it == items.cend()) {
            return nullptr;
        }

        return *it;
    };

    auto getTheTwoItems = [findFirstItemNamed](MultipleItemsResult result) {
        auto someItems = findItems<SomeItem>(result.itemsAfterLoad_);
        auto itemA2 = findFirstItemNamed(result.itemA->_name, someItems);
        auto itemB2 = findFirstItemNamed(result.itemB->_name, someItems);
        return std::make_pair(itemA2, itemB2);
    };

    SomeItem* itemA2;
    SomeItem* itemB2;
    std::tie(itemA2, itemB2) = getTheTwoItems(multipleItems_);

    QVERIFY2(itemA2 != nullptr, "Unable to find saved item A after load");
    QVERIFY2(itemB2 != nullptr, "Unable to find saved item B after load");

    auto connectors = findItems<Item_Connector>(multipleItems_.itemsAfterLoad_);
    QCOMPARE(connectors.length(), 1);
    auto connector = connectors.first();

    QCOMPARE(itemA2->outputs().length(), 1);
    QCOMPARE(itemB2->inputs() .length(), 1);

    auto output = itemA2->outputs().first();
    auto input  = itemB2->inputs() .first();

    QCOMPARE(connector->get_input(),  input);
    QCOMPARE(connector->get_output(), output);

    QCOMPARE(output->isConnected(), true);
    QCOMPARE(input->isConnected(),  true);

    QCOMPARE(input->output(),       output);
}

void test_ItemScene::initTestCase()
{
    int argc = 1;
    char argv0[] = "testApplication";
    char** argv = new char*[1]{argv0};
    QApplication a{argc, argv};

    // required, otherwise initialization will be canceled.
    QApplication::setOrganizationName("Testing Organisation");
    QApplication::setOrganizationDomain("testing.example.com");
    QApplication::setApplicationName("Testing Application");
    QApplication::setApplicationDisplayName("Testing Application 1.0");
    QApplication::setApplicationVersion("1.0-testing");

    // NOTE: Since the GuiManager is set to the Headless mode in the TestComponent,
    //       the application will exit after loading the environment required for
    //       our tests, and our tests will be run.
    a.exec();

    singleItem_    = initSingleItemTestCase();
    multipleItems_ = initMultipleItemsTestCase();
}

SingleItemResult test_ItemScene::initSingleItemTestCase()
{
    SingleItemResult result;
    QDomDocument document{};
    auto element = document.createElement("testItem");

    auto itemA = new SomeItem{"itemA", 42};
    result.itemsBeforeSave_.append(itemA);
    result.saveSuccess_ = ItemSerializer::saveToXml(document, element,
                                               reinterpret_cast<QList<QGraphicsItem const*>&>(
                                                   result.itemsBeforeSave_));

    auto const connectIO = true;

    result.loadSuccess_ = ItemSerializer::loadFromXml(element, &result.itemsAfterLoad_, ProgressReporter{}, connectIO);

    return result;
}

MultipleItemsResult test_ItemScene::initMultipleItemsTestCase()
{
    auto connectItems = [](SomeItem* itemA, SomeItem* itemB) {
        itemA->outputs().first()->connectInput(itemB->inputs().first());
        itemB->inputs().first()->connectOutput(itemA->outputs().first());
        return new Item_Connector{itemA->outputs().first(),
                                        itemB->inputs().first()};
    };

    MultipleItemsResult result;
    QDomDocument document{};
    auto element = document.createElement("testItems");

    auto itemA     = new SomeItem{"itemA", 42};
    auto itemB     = new SomeItem{"itemB", 43};
    auto connector = connectItems(itemA, itemB);
    auto note = new ItemNote{};

    result.itemA     = itemA;
    result.itemB     = itemB;
    result.connector = connector;
    result.note      = note;

    result.itemsBeforeSave_.append(itemA);
    result.itemsBeforeSave_.append(itemB);
    result.itemsBeforeSave_.append(connector);
    result.itemsBeforeSave_.append(note);

    result.saveSuccess_ = ItemSerializer::saveToXml(document, element,
                                               reinterpret_cast<QList<QGraphicsItem const*>&>(
                                               result.itemsBeforeSave_));

    auto const connectIO = true;

    result.loadSuccess_ = ItemSerializer::loadFromXml(element, &result.itemsAfterLoad_, ProgressReporter{}, connectIO);

    return result;
}

QTEST_APPLESS_MAIN(test_ItemScene)
