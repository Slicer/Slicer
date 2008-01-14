PROJECT(StochasticTractographyFilter)

SET(CMAKE_VERBOSE_MAKEFILE OFF)

# 
# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
#CONFIGURE_FILE(
#  vtkRealignConfigure.h.in
#  Applications/CLI/Realign/vtkRealignConfigure.h
#)

FIND_PACKAGE(GenerateCLP REQUIRED)

FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

IF (Slicer3_SOURCE_DIR)
  SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
ENDIF (Slicer3_SOURCE_DIR)

INCLUDE_DIRECTORIES( ${CLI_SOURCE_DIR} )

#####################

SET (CLP StochasticTractographyFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    ITKIO
    ITKBasicFilters 
    ITKCommon
    vtkIO
    vtkCommon
    vtkGraphics
    ModuleDescriptionParser
    #MRML
)

SET (CLP GenerateConnectivityMap)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    ITKIO
    ITKBasicFilters 
    ITKCommon
    vtkIO
    vtkCommon
    ModuleDescriptionParser
    # MRML
)

SET (CLP ROITractFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    ITKIO
    ITKBasicFilters 
    ITKCommon
    vtkIO
    vtkCommon
    vtkGraphics
    ModuleDescriptionParser
    #MRML
)

SET (CLP CalculateVolumeStatistics)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/MIT_NAMIC_PNL_Logo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    ITKIO
    ITKBasicFilters 
    ITKCommon
    vtkIO
    vtkCommon
    vtkGraphics
    ModuleDescriptionParser
    #MRML
)

IF (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  SET(TARGETS        
        StochasticTractographyFilter
        GenerateConnectivityMap
        ROITractFilter
        CalculateVolumeStatistics
  )

  FOREACH(target ${TARGETS})
    INSTALL(TARGETS ${target}
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
