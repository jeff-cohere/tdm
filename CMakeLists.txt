cmake_minimum_required (VERSION 3.15.0)

# Adjust CMake's module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

enable_language(C)
enable_language(CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17) # needed by Jigsaw

project (tdm VERSION 0.1.0)

message(STATUS "Generating project files in build directory: ${PROJECT_BINARY_DIR}")
message(STATUS "C compiler is ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "Configuring tdymesher (tdm) v${CMAKE_PROJECT_VERSION}")
message(STATUS "Installation prefix is ${CMAKE_INSTALL_PREFIX}")

# We jam everything into the executable.
set(BUILD_SHARED_LIBS OFF)

# -------
#  PETSC
# -------

if (NOT DEFINED ENV{PETSC_DIR})
  message(FATAL_ERROR "PETSC_DIR is undefined! Please set this to PETSc's location.")
endif()
if (NOT DEFINED ENV{PETSC_ARCH})
  message(FATAL_ERROR "PETSC_ARCH is undefined! Please set this to PETSc's architecture.")
endif()
find_package(PETSc REQUIRED)

# General C compiler flags.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
if (APPLE AND CMAKE_BUILD_TYPE STREQUAL Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

# Jigsaw
include(ExternalProject)
set(JIGSAW_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
    -DCMAKE_
ExternalProject_Add(jigsaw_proj
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/jigsaw
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jigsaw
                    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jigsaw
                    INSTALL_DIR ${PROJECT_BINARY_DIR}
                    CMAKE_ARGS ${JIGSAW_CMAKE_ARGS}
                    LOG_CONFIGURE TRUE
                    BUILD_COMMAND ${MAKE} -j
                    LOG_BUILD TRUE
                    INSTALL_COMMAND ${MAKE} install
                    LOG_INSTALL TRUE)

add_library(jigsaw SHARED IMPORTED GLOBAL)
set_target_properties(jigsaw PROPERTIES IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/lib/libjigsaw${CMAKE_SHARED_LIBRARY_SUFFIX})

set(TDM_LIBRARIES ${PETSC_LIBRARIES} jigsaw)

# Set build-time and install RPATH values.
list(APPEND CMAKE_BUILD_RPATH "${PROJECT_BINARY_DIR}/lib")
list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

include_directories(${PROJECT_BINARY_DIR})

# -------------
#  Mesher Code
# -------------

# We use CTest for testing.
#enable_testing()

# The goods!
add_subdirectory(src)

