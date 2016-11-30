#include "ui_gui_plugin_manager.h"
#include "gui/gui_plugin_manager.h"
#include "gui/gui_manager.h"
#include "plugin/plugin_manager.h"
#include "plugin/plugin_table_model.h"
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QStandardPaths>

GuiPluginManager::GuiPluginManager(QWidget* parent) :
    QDialog(parent), _ui(new Ui::GuiPluginManager)
{
    _ui->setupUi(this);
    // show ui on action
    QAction* showPluginManager = GuiManager::instance()->action("actionPlugins");
    connect(showPluginManager, &QAction::triggered, this, &QDialog::exec);

    // create table and connections
    _pluginTableModel = new PluginTableModel();
    _pluginTableModel->appendPluginMetaDataList(PluginManager::instance()->pluginMetaDataList());
    _ui->tableViewPlugins->setModel(_pluginTableModel);
    _ui->tableViewPlugins->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(_pluginTableModel, &PluginTableModel::pluginMetaDataChanged,
            PluginManager::instance(), &PluginManager::serializePluginMetaData);

    _ui->treeWidgetPluginPath->clear();
    for(QString path : PluginManager::instance()->pluginPath()){
        QTreeWidgetItem* item = new QTreeWidgetItem(_ui->treeWidgetPluginPath);
        item->setText(0, path);
        _ui->treeWidgetPluginPath->addTopLevelItem(item);
    }
}

GuiPluginManager::~GuiPluginManager()
{
    _ui->treeWidgetPluginPath->clear();
    delete _ui;
    delete _pluginTableModel;
}

void GuiPluginManager::accept()
{
    QStringList paths;
    for(int i=0; i < _ui->treeWidgetPluginPath->topLevelItemCount(); ++i){
        paths.append(_ui->treeWidgetPluginPath->topLevelItem(i)->text(0));
    }
    PluginManager::instance()->setPluginPath(paths);
    QDialog::accept();
}

void GuiPluginManager::reject()
{
    QDialog::reject();
}

void GuiPluginManager::onAddPluginPath()
{
    if(_lastUsedPath.isEmpty()){
        QStringList pluginPaths = PluginManager::instance()->pluginPath();
        if(!pluginPaths.isEmpty()){
            _lastUsedPath = pluginPaths.first();
        } else {
            _lastUsedPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
    }
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the plugin folder"),
                                                    _lastUsedPath,
                                                    QFileDialog::DontResolveSymlinks);
    QFileInfo dirInfo(dir);
    if(!dirInfo.isDir()){
        return;
    }

    for(int i=0; i < _ui->treeWidgetPluginPath->topLevelItemCount(); ++i){
        QTreeWidgetItem* item = _ui->treeWidgetPluginPath->topLevelItem(i);
        if(item->text(0) == dir){
            return;
        }
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(_ui->treeWidgetPluginPath);
    item->setText(0, dir);
    _ui->treeWidgetPluginPath->addTopLevelItem(item);
    _lastUsedPath = dir;
}

void GuiPluginManager::onRemovePluginPath()
{
    QList<QTreeWidgetItem*> items = _ui->treeWidgetPluginPath->selectedItems();
    for(QTreeWidgetItem* item : items){
        int indexItem = _ui->treeWidgetPluginPath->indexOfTopLevelItem(item);
        delete _ui->treeWidgetPluginPath->takeTopLevelItem(indexItem);
    }
}

