include(base.pri)

QT              += core gui xml widgets
CONFIG          += plugin c++11
TEMPLATE        =  lib

LIBS            += -L$$BUILDDIR -l$$USERCORE_LIB

!mac: QMAKE_LFLAGS    += -Wl,--unresolved-symbols=report-all

DESTDIR         = $$BUILDDIR/plugins
unix:macx {
    DESTDIR     = $$BUILDDIR/traviz.app/Contents/MacOS/plugins
}
