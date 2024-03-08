project(TP-anim-external)

cmake_minimum_required(VERSION 3.14)

if ( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug )
endif()


macro(add_dependency_if_compiled target dep)
    if(TARGET ${dep})
        add_dependencies(${target} ${dep})
    endif()
endmacro()

# These paths need to be synchronized with FindRadium.cmake
set(BUNDLE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Bundle-${CMAKE_CXX_COMPILER_ID})
set(CMAKE_INSTALL_PREFIX ${BUNDLE_DIRECTORY})
set(CMAKE_PREFIX_PATH ${BUNDLE_DIRECTORY})

include(ExternalProject)
ExternalProject_Add(
        glm
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/glm"
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DGLM_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
    set(glm_DIR "${CMAKE_INSTALL_PREFIX}/glm")       
ExternalProject_Add(
    glbinding
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/glbinding"
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS}
                   -DOPTION_BUILD_TESTS=OFF
                   -DOPTION_BUILD_DOCS=OFF
                   -DOPTION_BUILD_TOOLS=OFF
                   -DOPTION_BUILD_EXAMPLES=OFF
                   -DOPTION_BUILD_OWN_KHR_HEADERS=ON
                   -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               )

    set(glbinding_DIR "${CMAKE_INSTALL_PREFIX}/glbinding")       

    add_custom_target(globjects-dep)
        add_dependency_if_compiled(globjects-dep glbinding)
    add_dependency_if_compiled(globjects-dep glm)

   ExternalProject_Add(
        globjects
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/globjects"
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/"
        DEPENDS globjects-dep
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS}
                   -DOPTION_BUILD_DOCS=OFF
                   -DOPTION_BUILD_EXAMPLES=OFF
                   -DOPTION_USE_EIGEN=ON
                   -Dglbinding_DIR=${glbinding_DIR}
                   -Dglm_DIR=${glm_DIR}
                   -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

    )
    set(globjects_DIR "${CMAKE_INSTALL_PREFIX}/globjects")       

