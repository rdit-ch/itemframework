#include "ui_gui_plugin_manager.h"
#include "gui/gui_plugin_manager.h"
#include "gui/gui_manager.h"
#include "plugin/plugin_manager.h"
#include "plugin/plugin_table_model.h"
#include <QTableWidgetItem>

GuiPluginManager::GuiPluginManager(QWidget* parent) :
    QDialog(parent), _ui(new Ui::GuiPluginManager)
{
    _ui->setupUi(this);
    // show ui on action
    QObject::connect(Gui_Manager::instance()->get_action("Plugins"), SIGNAL(triggered()), this, SLOT(exec()));
    // Buttons plugins
    QObject::connect(_ui->pushButton_edit,         SIGNAL(clicked(bool)), this, SLOT(onEditPluginPath()));
    QObject::connect(_ui->pushButton_cancelPlugin, SIGNAL(clicked(bool)), this, SLOT(close()));

    // create table and connections
    PluginTableModel* _tableModel = new PluginTableModel(this);
    _ui->tableView_plugins->setModel(_tableModel);
    _tableModel->appendPluginMetaDataList(PluginManager::instance()->pluginMetaDataList());
    _ui->tableView_plugins->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QObject::connect(_tableModel, SIGNAL(pluginMetaDataChanged(PluginMetaData*)), PluginManager::instance(), SLOT(serializePluginMetaData(PluginMetaData*)));
}

GuiPluginManager::~GuiPluginManager()
{
    delete _ui;
}

void GuiPluginManager::onEditPluginPath()
{
    bool success;
    QString path = PluginManager::instance()->pluginPath().join(";");
    QString text = QInputDialog::getText(this, tr("Plugin path"), tr("Plugin path"), QLineEdit::Normal, path, &success);

    if (success && !text.isEmpty()) {
        PluginManager::instance()->setPluginPath(text);
    }
}

