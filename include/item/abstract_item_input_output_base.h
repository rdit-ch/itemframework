#ifndef ABSTRACT_ITEM_INPUT_OUTPUT_BASE_H
#define ABSTRACT_ITEM_INPUT_OUTPUT_BASE_H

#include <QGraphicsObject>
#include <QRectF>
#include "appcore.h"

/**
 * @brief The AbstractItemInputOutputBase class provides common functionality for the
 * ItemInput and ItemOutput classes.
 */
class CORE_EXPORT AbstractItemInputOutputBase : public QGraphicsObject
{
    Q_OBJECT

    friend class AbstractItemPrivate;
    friend class AbstractItem;

public:
    /**
     * @brief Constructs an AbstractItemInputOutputBase
     * @param parent The AbstractItem this object belongs to
     * @param transportType The type of data this object handles
     * @param description A description of this object that is shown to the user
     * @param shape The shape of this object
     */
    AbstractItemInputOutputBase(class AbstractItem* parent, int transportType, QString const& description, QRectF const& shape);

    virtual ~AbstractItemInputOutputBase();

    /**
     * @brief Returns the type of data this object handles
     * @return The type of data this object handles
     */
    int transportType() const;

    /**
     * @brief Returns the owner of this object
     * @return The owner of this object
     */
    AbstractItem* owner() const;

    /**
     * @brief Returns the data provided by this object
     * @return The data provided by this object
     */
    virtual QObject* data() const = 0;

    /**
     * @brief Returns the local position of this object
     * @return The local position of this object
     *
     * \sa scenePosition
     */
    virtual QPointF localPosition() const;

    /**
     * @brief Returns the scene position of this object
     * @return The scene position of this object
     *
     * \sa localPosition
     */
    virtual QPointF scenePosition() const;

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget* widget);

    /**
     * @brief This is a virtual pure function. It is used to determine whether
     * or not this input or output is currently connected
     * @return true if the object is connected, false otherwise
     */
    virtual bool isConnected() const = 0;

protected:
    /**
     * @brief Sets the local position of this object
     * @param position The new center position of this object
     *
     * \sa positionChanged
     */
    void setLocalPosition(QPointF const& position);

    /**
     * @brief Updates the scene position. Causes this objects position within
     * the item scene to be updated
     *
     * \sa positionChanged
     */
    void updateScenePosition();

signals:
    /**
     * @brief This signal is emitted whenever this objects local or scene
     * position has changed
     *
     * \sa updateScenePosition
     */
    void positionChanged();

private:
    QScopedPointer<class AbstractItemInputOutputBasePrivate> const d_ptr;
    Q_DECLARE_PRIVATE(AbstractItemInputOutputBase)
};

#endif // ABSTRACT_ITEM_INPUT_OUTPUT_BASE_H
