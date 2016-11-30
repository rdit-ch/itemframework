include(../../itemframework/pri/base.pri)

TEMPLATE = app

# private headers to test non-public-API classes
INCLUDEPATH     += $$ITEMFRAMEWORK_ROOT/src

!mac: QMAKE_LFLAGS    += -Wl,--unresolved-symbols=report-all

DESTDIR = $$PWD/../build/$$PLATFORM/$$BUILDTARGET

QT += testlib gui xml widgets

CONFIG += qt console warn_on depend_includepath testcase c++11

# Set Libary path to executable programm file ($ORIGIN)
unix:!mac {
  QMAKE_LFLAGS += -Wl,--rpath=$$BUILDDIR -Wl,--rpath=$$ITEMFRAMEWORK_BUILDDIR
}

macx {
  QMAKE_RPATHDIR += $$BUILDDIR # or (relative): @executable_path/../../../  (should be added by macdeployqt in theory)
  QMAKE_RPATHDIR += $$ITEMFRAMEWORK_BUILDDIR
  QMAKE_RPATHDIR += /usr/local/lib/
}
