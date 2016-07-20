#include "console_model.h"
#include "helper/startup_helper.h"
#include <QBrush>
#include <QMetaEnum>
#include <QMutexLocker>
#include <iterator>

//--------------- Static & Initialization Stuff ----------------------

static QtMessageHandler original_message_handler; //Storage for the original message handler
static ConsoleModel consoleModel; //The instnace of ConsoleModel which will be used by our message handler.

//Our own Message handler (which will be called by Qt)
static void msg_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    original_message_handler(type, context, msg); //forward message to original handler
    consoleModel.appendMessage(ConsoleMessage(type, context, msg)); //append message to model instance (where it will be buffered)
}

static void register_msg_handler()
{
    qRegisterMetaType<ConsoleMessage>();
    original_message_handler = qInstallMessageHandler(msg_handler); //Install our message handler and backup the previous handler
}
Q_COREAPP_STARTUP_FUNCTION(register_msg_handler) //Registers the handler, so that it will be called before QApplication::exec()


ConsoleModel* ConsoleModel::instance()
{
    return &consoleModel; //Return the static instance.
}

//------------------ Class Implementation ------------------------------

ConsoleModel::ConsoleModel()
{
    connect(this, &ConsoleModel::consoleMessageCreated, this, &ConsoleModel::insertMessageIntoModel);
}

bool ConsoleModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count < 1 || row < 0 || (row + count) > rowCount(parent)) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    auto it = std::next(_msgs.begin(), row);

    for (int r = 0; r < count; ++r) {
        it = _msgs.erase(it);
    }

    endRemoveRows();
    return true;
}

void ConsoleModel::insertMessageIntoModel(ConsoleMessage msg)
{
    set_iterator it = _msgs.insert(msg); //insert new message at the correct position, and get the iterator to the new element
    int ind = std::distance(_msgs.begin(), it); //Calculate index of new element
    beginInsertRows(QModelIndex(), ind, ind);
    endInsertRows();
}

void ConsoleModel::appendMessage(const ConsoleMessage msg)
{
    emit consoleMessageCreated(msg);
    //emit signal that is connected to the method above, to ensure that the insertion happens in the Thread which owns this model (GUI-Thread)
}


int ConsoleModel::rowCount(const QModelIndex&) const
{
    return _msgs.size();
}

int ConsoleModel::columnCount(const QModelIndex&) const
{
    return ConsoleModel::NumberOfColumns;
}

QVariant ConsoleModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || (size_t)(index.row()) >= _msgs.size()) { //row out of bounds
        return QVariant();
    }

    const ConsoleMessage& msg = *(std::next(_msgs.cbegin(), index.row()));

    if (role == Qt::ForegroundRole && index.column() == ConsoleModel::Type) { //color for type column
        return QVariant::fromValue(QBrush(ConsoleMessage::colorForMsgType(msg.type())));
    }

    if (role == ModelRole::ConsoleMessageRole) { //ConsoleMessage object requested
        return QVariant::fromValue(msg);
    }

    if (role == Qt::DisplayRole) { //Text requested
        switch (index.column()) {
        case ConsoleModel::Type: {
            const QMetaEnum& me = QMetaEnum::fromType<ConsoleMessage::MsgType>();
            return me.valueToKey(msg.type()); //Return untranslated(!) enum key
        }

        case ConsoleModel::Message:
            return msg.message(); //Return untranslated(!) message

        case ConsoleModel::Category:
            return msg.category();

        case ConsoleModel::Location:
            return msg.location();

        case ConsoleModel::Time:
            return msg.time().toString("hh:mm:ss.z");

        case ConsoleModel::Thread:
            return QString("0x%1").arg(QString::number((quint64)(msg.threadId()), 16));

        }
    }

    return QVariant();
}

QVariant ConsoleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) { //Vertical Label or something else than "text" requested
        return QAbstractTableModel::headerData(section, orientation, role); //Forward to baseclass implementation
    }

    if (section >= 0 && section < ConsoleModel::NumberOfColumns) { //Column in bounds
        const QMetaEnum& me = QMetaEnum::fromType<ConsoleModel::ModelColumn>();
        return tr(me.valueToKey(section)); //Return translated enum key
    }

    return QVariant();
}
