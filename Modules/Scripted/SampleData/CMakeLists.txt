
#-----------------------------------------------------------------------------
set(MODULE_NAME SampleData)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/BaselineVolume.png
  Resources/Icons/CTACardio.png
  Resources/Icons/CTChest.png
  Resources/Icons/DTIBrain.png
  Resources/Icons/DTIVolume.png
  Resources/Icons/dwi.png
  Resources/Icons/MRBrainTumor1.png
  Resources/Icons/MRBrainTumor2.png
  Resources/Icons/MRHead.png
  Resources/Icons/MRProstate.png
  Resources/Icons/Panoramix-cropped.png
  Resources/Icons/PreDentalSurgery.png
  Resources/Icons/CTBrain.png
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
