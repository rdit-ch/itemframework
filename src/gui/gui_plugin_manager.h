#ifndef GUI_PLUGIN_MANAGER_H
#define GUI_PLUGIN_MANAGER_H

#include <QDialog>

class PluginTableModel;

namespace Ui
{
    class GuiPluginManager;
}

class GuiPluginManager : public QDialog
{
    Q_OBJECT
public:
    explicit GuiPluginManager(QWidget* parent = 0);
    ~GuiPluginManager();

protected:
    void accept();
    void reject();

private:
    Ui::GuiPluginManager* _ui = nullptr;
    PluginTableModel* _pluginTableModel = nullptr;
    QString _lastUsedPath;

private slots:
    void onAddPluginPath();
    void onRemovePluginPath();
};

#endif // GUI_PLUGIN_MANAGER_H
