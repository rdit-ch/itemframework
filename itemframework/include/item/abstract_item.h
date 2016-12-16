#ifndef ABSTRACT_ITEM_H
#define ABSTRACT_ITEM_H

#include <QGraphicsObject>
#include <QList>
#include "appcore.h"

class AbstractItemInputOutputBase;
class ItemInput;
class ItemOutput;
class SettingsScope;

/**
 * @brief The AbstractItem class is the base class for all items.
 *
 * Inputs and Outputs
 * ------------------
 * An item may have any number of inputs and any number of outputs.
 *
 * \sa ItemInput
 * \sa ItemOutput
 * \sa inputs
 * \sa outputs
 * \sa addInput
 * \sa addOutput
 * \sa clearInputs
 * \sa clearOutputs
 *
 * Progress
 * --------
 * An item may have a progress state between 0 and 100 percent.
 *
 * \sa progress
 * \sa setProgress
 * \sa setProgressAutohide
 *
 * Automatic property serialization
 * --------------------------------
 * The AbstractItem class provides default implementations of the functions save and load
 * that are able to serialize and deserialize Qt property members of the derived
 * class. In order for a property to be serialized and deserialized automatically,
 * it has to be a USER property.
 * Custom types must be made available to the Qt meta type system using
 * Q_DECLARE_METATYPE in order to be serialized and deserialized.
 * Types with a registered string converter can be serialized trivially to an
 * ASCII string.
 * Custom types with no registered string converter are converted to a base64
 * representation.
 *
 * \sa load
 * \sa save
 *
 * Transport Types
 * ---------------
 * Various types of data can be transported between items using inputs and outputs.
 * Every input and output has a transport type which represents the type of data
 * they support. The transport type is defined as the Qt meta type ID of the type
 * to be transported. It can be queried for any type using the qMetaTypeId<T>()
 * template function.
 *
 * Styles
 * ------
 * Every transport type has a style associated with it. Currently, a style is
 * defined simply as a color.
 *
 * \sa connectorStyle
 * \sa registerConnectorStyle
 */
class ITEMFRAMEWORK_EXPORT AbstractItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(SettingsScope* settingsScope READ settingsScope)

public:
    /**
     * @brief Constructs an AbstractItem with the name \a typeName
     * @param typeName The type name of your item
     */
    explicit AbstractItem(QString typeName);

    virtual ~AbstractItem();

    /**
     * @return The type name of this item. It is displayed above the item in bold.
     */
    QString typeName() const;

    /**
     * @return This name of this item. It is displayed below the item.
     *
     * \sa setName
     */
    QString name() const;

    /**
     * @return The item description. It is displayed in the Item_Origin_Visualizer
     * Widgets which are included in some Items. This description should describe
     * the current state of the object (e.g. what the object is doing right now)
     *
     * \sa setDescription
     */
    QString description() const;

    /**
     * @brief Set this items name.
     * @param name the name to set to
     *
     * \sa name
     */
    void setName(QString name);

    /**
     * @brief Returns the bounding box of the item.
     * @return The bounding box of the item.
     */
    QRectF boundingRect() const override;

    /**
     * @brief Returns the graphical shape of the item's image.
     * @return The graphical shape of the item's image.
     */
    QPainterPath shape() const override;

    /**
     * @brief Paints the contents of the item.
     * @param painter The QPainter to be used.
     * @param option Style options for the item.
     * @param widget Optional, points to the widget that is being painted on.
     */
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

    /**
     * @return A list of all the inputs of this item.
     *
     * \sa addInput
     * \sa clearInputs
     */
    QList<ItemInput*> const inputs() const;

    /**
     * @return A list of all the outputs of this item.
     *
     * \sa addOutput
     * \sa clearOutputs
     */
    QList<ItemOutput*> const outputs() const;

    /**
     * @return Get the Image/Icon to display in the center of the item.
     *
     * \sa setImage
     * \sa paintItemBox
     */
    QImage const image() const;

    /**
     * @return The current progress in percent from 0-100, or -1 if no progress
     * is displayed.
     *
     * \sa setProgress
     * \sa paintItemBox
     */
    int progress() const;

    /**
     * @brief Loads persistent settings for this item from given XML \a element.
     *
     * This base implementation will load the item's user properties and settings scope.
     *
     * Note that derived classes that overwrite this method should call the base implementation
     * in order load the persistent settings of the base class.
     *
     * Note that it is not considered an error if a property that is defined in the class
     * (and is saved by the default implementation of save()) is not present (and therefore not
     * loaded) in the given XML element.
     *
     * @param element The QDomElement containing the data to load.
     *
     * @return \c true upon success, \c false otherwise.
     *
     * \sa save
     */
    virtual bool load(class QDomElement& element);

    /**
     * @brief Saves persistent settings for this item into the  given XML \a element.
     *
     * This base implementation will save the item's user properties and settings scope.
     *
     * Note that derived classes that overwrite this method should call the base implementation
     * in order save the persistent settings of the base class.
     *
     * @param document The QDomDocument that contains the \a element.
     * @param element The QDomElement to save the data to.
     *
     * @return \c true upon success, \c false otherwise.
     *
     * \sa load
     */
    virtual bool save(class QDomDocument& document, class QDomElement& element) const;

    /**
     * @brief Returns a pointer to the settings scope associated with this item.
     *
     * @return A pointer to the settings scope associated with this item.
     */
    SettingsScope* settingsScope() const;

    /**
     * @return The height of the item input and output connector port
     *
     * \sa connectorWidth
     */
    static int connectorHeight();

    /**
     * @return The width of the item input and output connector port
     *
     * \sa connectorHeight
     */
    static int connectorWidth();

    /**
     * @brief Register a connector style for a transport type
     * @param pen The pen style to store
     * @param transportType The transport type to store the pen for
     * @return true if a previously stored style was overwritten, false otherwise
     *
     * \sa connectorStyle
     */
    static bool registerConnectorStyle(QColor const& color, int transportType);

    /**
     * @param transportType The transport type whose style is requested
     * @return The QPen style stored for the transport type \a transportType
     *
     * \sa registerConnectorStyle
     */
    static QPen connectorStyle(int transportType);


