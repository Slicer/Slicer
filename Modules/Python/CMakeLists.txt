PROJECT ( PythonModules )

SET (PYTHON_MODULES_SRCS
  ExplodeVolumeTransform.py
  GradientAnisotropicDiffusion.py
  GaussianSmooth.py
  ResliceAsVolume.py
  ResampleVolume.py
  SurfaceConnectivity.py
  SurfaceICPRegistration.py
  SurfaceToolbox.py
  PythonScript.py
)

IF ( USE_PYTHON )
  IF (Slicer3_SOURCE_DIR)
    FOREACH (PYTHON_MODULE_SRC ${PYTHON_MODULES_SRCS})
      CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PYTHON_MODULE_SRC}
        ${SLICER_LIBRARIES_DIR}/Plugins/${PYTHON_MODULE_SRC}
        COPYONLY)
    ENDFOREACH (PYTHON_MODULE_SRC)
  ENDIF (Slicer3_SOURCE_DIR)
ENDIF ( USE_PYTHON )

