#include "console_message.h"
#include <QThread>
#include <QMetaEnum>
#include <QColor>

QColor ConsoleMessage::colorForMsgType(ConsoleMessage::MsgType type)
{
    static QHash<MsgType, QColor> colors({
        {Debug, QColor("limegreen")},
        {Warning, QColor("orange")},
        {Critical, QColor("orangered")},
        {Fatal, Qt::red},
        {Info, Qt::blue}
    });

    return colors[type]; //Return color for type, or black if type not in hashmap.
}

ConsoleMessage::ConsoleMessage(QtMsgType type, const QMessageLogContext& cont, const QString& msg)
{
    _type = (MsgType)type;
    _message = msg;
    _time = QDateTime::currentDateTime();
    _threadId = QThread::currentThreadId();
    _category = cont.category;
    _location = QString("%1:%2 %3").arg(cont.file).arg(cont.line).arg(cont.function);
    _valid = true;
}

ConsoleMessage::ConsoleMessage()
{
    _valid = false;
}

const QString ConsoleMessage::message() const
{
    return _message;
}

ConsoleMessage::MsgType ConsoleMessage::type() const
{
    return _type;
}

const QString ConsoleMessage::category() const
{
    return _category;
}

const QString ConsoleMessage::location() const
{
    return _location;
}

Qt::HANDLE ConsoleMessage::threadId() const
{
    return _threadId;
}

const QDateTime ConsoleMessage::time() const
{
    return _time;
}

bool ConsoleMessage::isValid()
{
    return _valid;
}
