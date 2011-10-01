SET(CTWizard_PYTHON_SCRIPTS
  __init__.py
  Helper.py
  ChangeTrackerStep.py
  ChangeTrackerSelectScansStep.py
  ChangeTrackerDefineROIStep.py
  ChangeTrackerSegmentROIStep.py
  ChangeTrackerRegistrationStep.py
  ChangeTrackerAnalyzeROIStep.py
  ChangeTrackerReportROIStep.py
 )

SET(CTWizard_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME ChangeTrackerWizard
  SCRIPTS "${CTWizard_PYTHON_SCRIPTS}"
  RESOURCES "${CTWizard_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/ChangeTrackerWizard
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif(BUILD_TESTING)
