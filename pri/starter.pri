include(base.pri)

# Set project properties
QT          +=  core xml widgets
CONFIG      +=  c++11
TEMPLATE     =  app

# Link against usercore
LIBS        += -L$$BUILDDIR -l$$USERCORE_LIB

DESTDIR      = $$BUILDDIR

# Set Libary path to executable programm file ($ORIGIN)
unix:!mac {
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN -Wl,--rpath=$$ITEMFRAMEWORK_BUILDDIR
  QMAKE_LFLAGS += -lSegFault
}

macx {
  QMAKE_RPATHDIR += $$BUILDDIR # or (relativ): @executable_path/../../../  (should be added by macdeployqt in theory)
  QMAKE_RPATHDIR += $$ITEMFRAMEWORK_BUILDDIR
  QMAKE_RPATHDIR += /usr/local/lib/
}
