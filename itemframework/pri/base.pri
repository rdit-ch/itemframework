# Provide path to itemframework root
ITEMFRAMEWORK_ROOT      = $$PWD/..
USERCORE                = $$PROJECT_ROOT/usercore

# Determine platform and build target
unix: {
    PLATFORM            = linux
    ITEMFRAMEWORK_LIB   = itemframework
    USERCORE_LIB        = usercore
}

win32: {
    PLATFORM            = win
    ITEMFRAMEWORK_LIB   = itemframework0
    USERCORE_LIB        = usercore0
}

CONFIG(release, debug|release): {
    BUILDTARGET     = release
}
CONFIG(debug, debug|release): {
    BUILDTARGET     = debug
}

BUILDDIR_BASE           = $$PROJECT_ROOT/build
ITEMFRAMEWORK_BUILDDIR_BASE = $$ITEMFRAMEWORK_ROOT/build
CURRENT_TARGET          = $$_PRO_FILE_PWD_

BUILDDIR                = $$BUILDDIR_BASE/$$PLATFORM/$$BUILDTARGET
ITEMFRAMEWORK_BUILDDIR  = $$ITEMFRAMEWORK_BUILDDIR_BASE/$$PLATFORM/$$BUILDTARGET

UI_DIR                  = $$CURRENT_TARGET/ui_header/
OBJECTS_DIR             = $$CURRENT_TARGET/obj/
MOC_DIR                 = $$CURRENT_TARGET/moc/

INCLUDEPATH            += $$ITEMFRAMEWORK_ROOT/include \
                          $$USERCORE/include \
                          $$CURRENT_TARGET/ui_header

LIBS                   += -L$$ITEMFRAMEWORK_BUILDDIR -l$$ITEMFRAMEWORK_LIB
