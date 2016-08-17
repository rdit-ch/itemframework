# Set project properties
QT          +=  gui xml sql widgets
CONFIG      +=  c++11
TEMPLATE    =   lib
VERSION     +=  0.1
DEFINES     +=  API_VERSION=\\\"$$VERSION\\\" #Version of the Public api (equal to lib version^^)
DEFINES     +=  CORE_BUILD=1 #used for CORE_EXPORT macro switch in appcore.h

SOURCE_VERSION = $$system(git describe --always --tags) #Source Code version
DEFINES     += SOURCE_VERSION=\\\"$$SOURCE_VERSION\\\"

QMAKE_CXXFLAGS +=  -fvisibility=hidden  #to enforce usage of CORE_EXPORT

SOURCES     +=  \
                src/appcore.cpp \
                src/gui/gui_main_window.cpp \
                src/gui/gui_manager.cpp \
                src/gui/gui_plugin_manager.cpp \
                src/gui/gui_progress_dialog.cpp \
                src/gui/livedoc_browser.cpp \
                src/gui/livedoc_home_provider.cpp \
                src/gui/livedoc_widget.cpp \
                src/gui/livedoc.cpp \
                src/gui/about_dialog.cpp \
                src/res/resource.cpp \
                src/helper/startup_helper.cpp \
                src/helper/settings_scope.cpp \
                src/helper/dom_helper.cpp \
                src/helper/singleton.cpp \
                src/item/abstract_item.cpp \
                src/item/abstract_window_item.cpp \
                src/item/abstract_item_input_output_base.cpp \
                src/item/item_connector.cpp \
                src/item/item_input.cpp \
                src/item/item_list_model.cpp \
                src/item/item_manager.cpp \
                src/item/item_note.cpp \
                src/item/item_origin_visualizer_entry.cpp \
                src/item/item_origin_visualizer.cpp \
                src/item/item_output.cpp \
                src/item/item_scene.cpp \
                src/item/item_toolbox_view.cpp \
                src/item/item_toolbox.cpp \
                src/item/item_view.cpp \
                src/item/item_templates_view.cpp \
                src/item/item_templates_widget.cpp \
                src/item/item_templates_model.cpp \
                src/plugin/plugin_manager.cpp \
                src/plugin/plugin_meta_data.cpp \
                src/plugin/plugin_table_model.cpp \
                src/error/console_widget.cpp \
                src/error/console_message.cpp \
                src/error/console_model.cpp \
                src/project/abstract_workspace.cpp \
                src/project/file_workspace.cpp \
                src/project/sql_workspace.cpp \
                src/project/select_workspace_dialog.cpp \
                src/project/file_workspace_gui.cpp \
                src/project/sql_workspace_gui.cpp \
                src/project/project_manager_gui.cpp \
                src/project/project_manager.cpp \
                src/project/project_gui.cpp \
                src/project/abstract_workspace_gui.cpp \
                src/project/abstract_project.cpp \
                src/project/file_project.cpp \
                src/project/file_helper.cpp \
                src/project/file_project_new_dialog.cpp \
                src/project/file_workspace_new_dialog.cpp \
                src/project/project_changed_extern_dialog.cpp \
                src/project/file_project_load_dialog.cpp \
                src/project/file_workspace_load_dialog.cpp \
                src/project/projectlist_dockwidget.cpp \
                src/project/project_save_reminder_dialog.cpp \
                src/project/sql_workspace_new_dialog.cpp \
                src/project/file_workspace_edit_dialog.cpp \
                src/project/file_project_edit_dialog.cpp \
                src/project/wizard_page_workspace_export_type.cpp \
                src/project/wizard_workspace_export.cpp \
                src/project/wizard_page_file_system_export.cpp \
                src/project/wizard_workspace_import.cpp \
                src/project/wizard_page_workspace_import_type.cpp \
                src/project/wizard_page_file_system_project_import.cpp


