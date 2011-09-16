project(EMSegmentCommandLineExecutable)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3 or Slicer4

if(NOT EM_Slicer4_FOUND)

  # Slicer3

  if(NOT Slicer3_SOURCE_DIR)
    # we are not even in the Slicer3 Source Dir, we have to find Slicer3
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  endif(NOT Slicer3_SOURCE_DIR)
  
  set (EM_Slicer3_FOUND ${Slicer3_SOURCE_DIR})

  set(COMMANDLINE_LINK_LIBRARIES
    EMSegment
    vtkCommon
    AtlasCreatorCxxModule
    )

else(NOT EM_Slicer4_FOUND)

  # Slicer4
  find_package(SlicerExecutionModel REQUIRED GenerateCLP)
  
  set(COMMANDLINE_LINK_LIBRARIES
    vtkCommon
    vtkSlicerAtlasCreatorModuleLogic
    vtkSlicerEMSegmentModuleLogic
    )

endif(NOT EM_Slicer4_FOUND)



# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${EMSegment_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/CommandLineApplication  
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  #${Slicer_Libs_INCLUDE_DIRS}
  #${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerAtlasCreatorModuleLogic_SOURCE_DIR}
  ${vtkSlicerAtlasCreatorModuleLogic_BINARY_DIR}  
  )


# --------------------------------------------------------------------------
#
# EMSegmentCommandline
#

set(CLP_MODULE EMSegmentCommandLine)

if(NOT EM_Slicer4_FOUND)

# Slicer3 version
set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  ${COMMANDLINE_LINK_LIBRARIES}
  )

slicer3_install_plugins(${CLP_MODULE})

else(NOT EM_Slicer4_FOUND)

slicerMacroBuildCLI(
  NAME ${CLP_MODULE}
  TARGET_LIBRARIES ${COMMANDLINE_LINK_LIBRARIES}
  EXECUTABLE_ONLY
  INCLUDE_DIRECTORIES ${Slicer_ModuleLogic_INCLUDE_DIRS}
  )

endif(NOT EM_Slicer4_FOUND)

# --------------------------------------------------------------------------
#
# EMSegmentTclInterpreter
#

set(CLP_MODULE EMSegmentTclInterpreter)

if(NOT EM_Slicer4_FOUND)

# Slicer3 version
set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  ${COMMANDLINE_LINK_LIBRARIES}
  )

slicer3_install_plugins(${CLP_MODULE})

# TODO
#else(NOT EM_Slicer4_FOUND)

#slicerMacroBuildCLI(
#  NAME ${CLP_MODULE}
#  TARGET_LIBRARIES ${COMMANDLINE_LINK_LIBRARIES}
#  EXECUTABLE_ONLY  
#  )

endif(NOT EM_Slicer4_FOUND)

# --------------------------------------------------------------------------
#
# EMSegmentTransformToNewFormat
#

set(CLP_MODULE EMSegmentTransformToNewFormat)

if(NOT EM_Slicer4_FOUND)

# Slicer3 version
set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  ${COMMANDLINE_LINK_LIBRARIES}
  )

slicer3_install_plugins(${CLP_MODULE})

else(NOT EM_Slicer4_FOUND)

slicerMacroBuildCLI(
  NAME ${CLP_MODULE}
  TARGET_LIBRARIES ${COMMANDLINE_LINK_LIBRARIES}
  EXECUTABLE_ONLY  
  )

endif(NOT EM_Slicer4_FOUND)

