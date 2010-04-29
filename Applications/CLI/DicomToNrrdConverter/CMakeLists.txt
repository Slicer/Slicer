project (DicomToNrrdConverter)
cmake_minimum_required(VERSION 2.6)

enable_testing()

if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

if (Slicer3_SOURCE_DIR)
  ##The current testing suite only works from within Slicer3.  Other packages
  ##that use this tool will not properly run the test
  ADD_SUBDIRECTORY(Testing)
endif (Slicer3_SOURCE_DIR)

set (CLP DicomToNrrdConverter)
set ( ${CLP}_SOURCE ${CLP}.cxx)

if (Slicer3_SOURCE_DIR)
  generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
else (Slicer3_SOURCE_DIR)
  find_package(GenerateCLP NO_MODULE REQUIRED)
  include(${GenerateCLP_USE_FILE})
  generateclp(${CLP}_SOURCE ${CLP}.xml)
endif (Slicer3_SOURCE_DIR)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

if (Slicer3_SOURCE_DIR)
  slicer3_set_plugins_output_path(${CLP})

  add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
  slicer3_set_plugins_output_path(${CLP}Lib)
  set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

  set(TARGETS ${CLP}Lib ${CLP})
  slicer3_install_plugins(${TARGETS})
else (Slicer3_SOURCE_DIR)

install(TARGETS
  DicomToNrrdConverter
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

endif (Slicer3_SOURCE_DIR)
