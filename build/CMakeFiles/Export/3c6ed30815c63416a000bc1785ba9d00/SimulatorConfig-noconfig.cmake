#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Simulator::Simulator" for configuration ""
set_property(TARGET Simulator::Simulator APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Simulator::Simulator PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libSimulator.a"
  )

list(APPEND _cmake_import_check_targets Simulator::Simulator )
list(APPEND _cmake_import_check_files_for_Simulator::Simulator "${_IMPORT_PREFIX}/lib/libSimulator.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
