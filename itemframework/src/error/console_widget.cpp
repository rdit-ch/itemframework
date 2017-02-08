#include "console_widget.h"
#include "ui_console_widget.h"
#include <QMenu>
#include <QClipboard>
#include <QMetaEnum>
#include "console_message.h"
#include "console_model.h"

ConsoleWidget::ConsoleWidget(QAbstractTableModel* model):
    ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);
    _proxyModel.setSourceModel(model);
    _proxyModel.setFilterKeyColumn(-1); //filter on all columns
    _proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive); //case intensitive
    ui->tvOutput->setModel(&_proxyModel);
    ui->tvOutput->horizontalHeader()->setSectionsMovable(true);
    ui->tvOutput->verticalHeader()->setDefaultSectionSize(15);
    ui->tvOutput->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(ui->tvOutput, &QTableView::customContextMenuRequested, this, &ConsoleWidget::tableContextMenu);
}

ConsoleWidget::~ConsoleWidget()
{
    delete ui;
}

void ConsoleWidget::filterChanged(QString text)
{
    _proxyModel.setFilterWildcard(text);
}

void ConsoleWidget::clearPressed()
{
    _proxyModel.removeRows(0, _proxyModel.rowCount());
}

void ConsoleWidget::tableContextMenu(QPoint pos)
{
    QModelIndex itemIndex = ui->tvOutput->indexAt(pos);    // Nothing to do if invalid

    if (!itemIndex.isValid()) {
        return;
    }

    QMenu contextMenu(tr("Context Menu"), ui->tvOutput);
    QAction* cpyMsg = contextMenu.addAction(tr("Copy Message"));
    QAction* cpyMsgLoc = contextMenu.addAction(tr("Copy Message && Location"));
    QAction* cpyMsgFull = contextMenu.addAction(tr("Copy entire Entry"));
    QAction* delMsg = contextMenu.addAction(tr("Delete Entry"));
    QAction* selectedAction = contextMenu.exec(ui->tvOutput->viewport()->mapToGlobal(pos));

    if (selectedAction == delMsg) { //delete message?
        ui->tvOutput->model()->removeRow(itemIndex.row(), itemIndex.parent());
    } else { //copy to clipboard
        const ConsoleMessage msg = itemIndex.data(ConsoleModel::ConsoleMessageRole).value<ConsoleMessage>();
        QString cpContent;

        if (selectedAction == cpyMsg) { //copy only message
            cpContent = msg.message();
        } else if (selectedAction == cpyMsgLoc) { //copy message + location
            cpContent = QString("%1 %2").arg(msg.location(), msg.message());
        } else if (selectedAction == cpyMsgFull) { //copy full row
            cpContent = QString("%1 %2/%3 %4 0x%5 %6")
                        .arg(msg.time().toString("d.MM.yyyy hh:mm:ss.z"))
                        .arg(QMetaEnum::fromType<ConsoleMessage::MsgType>().valueToKey(msg.type()))
                        .arg(msg.category())
                        .arg(msg.location())
                        .arg((quint64)(msg.threadId()), 0, 16) //hex
                        .arg(msg.message());
        }


        QApplication::clipboard()->setText(cpContent);
    }
}
