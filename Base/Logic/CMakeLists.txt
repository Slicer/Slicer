PROJECT(SlicerBaseLogic)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBase_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

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
  vtkImageLabelOutline.cxx
  vtkSlicerTransformLogic.cxx
  vtkPichonFastMarching.cxx
  vtkPichonFastMarchingPDF.cxx
  vtkImageRectangularSource.cxx
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
# Wrapping
IF (VTK_WRAP_TCL)
  set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
  vtk_wrap_tcl3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
# Build the library

set(lib_name SlicerBaseLogic)

SET(srcs
  ${SlicerBaseLogic_SRCS}
  )
IF (VTK_WRAP_TCL)
  SET(srcs
    ${srcs}
    ${SlicerBaseLogic_TCL_SRCS}
    )
ENDIF (VTK_WRAP_TCL)

add_library(${lib_name} 
  ${srcs}
  )

SET(libs
  MRML
  MRMLCLI
  vtkITK
  ${OPENTRACKER_LIB}
  FreeSurfer
  vtkTeem
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkIO
  vtkHybrid
  )
IF (VTK_WRAP_TCL AND Slicer3_USE_KWWIDGETS)
  SET(libs
    ${libs}
    ${KWWidgets_LIBRARIES}
    vtkCommonTCL
    vtkFilteringTCL
    vtkGraphicsTCL
    vtkIOTCL
    vtkHybridTCL
    )
ENDIF (VTK_WRAP_TCL AND Slicer3_USE_KWWIDGETS)
target_link_libraries(${lib_name} 
  ${libs}
  )

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Should this location change, update Base/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${colorfile}
    COPYONLY
    )
endforeach(colorfile)
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources/ColorFiles
  )
