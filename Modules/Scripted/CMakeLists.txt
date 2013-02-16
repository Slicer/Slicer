
include(SlicerMacroBuildScriptedModule)

if(Slicer_BUILD_DICOM_SUPPORT)
  add_subdirectory(DICOMLib)
endif()
add_subdirectory(EditorLib)
add_subdirectory(Scripts)

if(BUILD_TESTING AND PYTHON_EXECUTABLE)
  add_subdirectory(Testing)
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  # NOTE: MultiVolumeImporter source code is checkout using "External_MultiVolumeImporter.cmake".
  add_subdirectory(
    ${MultiVolumeImporter_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeImporter
    )
endif()
