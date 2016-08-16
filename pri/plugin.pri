include(base.pri)

QT              += core gui xml widgets
CONFIG          += plugin c++11
TEMPLATE        =  lib
    
INCLUDEPATH     += \
                   $$PROJECT_ROOT/itemframework/include \
                   $$PWD/ui_header

!mac: QMAKE_LFLAGS    += -Wl,--unresolved-symbols=report-all

UI_DIR          =  $$_PRO_FILE_PWD_/ui_header/
OBJECTS_DIR     =  $$_PRO_FILE_PWD_/obj/
MOC_DIR         =  $$_PRO_FILE_PWD_/moc/

# Platform dependent stuff
BUILDDIR = $$PROJECT_ROOT/build
unix: {
CONFIG(release, debug|release):   PROJ_BUILDDIR  = $$BUILDDIR/linux/release
CONFIG(debug, debug|release):     PROJ_BUILDDIR  = $$BUILDDIR/linux/debug

CORE_LIB         = itemframework
}
win32: {
CONFIG(release, debug|release):   PROJ_BUILDDIR  = $$BUILDDIR/win/release
CONFIG(debug, debug|release):     PROJ_BUILDDIR  = $$BUILDDIR/win/debug

CORE_LIB         = itemframework0
}

DESTDIR          = $$PROJ_BUILDDIR/plugins
LIBS            += -L$$PROJ_BUILDDIR -l$$CORE_LIB
