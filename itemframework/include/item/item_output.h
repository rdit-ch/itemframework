#ifndef ITEM_OUTPUT_H
#define ITEM_OUTPUT_H

#include "item/abstract_item_input_output_base.h"
#include "appcore.h"

#include <QObject>
#include <QRectF>
#include <QList>

class AbstractItem;
class ItemInput;
class ItemOutputPrivate;

/**
 * @brief The ItemOutput class represents an output that belongs to an item.
 *
 * An output can be connected to zero or more inputs which may read its data.
 *
 * An outputs transport type specifies what type of data it supports. Only inputs
 * and outputs that have the same transport type may be connected to each other.
 * See the \a AbstractItem class for a description of the transport type.
 *
 * \sa Item
 * \sa ItemInput
 * \sa ItemInputOutputBase
 */
class ITEMFRAMEWORK_EXPORT ItemOutput : public AbstractItemInputOutputBase
{
    Q_OBJECT

    friend ItemInput;
    friend AbstractItem;

public:
    /**
     * @brief Constructs an ItemOutput
     * @param owner The Item this output belongs to
     * @param transportType The type of data this output provides
     * @param description A description of this output that is shown to the user
     * @param shape The shape of this output
     */
    ItemOutput(AbstractItem* owner, int transportType, QString const& description,
               QRectF const& shape = {0, 0, 0, 0});

    /**
     * @return true if this output is connected to at least one input, false
     * otherwise
     *
     * \sa connectInput
     * \sa disconnectInput
     * \sa disconnectInputs
     * \sa inputs
     */
    virtual bool isConnected() const;

    /**
     * @return A list of pointers to the inputs this output is connected to. The
     * list is empty iff this output is not connected to any inputs
     *
     * \sa connectInput
     * \sa disconnectInput
     * \sa disconnectInputs
     */
    QList<ItemInput*> inputs() const;

    /**
     * @return A pointer to the data provided by this output
     */
    QObject* data() const;

    virtual QPointF scenePosition() const;

    /**
     * @brief Disconnects the input \a input from this output
     *
     * \sa disconnectInputs
     * \sa connectInput
     */
    void disconnectInput(ItemInput* input);

    /**
     * @brief Disconnects all connected inputs from this output
     *
     * \sa disconnectInput
     * \sa connectInput
     */
    void disconnectInputs();

    /**
     * @brief Connects this output to the input \a input
     * @param input The input this output is to be connected to
     * @return true upon success, false otherwise. The function fails if a NULL
     * output is provided, or if the types of data of the output and input do not
     * match, or if the input is already connected to an output, or if this output
     * is already connected to the input \a input
     *
     * \sa disconnectInput
     * \sa disconnectInputs
     */
    bool connectInput(ItemInput* input);

protected:
    /**
     * @brief Sets the data provided by this output to \a data.
     * Causes this output and the connected inputs to emit the dataChanged signal
     * @param data A pointer to the new data
     *
     * \sa dataChanged
     */
    void setData(QObject* data);

signals:
    /**
     * @brief This signal is emitted whenever an input this output is connected
     * to is disconnected
     *
     * \sa inputConnected
     */
    void inputDisconnected();

    /**
     * @brief This signal is emitted whenever a new input is connected to this
     * output
     *
     * \sa inputDisconnected
     */
    void inputConnected();

    /**
     * @brief This signal is emitted whenever the data provided by this output
     * has changed
     *
     * \sa setData
     */
    void dataChanged();

private:
    Q_DECLARE_PRIVATE(ItemOutput)
    ItemOutputPrivate* const d_ptr;
};

#endif // ITEM_OUTPUT_H
