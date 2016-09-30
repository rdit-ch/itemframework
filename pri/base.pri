# Provide path to itemframework root
ITEMFRAMEWORK_ROOT = $$PWD/../

# Determine platform and build target
BUILDDIR_BASE = $$ITEMFRAMEWORK_ROOT/build
unix: {
    PLATFORM          = linux
    ITEMFRAMEWORK_LIB = itemframework
    USERCORE_LIB      = usercore
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
    USERCORE_LIB      = usercore0
    CONFIG(release, debug|release): {
        BUILDTARGET = release
    }
    CONFIG(debug, debug|release): {
        BUILDTARGET = debug
    }
}
BUILDDIR = $$BUILDDIR_BASE/$$PLATFORM/$$BUILDTARGET
