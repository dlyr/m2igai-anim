# This file define common compile flags for all Radium projects.


# Set build configurations ====================================================
# Debug by default: Do it at the beginning to ensure proper configuration
if ( NOT MSVC )
    set( VALID_CMAKE_BUILD_TYPES "Debug Release RelWithDebInfo" )
    if ( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Debug )
    elseif ( NOT "${VALID_CMAKE_BUILD_TYPES}" MATCHES ${CMAKE_BUILD_TYPE} )
        set( CMAKE_BUILD_TYPE Debug )
    endif()
endif()



set(UNIX_DEFAULT_CXX_FLAGS                "-Wall -Wextra  -pthread -msse3 -Wno-sign-compare -Wno-unused-parameter -fno-exceptions -fPIC")
set(UNIX_DEFAULT_CXX_FLAGS_DEBUG          "-D_DEBUG -DCORE_DEBUG -g3 -ggdb")
set(UNIX_DEFAULT_CXX_FLAGS_RELEASE        "-DNDEBUG -O3")
set(UNIX_DEFAULT_CXX_FLAGS_RELWITHDEBINFO "-g3")

set(CMAKE_CXX_STANDARD 14)

# Compilation flag for each platforms =========================================

if (APPLE)
  #    message(STATUS "${PROJECT_NAME} : Compiling on Apple with compiler " ${CMAKE_CXX_COMPILER_ID})

    set(MATH_FLAG "-mfpmath=sse")

    set(CMAKE_CXX_FLAGS                "${UNIX_DEFAULT_CXX_FLAGS}                ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG          "${UNIX_DEFAULT_CXX_FLAGS_DEBUG}          ${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE        "${UNIX_DEFAULT_CXX_FLAGS_RELEASE}        ${MATH_FLAG}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${UNIX_DEFAULT_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS_RELEASE}")

    #add_definitions( -Wno-deprecated-declarations ) # Do not warn for eigen bind being deprecated
elseif (UNIX OR MINGW)
    set(MATH_FLAG "-mfpmath=sse")

    if( MINGW )
        set( EIGEN_ALIGNMENT_FLAG "-mincoming-stack-boundary=2" )
        add_definitions( -static-libgcc -static-libstdc++) # Compile with static libs
    else()
        set( EIGEN_ALIGNMENT_FLAG "" )
    endif()

    set(CMAKE_CXX_FLAGS                "${UNIX_DEFAULT_CXX_FLAGS}                ${EIGEN_ALIGNMENT_FLAG} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG          "${UNIX_DEFAULT_CXX_FLAGS_DEBUG}          ${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE        "${UNIX_DEFAULT_CXX_FLAGS_RELEASE}        ${MATH_FLAG}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${UNIX_DEFAULT_CXX_FLAGS_RELWITHDEBINFO} -ggdb ${CMAKE_CXX_FLAGS_RELEASE}")

    # Prevent Eigen from spitting thousands of warnings with gcc 6+
    add_definitions(-Wno-deprecated-declarations)
    if( NOT(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 5.4))
        add_definitions(-Wno-ignored-attributes -Wno-misleading-indentation)
    endif()
elseif (MSVC)
    # Visual studio flags breakdown
    # /GR- : no rtti ; /Ehs-c- : no exceptions
    # /Od  : disable optimization
    # /Ox :  maximum optimization
    # /GL : enable link time optimization
    # /Zi  : generate debug info

    # remove exceptions from default args
    add_definitions(-D_HAS_EXCEPTIONS=0)
    # disable secure CRT warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    string (REGEX REPLACE "/EHsc *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string (REGEX REPLACE "/GR" ""     CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # remove library compilation flags (MT, MD, MTd, MDd
    string( REGEX REPLACE "/M(T|D)(d)*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string( REGEX REPLACE "/M(T|D)(d)*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string( REGEX REPLACE "/M(T|D)(d)*" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

    set(CMAKE_CXX_FLAGS                "/arch:AVX2 /EHs-c- /MP ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /DCORE_DEBUG /Od /Zi ${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE        "/DNDEBUG /Ox /fp:fast ${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi ${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_SHARED_LINKER_FLAGS      "/LTCG ${CMAKE_SHARED_LINKER_FLAGS}")

    # Problem with Qt linking
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_COMPILING_QSTRING_COMPAT_CPP")

endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "${PROJECT_NAME} : 64 bits build")
else()
    message(STATUS "${PROJECT_NAME} : 32 bits build")
endif()