HEADERS     +=  \
                src/gui/gui_main_window.h \
                src/gui/gui_progress_dialog.h \
                src/gui/gui_plugin_manager.h \
                src/gui/livedoc_browser.h \
                src/gui/livedoc_home_provider.h \
                src/gui/livedoc_widget.h \
                src/gui/about_dialog.h \
                src/helper/startup_helper_p.h \
                src/helper/settings_scope_p.h \
                src/item/abstract_item_p.h \
                src/item/abstract_window_item_p.h \
                src/item/abstract_item_input_output_base_p.h \
                src/item/item_input_p.h \
                src/item/item_output_p.h \
                src/item/item_connector.h \
                src/item/item_list_model.h \
                src/item/item_manager.h \
                src/item/item_note.h \
                src/item/item_origin_visualizer_entry.h \
                src/item/item_scene.h \
                src/item/item_toolbox_view.h \
                src/item/item_toolbox.h \
                src/item/item_view.h \
                src/item/item_origin_visualizer_p.h \
                src/item/item_templates_model.h \
                src/item/item_templates_view.h \
                src/item/item_templates_widget.h \
                src/res/resource.h \
                src/plugin/plugin_meta_data.h \
                src/plugin/plugin_table_model.h \
                src/plugin/plugin_manager_p.h \
                src/error/console_widget.h \
                src/error/console_message.h \
                src/error/console_model.h \
                src/project/abstract_workspace.h \
                src/project/file_workspace.h \
                src/project/sql_workspace.h \
                src/project/select_workspace_dialog.h \
                src/project/file_workspace_gui.h \
                src/project/sql_workspace_gui.h \
                src/project/project_manager_gui.h \
                src/project/project_manager.h \
                src/project/project_gui.h \
                src/project/abstract_workspace_gui.h \
                src/project/abstract_project.h \
                src/project/file_project.h \
                src/project/file_helper.h \
                src/project/file_project_new_dialog.h \
                src/project/file_workspace_new_dialog.h \
                src/project/file_datatype_helper.h \
                src/project/project_changed_extern_dialog.h \
                src/project/file_project_load_dialog.h \
                src/project/file_workspace_load_dialog.h \
                src/project/projectlist_dockwidget.h \
                src/project/project_save_reminder_dialog.h \
                src/project/sql_workspace_new_dialog.h \
                src/project/file_workspace_edit_dialog.h \
                src/project/project_manager_config.h \
                src/project/file_project_edit_dialog.h \
                src/project/wizard_page_workspace_export_type.h \
                src/project/wizard_workspace_export.h \
                src/project/wizard_page_file_system_export.h \
                src/project/wizard_workspace_import.h \
                src/project/wizard_page_workspace_import_type.h \
                src/project/wizard_page_file_system_project_import.h \
                src/project/export_import_helper.h \
                src/project/wizard_page_config.h



HEADERS     +=     \
                include/appcore.h \
                include/gui/gui_manager.h \
                include/gui/interface_livedoc_provider.h \
                include/gui/livedoc.h \
                include/item/abstract_item.h \
                include/item/abstract_window_item.h \
                include/item/abstract_item_input_output_base.h \
                include/item/item_input.h \
                include/item/item_output.h \
                include/item/item_origin_visualizer.h \
                include/plugin/plugin_manager.h \
                include/plugin/interface_factory.h \
                include/helper/singleton.h \
                include/helper/startup_helper.h \
                include/helper/startup_helper_templates.h \
                include/helper/settings_scope.h \
                include/helper/dom_helper.h

FORMS       +=  \
                src/gui/gui_main_window.ui \
                src/gui/gui_progress_dialog.ui \
                src/gui/livedoc_widget.ui \
                src/gui/gui_plugin_manager.ui \
                src/gui/about_dialog.ui \
                src/item/graphics_item_origin_visualizer_entry.ui \
                src/item/graphics_item_origin_visualizer.ui \
                src/item/graphics_item_toolbox.ui \
                src/item/item_templates_widget.ui \
                src/error/console_widget.ui \
                src/project/select_workspace_dialog.ui \
                src/project/file_workspace_new_dialog.ui \
                src/project/file_project_new_dialog.ui \
                src/project/project_save_reminder_dialog.ui \
                src/project/projectlist_dockwidget.ui \
                src/project/sql_workspace_new_dialog.ui \
                src/project/project_changed_extern_dialog.ui \
                src/project/file_workspace_edit_dialog.ui \
                src/project/file_project_edit_dialog.ui \
                src/project/project_info_dialog.ui \
                src/project/wizard_page_workspace_export_type.ui \
                src/project/wizard_workspace_export.ui \
                src/project/wizard_page_file_system_export.ui \
                src/project/wizard_workspace_import.ui \
                src/project/wizard_page_workspace_import_type.ui \
                src/project/wizard_page_file_system_project_import.ui


RESOURCES   +=  src/res/res_core.qrc


DISTFILES   +=  src/res/livedoc_items.html \
                src/res/livedoc_func.css

INCLUDEPATH +=  $$PWD/include \         #public api
                $$PWD/src \             #private stuff
                $$PWD/ui_header

OTHER_FILES += style.astylerc

unix:macx {
	LIBS += -L"/usr/local/lib/"
        QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@rpath/
}

UI_DIR       =  $$PWD/ui_header/
OBJECTS_DIR  =  $$PWD/obj/
MOC_DIR      =  $$PWD/moc/

# Set build destination folder for manager libary
PROJ_DIR = $$PWD/../
# Linux:
unix:CONFIG(debug, debug|release):          DESTDIR = $$PROJ_DIR/build/linux/debug/
else:unix:CONFIG(release, debug|release):   DESTDIR = $$PROJ_DIR/build/linux/release/
# Windows:
win32:CONFIG(debug, debug|release):         DESTDIR = $$PROJ_DIR/build/win/debug/
else:win32:CONFIG(release, debug|release):  DESTDIR = $$PROJ_DIR/build/win/release/

LIBS += -L$$DESTDIR
