
if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_EMSegment)
  # NOTE: EMSegment source code is checkout from Slicer3 repository using
  #       "External_EMSegment.cmake".
  add_subdirectory(
    ${EMSegment_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/EMSegment
    )
endif()

if(Slicer_USE_OpenIGTLink AND Slicer_BUILD_OpenIGTLinkIF)
  # NOTE: OpenIGTLinkIF source code is checkout from NAMICSandBox repository using
  #       "External_OpenIGTLinkIF.cmake".
  add_subdirectory(
    ${OpenIGTLinkIF_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/OpenIGTLinkIF
    )
endif()

if(Slicer_BUILD_MultiVolumeExplorer AND Slicer_BUILD_DICOM_SUPPORT)
  # NOTE: MultiVolumeExplorer source code is checkout using "External_MultiVolumeExplorer.cmake".
  add_subdirectory(
    ${MultiVolumeExplorer_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeExplorer
    )
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  # NOTE: MultiVolumeImporter source code is checkout using "External_MultiVolumeImporter.cmake".
  add_subdirectory(
    ${MultiVolumeImporter_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeImporter
    )
endif()


if(Slicer_BUILD_SimpleFilters)
  # NOTE: SimpleFilters source is checkout in Superbuild
  add_subdirectory(
    ${SimpleFilters_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleFilters
    )
endif()

if(Slicer_BUILD_BRAINSTOOLS)
  # NOTE: BRAINSTools source code is checkout using "External_BRAINSTools.cmake".
  set(BRAINSTools_CLI_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_LIBRARY_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_ARCHIVE_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_RUNTIME_DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR} CACHE PATH "" FORCE)

  if(BUILD_TESTING)
    set(LAUNCH_EXE ${SEM_LAUNCH_COMMAND})
  endif()

  set(USE_BRAINSFit                       ON CACHE BOOL "Build BRAINSFit"       )
  set(USE_BRAINSROIAuto                   ON CACHE BOOL "Build BRAINSROIAuto"   )
  set(USE_BRAINSResample                  ON CACHE BOOL "Build BRAINSResample"  )
  set(USE_BRAINSDemonWarp                 ON CACHE BOOL "Build BRAINSDemonWarp ")
  if("${ITK_VERSION_MAJOR}" GREATER 3)
    set(USE_BRAINSTransformConvert          ON CACHE BOOL "Build BRAINSTransformConvert ")
    set(USE_DWIConvert                      ON CACHE BOOL "Build DWIConvert ") ## Need to figure out library linking
  else()
    set(USE_BRAINSTransformConvert          OFF CACHE BOOL "Build BRAINSTransformConvert ")
    set(USE_DWIConvert                      OFF CACHE BOOL "Build DWIConvert ")
  endif()

  ## BRAINSTools comes with some extra tool
  ## options that should not be compiled by default.
  set(USE_AutoWorkup                     OFF CACHE BOOL "" FORCE)
  set(USE_ANTS                           OFF CACHE BOOL "Build with ANTS capabilities" )
  set(USE_GTRACT                         OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSABC                      OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSTalairach                OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSConstellationDetector    OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSMush                     OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSInitializedControlPoints OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSMultiModeSegment         OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSCut                      OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSLandmarkInitializer      OFF CACHE BOOL "" FORCE)
  set(USE_ImageCalculator                OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSSurfaceTools             OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSSnapShotWriter           OFF CACHE BOOL "" FORCE)
  set(USE_ConvertBetweenFileFormats      OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSMultiSTAPLE              OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSCreateLabelMapFromProbabilityMaps OFF OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSContinuousClass          OFF CACHE BOOL "" FORCE)
  set(USE_ICCDEF                         OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSPosteriorToContinuousClass OFF CACHE BOOL "" FORCE)
  set(USE_DebugImageViewer               OFF CACHE BOOL "" FORCE)
  set(BRAINS_DEBUG_IMAGE_WRITE           OFF CACHE BOOL "" FORCE)

  # HACK - Ensure BRAINSTools uses the same ExternalData version as Slicer.
  #        See https://github.com/BRAINSia/BRAINSTools/issues/3
  configure_file(
    ${CMAKE_SOURCE_DIR}/CMake/ExternalData.cmake
    ${BRAINSTools_SOURCE_DIR}/CMake/ExternalData.cmake
    COPYONLY
    )

  set(ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR} CACHE STRING "")
  set(BRAINSTools_SUPERBUILD             OFF CACHE BOOL "" FORCE)
  add_subdirectory(
    ${BRAINSTools_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/BRAINSTools
    )
endif()
