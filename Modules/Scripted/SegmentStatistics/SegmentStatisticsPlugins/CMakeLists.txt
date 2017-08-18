set(SegmentStatisticsPlugins_PYTHON_SCRIPTS
  __init__
  SegmentStatisticsPluginBase
  LabelmapSegmentStatisticsPlugin
  ScalarVolumeSegmentStatisticsPlugin
  ClosedSurfaceSegmentStatisticsPlugin
  )

set(SegmentStatisticsPlugins_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME SegmentStatisticsPlugins
  SCRIPTS "${SegmentStatisticsPlugins_PYTHON_SCRIPTS}"
  RESOURCES "${SegmentStatisticsPlugins_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/SegmentStatisticsPlugins
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/SegmentStatisticsPlugins
  NO_INSTALL_SUBDIR
  )
