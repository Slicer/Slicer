add_subdirectory(Scripts)
add_subdirectory(EditorLib)
add_subdirectory(DICOMLib)

if(BUILD_TESTING AND PYTHON_EXECUTABLE)
  add_subdirectory(Testing)
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  if(Slicer_BUILD_ChangeTrackerPy)
    # NOTE: ChangeTrackerPy source code is checkout using "External_ChangeTrackerPy.cmake".
    add_subdirectory(
      ${ChangeTrackerPy_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/ChangeTrackerPy
      )
  endif()
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  # NOTE: MultiVolumeImporter source code is checkout using "External_MultiVolumeImporter.cmake".
  add_subdirectory(
    ${MultiVolumeImporter_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeImporter
    )
endif()

if(Slicer_BUILD_SlicerWebGLExport)
  # NOTE: SlicerWebGLExport source code is checkout using "External_SlicerWebGLExport.cmake".
  add_subdirectory(
    ${SlicerWebGLExport_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/SlicerWebGLExport
    )
endif()
