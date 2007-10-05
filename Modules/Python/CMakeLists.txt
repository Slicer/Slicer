PROJECT ( PythonModules )

IF ( USE_PYTHON )
  IF (Slicer3_SOURCE_DIR)
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/GradientAnisotropicDiffusion.py
      ${SLICER_LIBRARIES_DIR}/Plugins/GradientAnisotropicDiffusion.py
      COPYONLY)
  ENDIF (Slicer3_SOURCE_DIR)
ENDIF ( USE_PYTHON )
