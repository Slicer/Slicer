
if(Slicer_EP_LABEL_REMOTE_MODULE)
  foreach(varname ${Slicer_EP_LABEL_REMOTE_MODULE})
    set(_source_dir ${${varname}})
    get_filename_component(_project_name ${_source_dir} NAME_WE)
    add_subdirectory(
      ${_source_dir}
      ${CMAKE_CURRENT_BINARY_DIR}/${_project_name}
      )
  endforeach()
endif()

if(Slicer_BUILD_CLI AND Slicer_BUILD_BRAINSTOOLS)
  # NOTE: BRAINSTools source code is checkout using 'Slicer_Remote_Add' called in SuperBuild.cmake
  set(BRAINSTools_CLI_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "")
  set(BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "")
  set(BRAINSTools_CLI_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR} CACHE PATH "")
  set(BRAINSTools_CLI_INSTALL_LIBRARY_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "")
  set(BRAINSTools_CLI_INSTALL_ARCHIVE_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "")
  set(BRAINSTools_CLI_INSTALL_RUNTIME_DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR} CACHE PATH "")

  if(BUILD_TESTING)
    set(LAUNCH_EXE ${SEM_LAUNCH_COMMAND})
  endif()

  set(USE_BRAINSFit                       ON CACHE BOOL "Build BRAINSFit"       )
  set(USE_BRAINSROIAuto                   ON CACHE BOOL "Build BRAINSROIAuto"   )
  set(USE_BRAINSResample                  ON CACHE BOOL "Build BRAINSResample"  )
  set(USE_BRAINSDemonWarp                 ON CACHE BOOL "Build BRAINSDemonWarp ")
  if("${ITK_VERSION_MAJOR}" GREATER 3)
    set(USE_BRAINSTransformConvert          ON CACHE BOOL "Build BRAINSTransformConvert ")
    set(USE_DWIConvert                      ${Slicer_BUILD_DICOM_SUPPORT} CACHE BOOL "Build DWIConvert ") ## Need to figure out library linking
  else()
    set(USE_BRAINSTransformConvert          OFF CACHE BOOL "Build BRAINSTransformConvert ")
    set(USE_DWIConvert                      OFF CACHE BOOL "Build DWIConvert ")
  endif()

  ## BRAINSTools comes with some extra tool
  ## options that should not be compiled by default.
  set(USE_AutoWorkup                     OFF CACHE BOOL "")
  set(USE_ANTS                           OFF CACHE BOOL "")
  set(USE_GTRACT                         OFF CACHE BOOL "")
  set(USE_BRAINSABC                      OFF CACHE BOOL "")
  set(USE_BRAINSTalairach                OFF CACHE BOOL "")
  set(USE_BRAINSConstellationDetector    OFF CACHE BOOL "")
  set(USE_BRAINSMush                     OFF CACHE BOOL "")
  set(USE_BRAINSInitializedControlPoints OFF CACHE BOOL "")
  set(USE_BRAINSMultiModeSegment         OFF CACHE BOOL "")
  set(USE_BRAINSCut                      OFF CACHE BOOL "")
  set(USE_BRAINSLandmarkInitializer      OFF CACHE BOOL "")
  set(USE_ImageCalculator                OFF CACHE BOOL "")
  set(USE_BRAINSSurfaceTools             OFF CACHE BOOL "")
  set(USE_BRAINSSnapShotWriter           OFF CACHE BOOL "")
  set(USE_ConvertBetweenFileFormats      OFF CACHE BOOL "")
  set(USE_BRAINSMultiSTAPLE              OFF CACHE BOOL "")
  set(USE_BRAINSCreateLabelMapFromProbabilityMaps OFF CACHE BOOL "")
  set(USE_BRAINSContinuousClass          OFF CACHE BOOL "")
  set(USE_ICCDEF                         OFF CACHE BOOL "")
  set(USE_BRAINSPosteriorToContinuousClass OFF CACHE BOOL "")
  set(USE_DebugImageViewer               OFF CACHE BOOL "")
  set(BRAINS_DEBUG_IMAGE_WRITE           OFF CACHE BOOL "")

  set(ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR} CACHE STRING "")
  set(BRAINSTools_SUPERBUILD             OFF CACHE BOOL "")
  add_subdirectory(
    ${BRAINSTools_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/BRAINSTools
    )
endif()
