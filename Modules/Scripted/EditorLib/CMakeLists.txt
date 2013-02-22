
#-----------------------------------------------------------------------------
set(MODULE_NAME EditorLib)

#-----------------------------------------------------------------------------
set(EDITOR_BUILTIN_EFFECTS
  LabelEffect
  PaintEffect
  DrawEffect
  ThresholdEffect
  RectangleEffect
  LevelTracingEffect
  MakeModelEffect
  MorphologyEffect
  ErodeEffect
  DilateEffect
  IslandEffect
  IdentifyIslandsEffect
  ChangeIslandEffect
  RemoveIslandsEffect
  SaveIslandEffect
  ChangeLabelEffect
  GrowCutEffect
  FastMarchingEffect
  )

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  __init__
  ColorBox
  EditBox
  EditColor
  EditOptions
  EditUtil
  Effect
  HelperBox
  ${EDITOR_BUILTIN_EFFECTS}
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_SUBDIR
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
