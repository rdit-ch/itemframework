#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QMutex>
#include "console_message.h"
#include <set>

class ConsoleModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    /**
     * @brief The ModelColumn enum defines the Names and the order of the Models Columns
     */
    enum ModelColumn {Type, /*<! The type of the Message. \sa ConsoleMessage::MsgType */
                      Message, /*<! The content of the Message */
                      Category,  /*<! The category of the Message. See QMessageLogger for more info about categories. */
                      Location,  /*<! The location (file & function) where the message occoured. */
                      Time,  /*<! The time when the messsage occoured. */
                      Thread, /*<! The thread where the message was created in */
                      NumberOfColumns /*<! Not a real Column, but the Number of Columns in this enum. Do not insert new columns after this entry */
                     };
    Q_ENUM(ModelColumn)

    /**
     * @brief The ModelRole enum defines the Custom Roles that are used by this model
     */
    enum ModelRole {
        ConsoleMessageRole = Qt::UserRole /*<! This Role represents a ConsoleMessage Object (by value). */
    };


    /**
     * @brief Returns the one and only instance of the ConsoleModel which was automatically created during startup.
     */
    static ConsoleModel* instance();

    /**
     * @brief Appends a new ConsoleMessage to the model. This Method is Threadsafe.
     * @param msg
     */
    void appendMessage(const ConsoleMessage msg);

    ConsoleModel();
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool removeRows(int row, int count, const QModelIndex& parent);

signals:
    /**
     * @brief Internal signal, which is used to ensure that the new messages are inserted from the Object owning thread.
     * Use appendMessage() instead of this signal.
     * @param msg The new message to insert.
     */
    void consoleMessageCreated(const ConsoleMessage msg);

private:

    /**
     * @brief Comperator used with std::multiset to sort the ConsoleMessages by time (ascending)
     */
    struct MsgComperator : std::binary_function <ConsoleMessage, ConsoleMessage, bool> {
        bool operator()(const ConsoleMessage& a, const ConsoleMessage& b)
        {
            return a.time() < b.time();
        }
    };

    /**
     * @brief Buffer for the Console Messages.
     * Can be cleared by calling removeRows().
     */
    std::multiset<ConsoleMessage, MsgComperator> _msgs;
    using set_iterator = std::multiset<ConsoleMessage, MsgComperator>::iterator;
private slots:
    void insertMessageIntoModel(const ConsoleMessage msg);
};

#endif // CONSOLEMODEL_H
