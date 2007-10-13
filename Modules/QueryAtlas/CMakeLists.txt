PROJECT(QueryAtlas)


#
# install the tcl package for the Module
# - make the module packages build directory if needed
# - make this module's package directory
# - copy over the tcl package files to the build directory
#

SET (Package_DIR ${SLICER_LIBRARIES_DIR}/Modules/Packages)
IF (NOT EXISTS ${Package_DIR})
  FILE(MAKE_DIRECTORY ${Package_DIR})
ENDIF (NOT EXISTS ${Package_DIR})

SET(QueryAtlas_DIR ${Package_DIR}/QueryAtlas)
#
# copy over the tcl files
#
FILE(MAKE_DIRECTORY ${QueryAtlas_DIR})
SET(TCLFILES
  ./Tcl/pkgIndex.tcl
  ./Tcl/Card.tcl ./Tcl/CardFan.tcl
  ./Tcl/QueryAtlas.tcl
  ./Tcl/QueryAtlasAnno.tcl
  ./Tcl/QueryAtlasCSV.tcl
  ./Tcl/QueryAtlasControlledVocabulary.tcl
  ./Tcl/QueryAtlasWeb.tcl
)

FOREACH(tclfile ${TCLFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/QueryAtlas/${tclfile}
    ${QueryAtlas_DIR}/${tclfile}
    COPYONLY
  )
ENDFOREACH(tclfile)


SET (CSVFILES
  ./Resources/NN2002-2-synonyms.csv
  ./Resources/controlledVocabulary.csv
  ./Resources/Neurons_CellpropDB.csv
  ./Resources/NN2002-3-BrainInfoURI.csv
)

FOREACH( csvfile ${CSVFILES})
   CONFIGURE_FILE(
      ${Slicer3_SOURCE_DIR}/Modules/QueryAtlas/${csvfile}
      ${QueryAtlas_DIR}/${csvfile}
      COPYONLY
)
ENDFOREACH(csvfile)


SET (ONTOVIZFILES
    ./OntologyViz/build.xml
    ./OntologyViz/bin/birnlexvis-support.jar
    ./OntologyViz/bin/birnlexvis.jar
    ./OntologyViz/bin/birnlexvis.tcl
    ./OntologyViz/bin/birnlexvisUnix.sh
    ./OntologyViz/bin/birnlexvisWin.sh
    ./OntologyViz/data/neuronames-v1.json
    ./OntologyViz/data/birnlex-body-v1.json
    ./OntologyViz/extjars/json.jar
    ./OntologyViz/extjars/jython.jar
    ./OntologyViz/extjars/prefuse.jar    
)


FOREACH( ovfile ${ONTOVIZFILES})
   CONFIGURE_FILE(
      ${Slicer3_SOURCE_DIR}/Modules/QueryAtlas/${ovfile}
      ${QueryAtlas_DIR}/${ovfile}
      COPYONLY
)
ENDFOREACH(ovfile)



# --------------------------------------------------------------------------
# Sources

SET(QueryAtlas_SRCS
  vtkQueryAtlasIcons.cxx        
  vtkQueryAtlasLogic.cxx
  vtkQueryAtlasGUI.cxx
  vtkMRMLQueryAtlasNode.cxx
  vtkQueryAtlasCollaboratorIcons.cxx
  vtkQueryAtlasUseSearchTermWidget.cxx  
  vtkQueryAtlasSearchTermWidget.cxx  
  # cards and related classes
  vtkBIRNCard.cxx
  vtkBIRNCardManager.cxx
  vtkCard.cxx
  vtkCardManager.cxx
  vtkEdgeGradient.cxx
  vtkFontParameters.cxx
  vtkFreetypeRasterizer.cxx
  vtkRasterizerCharacter.cxx
  vtkRectangle.cxx
  #vtkSortCommand.cxx
  vtkSorter.cxx
  vtkTextureFont.cxx
  vtkTextureFontManager.cxx
  vtkTextureText.cxx
  vtkTextureTextPolys.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${QueryAtlas_SOURCE_DIR}
  ${QueryAtlas_BINARY_DIR}
  ${QdecModule_SOURCE_DIR}
  ${QdecModule_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${Qdec_SOURCE_DIR}
  ${Qdec_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(QueryAtlas QueryAtlas_TCL_SRCS "${QueryAtlas_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

LINK_DIRECTORIES(${Qdec_BINARY_DIR})

ADD_LIBRARY(QueryAtlas
  ${QueryAtlas_SRCS}
  ${QueryAtlas_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS QueryAtlas
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(QueryAtlas
  QdecModule        
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES}
  Qdec
  )



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${QueryAtlas_SOURCE_DIR}/vtkQueryAtlasConfigure.h.in 
  ${QueryAtlas_BINARY_DIR}/vtkQueryAtlasConfigure.h
)



