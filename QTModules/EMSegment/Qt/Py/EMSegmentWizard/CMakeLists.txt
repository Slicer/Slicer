SET(EMSegmentWizard_PYTHON_SCRIPTS
  __init__.py
  Helper.py
  EMSegmentDynamicFrame.py
  EMSegmentStep.py
  EMSegmentSelectTaskStep.py                        # step 1
  EMSegmentDefineInputChannelsStep.py               # step 2
  EMSegmentDefineAnatomicalTreeStep.py              # step 3
  EMSegmentDefineAtlasStep.py                       # step 4
  EMSegmentEditRegistrationParametersStep.py        # step 5
  EMSegmentDefinePreprocessingStep.py               # step 6
  EMSegmentSpecifyIntensityDistributionStep.py      # step 7
  EMSegmentEditNodeBasedParametersStep.py           # step 8
  EMSegmentDefineMiscParametersStep.py              # step 9
  EMSegmentStartSegmentationStep.py
  EMSegmentLabelStatisticsStep.py                   # step after segmentation, step 11  
  EMSegmentDummyStep.py  
  
  # easy/quick mode
  EMSegmentQuickStep1.py
  EMSegmentQuickStep2.py
  EMSegmentQuickStep3.py
  EMSegmentQuickStep4.py
  EMSegmentQuickStep5.py
  EMSegmentQuickSegmentationStep.py
  )

SET(EMSegmentWizard_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME EMSegmentWizard
  SCRIPTS "${EMSegmentWizard_PYTHON_SCRIPTS}"
  RESOURCES "${EMSegmentWizard_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/EMSegmentWizard
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif(BUILD_TESTING)
