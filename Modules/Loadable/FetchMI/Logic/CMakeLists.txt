project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtk${MODULE_NAME}Logic.cxx
  vtk${MODULE_NAME}Parser.cxx
  vtk${MODULE_NAME}ParserHID.cxx
  vtk${MODULE_NAME}ParserXND.cxx
  vtk${MODULE_NAME}ServerCollection.cxx
  vtk${MODULE_NAME}Server.cxx
  vtk${MODULE_NAME}WebServicesClient.cxx
  vtk${MODULE_NAME}WebServicesClientXND.cxx
  vtk${MODULE_NAME}Writer.cxx
  vtk${MODULE_NAME}WriterXND.cxx
  vtkMRML${MODULE_NAME}Node.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  RemoteIO
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
