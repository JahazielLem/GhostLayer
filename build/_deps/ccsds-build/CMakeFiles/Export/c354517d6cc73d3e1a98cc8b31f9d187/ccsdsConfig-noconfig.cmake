#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ccsds::ccsds" for configuration ""
set_property(TARGET ccsds::ccsds APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ccsds::ccsds PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libccsds.a"
  )

list(APPEND _cmake_import_check_targets ccsds::ccsds )
list(APPEND _cmake_import_check_files_for_ccsds::ccsds "${_IMPORT_PREFIX}/lib/libccsds.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
