
#-----------------------------------------------------------------------------
set(MODULE_NAME EditorLib)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

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
  WatershedFromMarkerEffect
  FastMarchingEffect
  WandEffect
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
  LabelCreateDialog
  LabelStructureListWidget
  ${EDITOR_BUILTIN_EFFECTS}
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/ChangeIsland.png
  Resources/Icons/ChangeLabel.png
  Resources/Icons/DefaultTool.png
  Resources/Icons/DilateLabel.png
  Resources/Icons/Draw.png
  Resources/Icons/EraseLabel.png
  Resources/Icons/ErodeLabel.png
  Resources/Icons/FastMarchingEffect.png
  Resources/Icons/GrowCutSegment.png
  Resources/Icons/WatershedFromMarkerEffect.png
  Resources/Icons/IdentifyIslands.png
  Resources/Icons/ImplicitRectangle.png
  Resources/Icons/LevelTracing.png
  Resources/Icons/MakeModel.png
  Resources/Icons/NextCheckPoint.png
  Resources/Icons/Paint.png
  Resources/Icons/PreviousCheckPoint.png
  Resources/Icons/RemoveIslands.png
  Resources/Icons/SaveIsland.png
  Resources/Icons/Threshold.png
  Resources/Icons/ToolbarEditorToolbox.png
  Resources/Icons/WandEffect.png
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
