project(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Sources

set(SlicerBaseLogic_SRCS
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
  vtkDataIOManagerLogic.cxx
  # slicer's vtk extensions (filters)
  vtkImageSlicePaint.cxx 
  vtkImageSlice.cxx 
  vtkImageFillROI.cxx
  vtkSlicerGlyphSource2D.cxx
  vtkSlicerROILogic.cxx
  vtkImageResliceMask.cxx 
  vtkImageConnectivity.cxx 
  vtkImageLabelChange.cxx 
  vtkImageErode.cxx 
  vtkImageNeighborhoodFilter.cxx
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

configure_file(
  ${SlicerBaseLogic_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${SlicerBaseLogic_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${SlicerBaseLogic_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  DESTINATION include/Slicer3 COMPONENT Development
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
set(VTK_WRAP_HINTS ${SlicerBaseLogic_SOURCE_DIR}/Wrapping/Tcl/hints)
vtk_wrap_tcl3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

add_library(SlicerBaseLogic 
  ${SlicerBaseLogic_SRCS} ${SlicerBaseLogic_TCL_SRCS} 
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} SlicerBaseLogic)

if(BUILD_SHARED_LIBS)
  install(TARGETS SlicerBaseLogic
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(SlicerBaseLogic 
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

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Resources
set(COLORFILES
  ./Resources/ColorFiles/PelvisColor.txt
  ./Resources/ColorFiles/64Color-Nonsemantic.txt
  ./Resources/ColorFiles/SPL-BrainAtlas-ColorFile.txt
  ./Resources/ColorFiles/AbdomenColors.txt
  )

set(Color_DIR ${SLICER_LIBRARIES_DIR}/Base/Logic)
if(NOT EXISTS ${Color_DIR})
  file(MAKE_DIRECTORY ${Color_DIR})
endif(NOT EXISTS ${Color_DIR})

foreach(colorfile ${COLORFILES})
  configure_file(
    ${SlicerBaseLogic_SOURCE_DIR}/${colorfile}
    ${Color_DIR}/${colorfile}
    COPYONLY
    )
  install(
    FILES ${Color_DIR}/${colorfile}
    DESTINATION lib/Slicer3/Base/Logic/Resources/ColorFiles
    )
endforeach(colorfile)
