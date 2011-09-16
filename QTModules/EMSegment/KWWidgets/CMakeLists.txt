
# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  
  ${CMAKE_CURRENT_SOURCE_DIR}/Graph
  )

#-----------------------------------------------------------------------------

file(GLOB headers 
  "${EMSegment_SOURCE_DIR}/*.h"
  "${EMSegment_SOURCE_DIR}/MRML/*.h"
  "${EMSegment_SOURCE_DIR}/Algorithm/*.h"
  "${EMSegment_SOURCE_DIR}/Algorithm/*.txx"
  "${EMSegment_SOURCE_DIR}/ImageFilters/*.h"
  "${EMSegment_SOURCE_DIR}/Registration/*.h"
  "${EMSegment_SOURCE_DIR}/Tasks/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Graph/*.h"
  )

# --------------------------------------------------------------------------
# Sources



set(EMSegment_SRCS ${EMSegment_SRCS} ${EMSegment_BASE_SRCS}
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentGUI.cxx 
  # wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentParametersSetStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentIntensityImagesStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentAnatomicalStructureStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentSpatialPriorsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentNodeParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentIntensityDistributionsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentRegistrationParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentRunSegmentationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentInputChannelsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentPreProcessingStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentKWDynamicFrame.cxx

  ${CMAKE_CURRENT_SOURCE_DIR}/Graph/vtkImageGraph.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Graph/vtkIndirectLookupTable.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Graph/vtkImageDrawObjects.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Graph/vtkImageCurveRegion.cxx
  )

set_source_files_properties(
  # Algorithm sources (temporary exclusion)
  ${EMSegment_SOURCE_DIR}/Algorithm/EMLocalInterface.cxx
  ${EMSegment_SOURCE_DIR}/Algorithm/EMLocalRegistrationCostFunction.cxx
  ${EMSegment_SOURCE_DIR}/Algorithm/EMLocalShapeCostFunction.cxx
  ${EMSegment_SOURCE_DIR}/Algorithm/vtkDataDef.cxx
  ${EMSegment_SOURCE_DIR}/Algorithm/vtkFileOps.cxx
  ${EMSegment_SOURCE_DIR}/Algorithm/vtkTimeDef.cxx
  ${EMSegment_SOURCE_DIR}/ImageFilters/vtkImageMeanIntensityNormalization.cxx
  ${EMSegment_SOURCE_DIR}/ImageFilters/vtkSimonParameterReaderWriter.cxx
  ${EMSegment_SOURCE_DIR}/Registration/vtkBSplineRegistrator.cxx
  ${EMSegment_SOURCE_DIR}/Registration/vtkRigidRegistrator.cxx
  ${EMSegment_SOURCE_DIR}/Registration/vtkITKTransformAdapter.cxx
  WRAP_EXCLUDE
  )


  # --------------------------------------------------------------------------
  # Wrapping

  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(EMSegment 
    EMSegment_TCL_SRCS 
    "${EMSegment_SRCS}" "")


  #---------------------------------------------------------------------------
  # Add Loadable Module support

  generatelm(EMSegment_SRCS EMSegment.txt)


  # --------------------------------------------------------------------------
  # Build and install the library

  set(lib_name EMSegment)
  add_library(${lib_name} 
    ${EMSegment_SRCS} 
    ${EMSegment_TCL_SRCS}
    )
  slicer3_set_modules_output_path(${lib_name})

if(Slicer3_USE_PYTHON)
 set(EM_PYTHON_LIBRARIES SlicerBaseGUIPython)
endif(Slicer3_USE_PYTHON)

  target_link_libraries(${lib_name}
      ${Slicer3_Libs_LIBRARIES}
      ${Slicer3_Base_LIBRARIES}
      ${KWWidgets_LIBRARIES} 
      ${ITK_LIBRARIES}
      vtkSlicerVolumesModuleLogic
      Volumes
      ${EM_PYTHON_LIBRARIES}
      AtlasCreatorCxxModule
      )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
      SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
      )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)

  slicer3_install_modules(${lib_name})



