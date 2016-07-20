#ifndef GUI_PLUGIN_MANAGER_H
#define GUI_PLUGIN_MANAGER_H

#include <QWidget>
#include <QDialog>

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
private:
    Ui::GuiPluginManager* _ui;
    bool cleanup();
public slots:
    void onEditPluginPath();
};

#endif // GUI_PLUGIN_MANAGER_H
