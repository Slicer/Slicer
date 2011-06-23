project(MIMXMeshIO)

configure_file(
  ${MIMXMeshIO_SOURCE_DIR}/vtkMeshIOConfigure.h.in
  ${MIMXMeshIO_BINARY_DIR}/vtkMeshIOConfigure.h
  )


# Include dirs
#
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_INCLUDE_DIRS}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxIOConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxIOConfigure.h
  )

set(MeshIO_SRCS
    vtkMimxAbaqusFileWriter.cxx
    vtkMimxIntervertebralDiscAbaqusWriter.cxx
    vtkPluginFilterWatcher.cxx
    ModuleProcessInformation.cxx
)

set_source_files_properties(
  vtkMimxAbaqusFileWriter.cxx
  WRAP_EXCLUDE
)

#
# Wrapping
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MIMXMeshIO MeshIO_TCL_SRCS "${MeshIO_SRCS}" "")
set(LIB_NAME "mimxMeshIO")

#add_library(mimxMeshIO ${MeshIO_TCL_SRCS} ${MeshIO_SRCS} )
add_library(${LIB_NAME} ${MeshIO_SRCS} )

target_link_libraries(${LIB_NAME}
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  )

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${LIB_NAME} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
endif(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )




