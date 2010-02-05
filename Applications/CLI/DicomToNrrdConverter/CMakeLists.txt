project (DicomToNrrdConverter)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


set (CLP DicomToNrrdConverter)

set ( ${CLP}_SOURCE ${CLP}.cxx)

if (Slicer3_SOURCE_DIR)
  generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
else (Slicer3_SOURCE_DIR)
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

INSTALL(TARGETS
  DicomToNrrdConverter
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

endif (Slicer3_SOURCE_DIR)
