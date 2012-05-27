project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkFetchMILogic.cxx
  vtkFetchMIParser.cxx
  vtkFetchMIParserHID.cxx
  vtkFetchMIParserXND.cxx
  vtkFetchMIServerCollection.cxx
  vtkFetchMIServer.cxx
  vtkFetchMIWebServicesClient.cxx
  vtkFetchMIWebServicesClientXND.cxx
  vtkFetchMIWriter.cxx
  vtkFetchMIWriterXND.cxx
  vtkMRMLFetchMINode.cxx
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
