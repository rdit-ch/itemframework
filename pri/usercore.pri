include(base.pri)

# Set project properties
QT          +=  gui xml sql widgets
CONFIG      +=  c++11
TEMPLATE    =   lib
VERSION     +=  0.1
DEFINES     +=  USERCORE_BUILD=1 # used for USERCORE_EXPORT macro switch in usercore.h

QMAKE_CXXFLAGS +=  -fvisibility=hidden  # to enforce usage of USERCORE_EXPORT

USERCORE     =  $$_PRO_FILE_PWD_
INCLUDEPATH +=  $$USERCORE/src

macx {
    QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@rpath/
}

DESTDIR = $$BUILDDIR
