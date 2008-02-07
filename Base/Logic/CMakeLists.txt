PROJECT(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Sources

SET(SlicerBaseLogic_SRCS
  # Logic classes (data management and calculation)
  vtkSlicerLogic.cxx 
  vtkSlicerApplicationLogic.cxx 
  vtkSlicerModuleLogic.cxx 
  vtkSlicerTask.cxx
  vtkSlicerSliceLogic.cxx 
  vtkSlicerSliceLayerLogic.cxx
  vtkSlicerSliceGlyphLogic.cxx
  vtkSlicerGlyphLogic.cxx
  vtkSlicerVolumesLogic.cxx 
  vtkSlicerModelsLogic.cxx 
  vtkSlicerModelHierarchyLogic.cxx
  vtkSlicerFiducialsLogic.cxx
  vtkSlicerColorLogic.cxx
  # slicer's vtk extensions (filters)
  vtkImageSlicePaint.cxx 
  vtkImageSlice.cxx 
  vtkImageAccumulateDiscrete.cxx
  vtkImageBimodalAnalysis.cxx
  vtkImageFillROI.cxx
  vtkSlicerGlyphSource2D.cxx
  vtkSlicerROILogic.cxx
  vtkImageResliceMask.cxx 
  vtkImageConnectivity.cxx 
  vtkImageLabelChange.cxx 
  vtkImageErode.cxx 
  vtkImageNeighborhoodFilter.cxx
  vtkSlicerGradientEditorLogic.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTK_INCLUDE_DIR}  
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${SlicerBase_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )



# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
SET(VTK_WRAP_HINTS ${SlicerBaseLogic_SOURCE_DIR}/Wrapping/Tcl/hints)
VTK_WRAP_TCL3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerBaseLogic 
  ${SlicerBaseLogic_SRCS} ${SlicerBaseLogic_TCL_SRCS} 
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS SlicerBaseLogic
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(SlicerBaseLogic 
  MRML
  ${KWWidgets_LIBRARIES}
  vtkITK
  vtkCommonTCL
  vtkFilteringTCL
  vtkGraphicsTCL
  vtkIOTCL
  vtkHybridTCL
  ${OPENTRACKER_LIB}
  FreeSurfer
  vtkTeem
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${SlicerBaseLogic_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${SlicerBaseLogic_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
)

# --------------------------------------------------------------------------
# Resources
SET (COLORFILES
    ./Resources/ColorFiles/PelvisColor.txt
    ./Resources/ColorFiles/64Color-Nonsemantic.txt
    ./Resources/ColorFiles/SPL-BrainAtlas-ColorFile.txt
    ./Resources/ColorFiles/AbdomenColors.txt
)

SET (Color_DIR ${SLICER_LIBRARIES_DIR}/Base/Logic)
IF (NOT EXISTS ${Color_DIR})
   FILE(MAKE_DIRECTORY ${Color_DIR})
ENDIF (NOT EXISTS ${Color_DIR})

FOREACH ( colorfile ${COLORFILES})
   CONFIGURE_FILE(
        ${SlicerBaseLogic_SOURCE_DIR}/${colorfile}
        ${Color_DIR}/${colorfile}
        COPYONLY
)
   INSTALL(
        FILES ${Color_DIR}/${colorfile}
        DESTINATION lib/Slicer3/Base/Logic/Resources/ColorFiles
)
ENDFOREACH(colorfile)
