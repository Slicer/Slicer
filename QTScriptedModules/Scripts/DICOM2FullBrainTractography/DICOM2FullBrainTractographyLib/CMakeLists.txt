set(DICOM2FullBrainTractographyLib_PYTHON_SCRIPTS
  __init__
  full_tractography_workflow
  )

set(DICOM2FullBrainTractographyLib_PYTHON_RESOURCES
  Resources/UI/dicom2nrrd.ui
  Resources/UI/done.ui
  Resources/UI/dti2fibers.ui
  Resources/UI/dwi2dti.ui
  )

ctkMacroCompilePythonScript(
  TARGET_NAME DICOM2FullBrainTractographyLib
  SCRIPTS "${DICOM2FullBrainTractographyLib_PYTHON_SCRIPTS}"
  RESOURCES "${DICOM2FullBrainTractographyLib_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/DICOM2FullBrainTractographyLib
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif()
