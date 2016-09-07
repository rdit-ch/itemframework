/*******************************************************************************
 * \brief       Project_Manager.
 *
 *              Provides all functions to load and use a source plugin
 *              (Level_0). This file is part of the Project TraViz3.
 * \version     1.0
 * \class       Project_Manager
 * \headerfile  Project_Manager.h
 * \file        Project_Manager.cpp
 * \author      Marcus Pfaff (marcus.pfaff@ruag.com)
 * \date        03-12-2012
 * \copyright   (C)2013 RUAG Defence. All rights reserved.
 ******************************************************************************/

#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QtWidgets>

#include "appcore.h"
#include "gui/gui_manager.h"

class AbstractProject;
class Project_Handler_Window;

class CORE_EXPORT Project_Manager : public QObject, public Singleton<Project_Manager>
{
    //------------------Singleton Stuff---------------------------
    Q_OBJECT
    Q_INTERFACES(AbstractSingleton)

    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "GuiManager")

public:
    Q_INVOKABLE Project_Manager();
    ~Project_Manager();

protected:
    bool postInit();
    bool preDestroy();
    //------------------End Singleton Stuff---------------------------

public:
    void            add_project(AbstractProject* project);

private:
    Project_Handler_Window*             project_handler_window;
    QVector<AbstractProject*>                   projects;
    bool                                unload_all();
protected:

public slots:
    AbstractProject*            create_new_project();
    AbstractProject*            open_project();
private slots:

signals:
};
#endif // PROJECT_MANAGER_H
//eof
