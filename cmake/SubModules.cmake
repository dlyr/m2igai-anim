
# EXTERNALS
# have ExternalProject available
include(ExternalProject)

set(SUBMODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SUBMODULES_INSTALL_DIRECTORY ${BUNDLE_DIRECTORY}/external)
set(SUBMODULES_BUILD_TYPE Release)

if (MSVC OR MSVC_IDE)
        set(SUBMODULES_INSTALL_DIRECTORY ${BUNDLE_DIRECTORY}/${CMAKE_BUILD_TYPE}/external)
        set(SUBMODULES_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else()
        set(SUBMODULES_INSTALL_DIRECTORY ${BUNDLE_DIRECTORY}/external)
        set(SUBMODULES_BUILD_TYPE Release)
endif()

#OpenGL Stuff
include(submoduleGLM)
include(submoduleGlBinding)
include(submoduleGlObjects)
