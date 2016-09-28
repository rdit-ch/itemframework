include(base.pri)

PLUGIN = $$_PRO_FILE_PWD_

QT              += core gui xml widgets
CONFIG          += plugin c++11
TEMPLATE        =  lib

INCLUDEPATH     += \
                   $$ITEMFRAMEWORK_ROOT/include \
                   $$PLUGIN/ui_header

!mac: QMAKE_LFLAGS    += -Wl,--unresolved-symbols=report-all

UI_DIR          =  $$PLUGIN/ui_header/
OBJECTS_DIR     =  $$PLUGIN/obj/
MOC_DIR         =  $$PLUGIN/moc/

DESTDIR          = $$BUILDDIR/plugins
unix:macx {
    DESTDIR      = $$BUILDDIR/traviz.app/Contents/MacOS/plugins
}

LIBS            += -L$$BUILDDIR -l$$ITEMFRAMEWORK_LIB

# Include and link against usercore
USERCORE         = $$PLUGIN/../../usercore

INCLUDEPATH     += \
                   $$USERCORE/include

USERCORE_LIB     = usercore
LIBS            += -L$$BUILDDIR -l$$USERCORE_LIB

