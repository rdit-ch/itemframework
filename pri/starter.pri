include(base.pri)

# Set project properties
QT          +=  core xml widgets
CONFIG      +=  c++11
TARGET       =  example
TEMPLATE     =  app

STARTER      = $$_PRO_FILE_PWD_$$
USERCORE     = $$STARTER/../usercore
INCLUDEPATH += $$ITEMFRAMEWORK_ROOT/include \
               $$USERCORE/include

# Link against itemframework and usercore
LIBS += -L$$BUILDDIR -l$$ITEMFRAMEWORK_LIB
LIBS += -L$$BUILDDIR -lusercore

DESTDIR = $$BUILDDIR

OBJECTS_DIR = $$STARTER/obj/
MOC_DIR = $$STARTER/moc/

# Set Libary path to executable programm file ($ORIGIN)
unix:!mac{
  QMAKE_LFLAGS  += -Wl,--rpath=\\\$\$ORIGIN
# Link against libsegfault
  QMAKE_LFLAGS  += -lSegFault
}
