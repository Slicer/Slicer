#-----------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/SegmentEditorEffects.__init__.py.in
  ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/SegmentEditorEffects/__init__.py
  )

#-----------------------------------------------------------------------------
set(SegmentEditorEffects_PYTHON_SCRIPTS
  AbstractScriptedSegmentEditorEffect
  AbstractScriptedSegmentEditorLabelEffect
  AbstractScriptedSegmentEditorIslandEffect
  SegmentEditorMarginEffect
  SegmentEditorDrawEffect
  SegmentEditorLogicalEffect
  SegmentEditorAutoCompleteEffect
  SegmentEditorIdentifyIslandsEffect
  SegmentEditorFloodFillingEffect
  SegmentEditorLevelTracingEffect
  SegmentEditorSmoothingEffect
  SegmentEditorThresholdEffect
  )

set(SegmentEditorEffects_PYTHON_RESOURCES
  Resources/Icons/Margin.png
  Resources/Icons/Draw.png
  Resources/Icons/Logical.png
  Resources/Icons/AutoComplete.png
  Resources/Icons/IdentifyIslands.png
  Resources/Icons/FloodFilling.png
  Resources/Icons/LevelTracing.png
  Resources/Icons/Smoothing.png
  Resources/Icons/Threshold.png
  Resources/Icons/SaveIsland.png
  )

ctkMacroCompilePythonScript(
  TARGET_NAME SegmentEditorEffects
  SCRIPTS "${SegmentEditorEffects_PYTHON_SCRIPTS}"
  RESOURCES "${SegmentEditorEffects_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/SegmentEditorEffects
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/SegmentEditorEffects
  )