protected:
    /**
     * @brief Adds an input to this item
     * @param transportType The transport type of the input
     * @param description An optional text description of the input
     * @return A pointer to the newly added input
     *
     * \sa inputs
     * \sa clearInputs
     */
    ItemInput* addInput(int transportType, QString const& description = "");

    /**
     * @brief Adds an output to this item
     * @param transportType The transport type of the output
     * @param description An optional text description of the output
     * @return A pointer to the newly added output
     *
     * \sa outputs
     * \sa remove
     * \sa clearOutputs
     */
    ItemOutput* addOutput(int transportType, QString const& description = "");

    /**
     * @brief Removes an input from this item
     * @param input a pointer to the output that should be removed
     *
     * \sa inputs
     * \sa remove
     * \sa clearInputs
     * \sa addInput
     */
    void remove(ItemInput* input);

    /**
     * @brief Removes an output from this item
     * @param output a pointer to the output that should be removed
     *
     * \sa outputs
     * \sa clearOutputs
     * \sa addOutput
     */
    void remove(ItemOutput* output);

    /**
     * @brief Remove all the inputs from this item
     *
     * \sa inputs
     * \sa addInput
     */
    void clearInputs();

    /**
     * @brief Remove all the outputs from this item
     *
     * \sa outputs
     * \sa addOutput
     */
    void clearOutputs();


    /**
     * @brief Set the description of the item.
     * @param description The description to set
     *
     * \sa description
     */
    void setDescription(QString const& description);

    /**
     * @brief Set the image displayed in the item.
     * @param image The image to set
     *
     * \sa image
     */
    void setImage(QImage const& image);

    /**
     * @brief Set the data of a given output
     * @param output The output whose data to set
     * @param data The data to set the output to
     */
    void setOutputData(ItemOutput* output, QObject* data);

    /**
     * @brief Set the current progress of the item.
     * @param progress The progress in percent from 0-100 or -1 if no progress
     * should be displayed
     *
     * \sa progress
     * \sa paintItemBox
     */
    void setProgress(int progress);

    /**
     * @brief Sets the progress to -1 after a certain amount of time
     * @param ms the number of miliseconds to wait, before setting the progress
     * to -1. Pass a number <= 0 to stop the timer.
     *
     * \sa setProgress
     * \sa progress
     */
    void setProgressAutohide(int ms);

    /**
     * @brief This method is called with an empty QMenu whenever a contextmenu
     * on the current item is requested. Add Actions to Display them in the Menu
     * @param menu The menu to append your items to.
     */
    virtual void contextMenuPrepare(QMenu& menu) const;

    /**
     * @brief A simple default implementation of the mouseDoubleClickEvent.
     * Override to do custom things (e.g. open window)
     * @param event Mouse event context information
     */
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    /**
     * @brief This Method paints the Item Rectangle, the Icon and the progress bar
     * of the item. Override for custom behavior.
     * @param painter The painter to use
     */
    virtual void paintItemBox(QPainter* painter);

    void timerEvent(QTimerEvent* t) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

signals:
    /**
     * @brief This signal should be emitted by the derived class whenever the
     * items state has changed and needs to be saved again.
     */
    void changed();

    /**
     * @brief This signal is emitted whenever the name has changed
     */
    void nameChanged();

    /**
     * @brief This signal is emitted whenever the description has changed.
     */
    void descriptionChanged();

    /**
     * @brief This signal is emitted whenever the progress has changed
     */
    void progressChanged();

public slots:
    /**
     * @brief Disconnects all connections from/to the item.
     */
    void disconnectConnections();

private:
    QScopedPointer<class AbstractItemPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(AbstractItem)

    friend class Item_Origin_Visualizer_Entry;
};

#endif // ABSTRACT_ITEM_H
