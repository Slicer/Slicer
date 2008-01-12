PROJECT(PythonGADScriptedModule)

SET (Package_DIR ${SLICER_LIBRARIES_DIR}/Modules/Packages)
IF (NOT EXISTS ${Package_DIR})
  FILE(MAKE_DIRECTORY ${Package_DIR})
ENDIF (NOT EXISTS ${Package_DIR})

SET(PythonGADScriptedModule_DIR ${Package_DIR}/PythonGADScriptedModule)

FILE(MAKE_DIRECTORY ${PythonGADScriptedModule_DIR})
SET(PYTHONFILES
  __init__.py
  PythonGADScriptedModuleGUI.py
)

FOREACH(pythonfile ${PYTHONFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/PythonGADScriptedModule/${pythonfile}
    ${PythonGADScriptedModule_DIR}/${pythonfile}
    COPYONLY
  )
ENDFOREACH(pythonfile)

