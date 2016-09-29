# Provide path to itemframework root
ITEMFRAMEWORK_ROOT = $$PWD/../

# Determine platform and build target
BUILDDIR_BASE = $$PROJECT_ROOT/build
BUILDDIR_ITEMFRAMEWORK_BASE = $$ITEMFRAMEWORK_ROOT/build
unix: {
    PLATFORM          = linux
    ITEMFRAMEWORK_LIB = itemframework
    CONFIG(release, debug|release): {
        BUILDTARGET = release
    }
    CONFIG(debug, debug|release): {
        BUILDTARGET = debug
    }
}
win32: {
    PLATFORM          = win
    ITEMFRAMEWORK_LIB = itemframework0
    CONFIG(release, debug|release): {
        BUILDTARGET = release
    }
    CONFIG(debug, debug|release): {
        BUILDTARGET = debug
    }
}
BUILDDIR = $$BUILDDIR_BASE/$$PLATFORM/$$BUILDTARGET
BUILDDIR_ITEMFRAMEWORK = $$BUILDDIR_ITEMFRAMEWORK_BASE/$$PLATFORM/$$BUILDTARGET
