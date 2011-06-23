project(MIMXAdapter)

#configure_file(
#  ${MIMXAdapter_SOURCE_DIR}/vtkMimxAdapterConfigure.h.in
#  ${MIMXAdapter_BINARY_DIR}/vtkMimxAdapterConfigure.h
#  )

set(MIMXAdapter_SRCS
    vtkUnstructuredGridToitkMesh.cxx
    itkMeshTovtkPolyData.cxx
    vtkPolyDataToitkMesh.cxx
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
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxAdapterConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxAdapterConfigure.h
  )

#
# Wrapping
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MIMXAdapter_BINARY_DIR})
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(mimxAdapter MimxAdapter_TCL_SRCS "${MIMXAdapter_SRCS}" "")

set(LIB_NAME "mimxAdapter")

add_library(${LIB_NAME} ${MIMXAdapter_SRCS} )

target_link_libraries(${LIB_NAME}
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  ${ITK_LIBRARIES}
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

