project(vtkSlicerTerminologiesModuleLogic)

find_package(RapidJSON REQUIRED)

set(classes
  vtkSlicerTerminologiesReader
  vtkSlicerTerminologiesModuleLogic
  vtkSlicerTerminologyCategory
  vtkSlicerTerminologyType
  vtkSlicerTerminologyEntry
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTerminologiesModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TerminologiesModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_TERMINOLOGIES_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::TerminologiesModuleLogic PRIVATE RapidJSON)

#-----------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/SegmentationCategoryTypeModifier-DICOM-Master.json
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/SegmentationCategoryTypeModifier-DICOM-Master.term.json
  COPYONLY)
install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/SegmentationCategoryTypeModifier-DICOM-Master.term.json
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Terminologies COMPONENT Runtime)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.json
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.term.json
  COPYONLY)
install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.term.json
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Terminologies COMPONENT Runtime)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/AnatomicRegionAndModifier-DICOM-Master.json
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/AnatomicRegionAndModifier-DICOM-Master.term.json
  COPYONLY)
install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/Terminologies/AnatomicRegionAndModifier-DICOM-Master.term.json
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Terminologies COMPONENT Runtime)
