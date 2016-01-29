# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------

# Should this location change, update Modules/Loadable/Colors/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${colorfile}
    COPYONLY
    )
endforeach()
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/ColorFiles
  COMPONENT Runtime
  )

# Related terminology files
set(Slicer_TERMINOLOGY_DIR "ColorFiles/Terminology")

set(terminology_filenames

  # Terminology file associated with specific LUTs
  GenericAnatomyColors-SNOMED.csv

  # Region and Category definitions
  AnatomicRegionAndModifier.xml
  SegmentationCategoryTypeModifier.xml
  )

foreach(filename ${terminology_filenames})
  message(STATUS "Configuring terminology file: ${filename}")
  set(terminology_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${Slicer_TERMINOLOGY_DIR}/${filename})
  configure_file(
    ${terminology_filepath}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${Slicer_TERMINOLOGY_DIR}/${filename}
    COPYONLY
    )
  install(
    FILES ${terminology_filepath}
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${Slicer_TERMINOLOGY_DIR}
    COMPONENT Runtime
    )
endforeach()

set(orientation_markers_dir "OrientationMarkers")
set(orientation_marker_filenames
  Human.vtp
  )
foreach(filename ${orientation_marker_filenames})
  set(orientation_marker_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${orientation_markers_dir}/${filename})
  configure_file(
    ${orientation_marker_filepath}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${orientation_markers_dir}/${filename}
    COPYONLY
    )
  install(
    FILES ${orientation_marker_filepath}
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${orientation_markers_dir}
    COMPONENT Runtime
    )
endforeach()
