project( MRIBiasFieldCorrection )

#find_package(GenerateCLP REQUIRED)

#if(GenerateCLP_FOUND)
#  include(${GenerateCLP_USE_FILE})
#endif(GenerateCLP_FOUND)

#link_directories(${ModuleDescriptionParser_BINARY_DIR})

#find_package(ITK REQUIRED)
#include(${ITK_USE_FILE})

#include_directories(${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR})

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkThresholdingFilterConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkThresholdingFilterConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkThresholdingFilterConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

#--------------------------------------------------------------------------
# Sources

set(MRIBiasFieldCorrection_SRCS
  vtkThresholdingFilterLogic.cxx
  vtkThresholdingFilterGUI.cxx
  vtkMRMLThresholdingFilterNode.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

#include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#vtk_wrap_tcl3(MRIBiasFieldCorrection 
#  MRIBiasFieldCorrection_TCL_SRCS 
#  "${MRIBiasFieldCorrection_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

#generatelm(MRIBiasFieldCorrection_SRCS 
#  MRIBiasFieldCorrection.xml)

# --------------------------------------------------------------------------
# Build and install the library

#set(lib_name MRIBiasFieldCorrection)
#add_library(${lib_name}
#  ${MRIBiasFieldCorrection_SRCS}
#  ${MRIBiasFieldCorrection_TCL_SRCS}
#  )
#slicer3_set_modules_output_path(${lib_name})

#target_link_libraries(${lib_name}
#  ${Slicer3_Libs_LIBRARIES}
#  ${Slicer3_Base_LIBRARIES}
#  ${KWWidgets_LIBRARIES}
#  ${ITK_LIBRARIES}
#  )

#slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  subdirs(Testing)
endif(BUILD_TESTING)

#####################

#set(CLP MRIBiasFieldCorrection)

#set(${CLP}_SOURCE ${CLP}.cxx)
#generateclp(${CLP}_SOURCE ${CLP}.xml
#  ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
#add_executable(${CLP} ${${CLP}_SOURCE})
#slicer3_set_plugins_output_path(${CLP})
#target_link_libraries (${CLP}
#  ITKIO
#  ITKCommon
#  ITKAlgorithms)

#add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})

#set_target_properties(${CLP}Lib PROPERTIES COMPILE_FLAGS
#  "-Dmain=ModuleEntryPoint")

#slicer3_set_plugins_output_path(${CLP}Lib)

#target_link_libraries(${CLP}Lib ITKIO ITKCommon ITKAlgorithms)

#if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in
  # an installation) and install each target in the developer area (for
  # running from a build)

  #set(TARGETS MRIBiasFieldCorrection)

  #slicer3_install_plugins(${TARGETS})

#endif(Slicer3_SOURCE_DIR)
