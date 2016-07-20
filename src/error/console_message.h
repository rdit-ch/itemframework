#ifndef CONSOLE_MESSAGE_H
#define CONSOLE_MESSAGE_H

#include <QString>
#include <QDateTime>
#include <QMessageLogContext>
#include <QObject>

class ConsoleMessage
{
    Q_GADGET
public:
    /**
     * @brief The MsgType enum represents all possible Message Types.
     * This enum is a copy of the QtMsgType enum, but with Q_ENUM support.
     */
    enum MsgType {
        Debug = QtDebugMsg,
        Warning = QtWarningMsg,
        Info = QtInfoMsg,
        Fatal = QtFatalMsg,
        Critical = QtCriticalMsg
    };
    Q_ENUM(MsgType)

    /**
     * @brief Returns the foreground color for the passed message type. (e.g green for debug, red for error)
     * @param type The type you want to get the color of
     * @return The assoicated color
     */
    static QColor colorForMsgType(MsgType type);

    /**
     * @brief Constructs a ConsoleMessage for the current Thread & Datetime.
     * @param type The type of the message
     * @param cont The context of the message
     * @param msg The content of the message
     */
    ConsoleMessage(QtMsgType type, const QMessageLogContext& cont, const QString& msg);

    /**
     * @brief Constructs an invalid ConsoleMessage
     */
    ConsoleMessage();

    /**
     * @brief The content of the message (= the message itself)
     */
    const QString message() const;

    /**
     * @brief The message type (debug, info, error,...)
     */
    MsgType type() const;

    /**
     * @brief The message category
     */
    const QString category() const;

    /**
     * @brief Th message location (file, line, function)
     */
    const QString location() const;

    /**
     * @brief The thread from which the message is originating
     */
    Qt::HANDLE threadId() const;

    /**
     * @brief The time when the message was created
     */
    const QDateTime time() const;

    /**
     * @brief Whether or not this is object holds a valid message
     */
    bool isValid();

private:
    QString _message;
    MsgType _type;
    QString _category;
    QString _location;
    Qt::HANDLE _threadId;
    QDateTime _time;
    bool _valid;
};

#endif // CONSOLE_MESSAGE_H

