cmake_minimum_required(VERSION 3.28.3)

# Project name and version
project(proba VERSION 1.0.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directory for public headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Collect all the source files recursively
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Add the library target
add_library(Simulator STATIC ${SOURCES})

# Specify the include directories for consumers of the library
target_include_directories(Simulator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/headers/>
    $<INSTALL_INTERFACE:include>
)

# Optionally, create an alias target for modern CMake
add_library(Simulator::Simulator ALIAS Simulator)

# Installation rules (optional)
install(TARGETS Simulator
    EXPORT SimulatorTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY headers/ DESTINATION include)

install(EXPORT SimulatorTargets
    FILE SimulatorConfig.cmake
    NAMESPACE Simulator::
    DESTINATION lib/cmake/Simulator
)
