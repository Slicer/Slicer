PROJECT (EMSegmentCommandLineExecutable)

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

INCLUDE_DIRECTORIES ( 
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/CommandLineApplication
  ${VTK_INCLUDE_DIR}
  )

SET (CLP_MODULE EMSegmentCommandLine)

SET ( ${CLP_MODULE}_SOURCE
  ${CLP_MODULE}.cxx
)

# generate command line parsing
GENERATECLP( ${CLP_MODULE}_SOURCE ${CLP_MODULE}.xml )

# build a stand-alone executable
ADD_EXECUTABLE( ${CLP_MODULE} ${${CLP_MODULE}_SOURCE} )
TARGET_LINK_LIBRARIES(
  ${CLP_MODULE}
  EMSegment
  vtkCommon
  )

SET( EMSEG_GENERATE_CLGUI 1 )

IF ( EMSEG_GENERATE_CLGUI )
  SET (CLP_MODULE_GUI EMSegmentCommandLine_GUIVersion)

  SET ( ${CLP_MODULE_GUI}_SOURCE
    ${CLP_MODULE_GUI}.cxx
    )

  # generate command line parsing
  GENERATECLP( ${CLP_MODULE_GUI}_SOURCE ${CLP_MODULE_GUI}.xml )

  # output libs should to in Plugins
  IF (Slicer3_SOURCE_DIR)
    SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
    #  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  ENDIF (Slicer3_SOURCE_DIR)
  
  # executable (mainly for batchmake right now)
  ADD_EXECUTABLE( ${CLP_MODULE_GUI} ${${CLP_MODULE_GUI}_SOURCE} )
  SET_TARGET_PROPERTIES ( ${CLP_MODULE_GUI}
    PROPERTIES COMPILE_FLAGS "-DEM_CL_GUI"
    )
  TARGET_LINK_LIBRARIES(
    ${CLP_MODULE_GUI}
    EMSegment
    vtkCommon
    )

  # pluggable library
  ADD_LIBRARY( ${CLP_MODULE_GUI}Lib SHARED ${${CLP_MODULE_GUI}_SOURCE} )
  SET_TARGET_PROPERTIES ( ${CLP_MODULE_GUI}Lib 
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DEM_CL_GUI"
    )
  TARGET_LINK_LIBRARIES( 
    ${CLP_MODULE_GUI}Lib
    EMSegment
    vtkCommon
    )
ENDIF ( EMSEG_GENERATE_CLGUI )
