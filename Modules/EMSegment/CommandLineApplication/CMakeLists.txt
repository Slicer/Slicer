PROJECT (EMSegmentCommandLineExecutable)

INCLUDE(${Slicer3_SOURCE_DIR}/Libs/GenerateCLP/UseGenerateCLP.cmake)

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

SET( EMSEG_GENERATE_CLGUI 0 )

IF ( EMSEG_GENERATE_CLGUI )
  # output libs should to in Plugins
  IF (Slicer3_SOURCE_DIR)
    SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
    #  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  ENDIF (Slicer3_SOURCE_DIR)
  
  # pluggable library
  ADD_LIBRARY( ${CLP_MODULE}Lib SHARED ${${CLP_MODULE}_SOURCE} )
  SET_TARGET_PROPERTIES ( ${CLP_MODULE}Lib 
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DEM_CL_GUI"
    )
  TARGET_LINK_LIBRARIES( 
    ${CLP_MODULE}Lib
    EMSegment
    vtkCommon
    )
ENDIF ( EMSEG_GENERATE_CLGUI )
