project(StochasticTractographyFilter)

set(CMAKE_VERBOSE_MAKEFILE OFF)

# 
# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
#CONFIGURE_FILE(
#  vtkRealignConfigure.h.in
#  Applications/CLI/Realign/vtkRealignConfigure.h
#)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

slicer3_set_default_plugins_output_path()

include_directories( ${CLI_SOURCE_DIR} )

#####################

set (CLP StochasticTractographyFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP}
  ITKIO
  ITKBasicFilters 
  ITKCommon
  vtkIO
  vtkCommon
  vtkGraphics
  ModuleDescriptionParser
  #MRML
  )

set (CLP GenerateConnectivityMap)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP}
  ITKIO
  ITKBasicFilters 
  ITKCommon
  vtkIO
  vtkCommon
  ModuleDescriptionParser
  # MRML
  )

set (CLP ROITractFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP}
  ITKIO
  ITKBasicFilters 
  ITKCommon
  vtkIO
  vtkCommon
  vtkGraphics
  ModuleDescriptionParser
  #MRML
  )

set (CLP CalculateVolumeStatistics)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP}
  ITKIO
  ITKBasicFilters 
  ITKCommon
  vtkIO
  vtkCommon
  vtkGraphics
  ModuleDescriptionParser
  #MRML
  )

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    StochasticTractographyFilter
    GenerateConnectivityMap
    ROITractFilter
    CalculateVolumeStatistics
    )

  foreach(target ${TARGETS})
    install(TARGETS ${target}
      RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
      )
  endforeach(target)

endif (Slicer3_SOURCE_DIR)
