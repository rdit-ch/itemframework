include(base.pri)

USERCORE     =  $$_PRO_FILE_PWD_

# Set project properties
QT          +=  gui xml sql widgets
CONFIG      +=  c++11
TEMPLATE    =   lib
VERSION     +=  0.1
DEFINES     +=  USERCORE_BUILD=1 # used for USERCORE_EXPORT macro switch in usercore.h

QMAKE_CXXFLAGS +=  -fvisibility=hidden  # to enforce usage of USERCORE_EXPORT

INCLUDEPATH +=  \
                $$ITEMFRAMEWORK_ROOT/include \
                $$ITEMFRAMEWORK_ROOT/src \

INCLUDEPATH +=  \
                $$USERCORE/include \
                $$USERCORE/src \
                $$USERCORE/ui_header

UI_DIR       =  $$USERCORE/ui_header/
OBJECTS_DIR  =  $$USERCORE/obj/
MOC_DIR      =  $$USERCORE/moc/

LIBS += -L$$BUILDDIR

macx {
    QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@rpath/
}

DESTDIR = $$BUILDDIR
