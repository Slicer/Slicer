project (DicomToNrrdConverter)
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

set (CLP DicomToNrrdConverter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)


if (Slicer3_SOURCE_DIR)
  add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
  slicer3_set_plugins_output_path(${CLP}Lib)
  set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

  set(TARGETS ${CLP}Lib ${CLP})
  slicer3_install_plugins(${TARGETS})
endif (Slicer3_SOURCE_DIR)

