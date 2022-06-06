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
message(STATUS "C++ compiler is ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION})")
message(STATUS "Configuring tdymesher (tdm) v${CMAKE_PROJECT_VERSION}")
message(STATUS "Installation prefix is ${CMAKE_INSTALL_PREFIX}")

# We jam everything into the executable.
set(BUILD_SHARED_LIBS OFF)

# -------
#  PETSC
# -------

if (DEFINED ENV{PETSC_DIR})
  set(PETSC_DIR $ENV{PETSC_DIR})
else()
  message(FATAL_ERROR "PETSC_DIR env var is undefined! Please set it to PETSc's location.")
endif()
if (DEFINED ENV{PETSC_ARCH})
  set(PETSC_ARCH $ENV{PETSC_ARCH})
else()
  message(FATAL_ERROR "PETSC_ARCH env var is undefined! Please set it to PETSc's architecture.")
endif()
find_package(PETSc REQUIRED)

# General compiler flags.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIE")

# Jigsaw
set(JIGSAW_DIR ${PROJECT_SOURCE_DIR}/ext/jigsaw)
add_library(jigsaw STATIC ${JIGSAW_DIR}/src/jigsaw.cpp)
target_include_directories(jigsaw PRIVATE ${JIGSAW_DIR}/src ${JIGSAW_DIR}/inc)
target_compile_definitions(jigsaw PRIVATE __lib_jigsaw)

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

