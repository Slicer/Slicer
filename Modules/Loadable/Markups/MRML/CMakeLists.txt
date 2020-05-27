project(vtkSlicer${MODULE_NAME}ModuleMRML)

find_package(RapidJSON REQUIRED)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${RapidJSON_INCLUDE_DIR}
  )

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}DisplayNode.cxx
  vtkMRML${MODULE_NAME}FiducialDisplayNode.cxx
  vtkMRML${MODULE_NAME}FiducialNode.cxx
  vtkMRML${MODULE_NAME}LineNode.cxx
  vtkMRML${MODULE_NAME}AngleNode.cxx
  vtkMRML${MODULE_NAME}CurveNode.cxx
  vtkMRML${MODULE_NAME}ClosedCurveNode.cxx
  vtkMRML${MODULE_NAME}PlaneNode.cxx
  vtkMRML${MODULE_NAME}Node.cxx
  vtkMRML${MODULE_NAME}FiducialStorageNode.cxx
  vtkMRML${MODULE_NAME}JsonStorageNode.cxx
  vtkCurveGenerator.cxx
  vtkCurveGenerator.h
  vtkLinearSpline.cxx
  vtkLinearSpline.h
  vtkParametricPolynomialApproximation.cxx
  vtkParametricPolynomialApproximation.h
  vtkSlicerDijkstraGraphGeodesicPath.cxx
  vtkSlicerDijkstraGraphGeodesicPath.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
