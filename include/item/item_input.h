#ifndef ITEM_INPUT_H
#define ITEM_INPUT_H

#include "item/abstract_item_input_output_base.h"
#include "appcore.h"

#include <QObject>
#include <QRectF>

class AbstractItem;
class ItemOutput;
class ItemInputPrivate;

/**
 * @brief The ItemInput class represents an input that belongs to an item.
 *
 * An input can be connected to zero or one output from which it receives data.
 *
 * An inputs transport type specifies what type of data it supports. Only inputs
 * and outputs that have the same transport type may be connected to each other.
 * See the \a AbstractItem class for a description of the transport type.
 *
 * \sa Item
 * \sa ItemOutput
 * \sa ItemInputOutputBase
 */
class CORE_EXPORT ItemInput : public AbstractItemInputOutputBase
{
    Q_OBJECT

    friend ItemOutput;

public:
    /**
     * @brief Constructs an ItemInput
     * @param owner The Item this input belongs to
     * @param transportType The type of data this input accepts
     * @param description A description of this input that is shown to the user
     * @param shape The shape of this input
     */
    ItemInput(AbstractItem* owner, int transportType, QString const& description,
              QRectF const& shape = {0, 0, 0, 0});

    /**
     * @return true if this input is connected to an output, false otherwise
     *
     * \sa connectOutput
     * \sa disconnectOutput
     */
    virtual bool isConnected() const;

    /**
     * @return A pointer to the output this input is connected to, or NULL if
     * this input is not connected to any output.
     *
     * \sa connectOutput
     * \sa disconnectOutput
     */
    ItemOutput* output() const;

    /**
     * @return A pointer to the data provided by this input
     */
    QObject* data() const;

    virtual QPointF scenePosition() const;

    /**
     * @brief Disconnects the output this input is connected to, if any
     *
     * \sa connectOutput
     * \sa output
     */
    void disconnectOutput();

    /**
     * @brief Connects this input to the output \a output
     * @param output The output this input is to be connected to
     * @return true upon success, false otherwise. The function fails if this
     * input is alread connected to an output, a NULL output is provided or the
     * types of data of the input and output do not match.
     *
     * \sa disconnectOutput
     * \sa output
     */
    bool connectOutput(ItemOutput* output);

signals:
    /**
     * @brief This signal is emitted whenever the output this input is connected
     * to is disconnected
     *
     * \sa outputConnected
     */
    void outputDisconnected();

    /**
     * @brief This signal is emitted whenever this input is connected to a new
     * output
     *
     * \sa outputDisconnected
     */
    void outputConnected();

    /**
     * @brief This signal is emitted whenever the data provided by this input has
     * changed
     */
    void dataChanged();

private:
    ItemInputPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(ItemInput)
};

#endif // ITEM_INPUT_H
