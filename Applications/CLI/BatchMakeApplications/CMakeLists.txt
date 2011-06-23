find_package(BatchMake REQUIRED)
include(${BatchMake_USE_FILE})

if(BatchMake_DIR)
  #find_program(bmGridSend
  #  NAMES bmGridSend.exe bmGridSend
  #  PATHS ${BatchMake_DIR}/bin/relwithdebinfo
  #  ${BatchMake_DIR}/bin/release
  #  ${BatchMake_DIR}/bin/debug
  #  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
  #  ${BatchMake_DIR}/bin)

  #find_program(bmGridStore
  #  NAMES bmGridStore.exe bmGridStore
  #  PATHS ${BatchMake_DIR}/bin/relwithdebinfo
  #  ${BatchMake_DIR}/bin/release
  #  ${BatchMake_DIR}/bin/debug
  #  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
  #  ${BatchMake_DIR}/bin)

  #find_program(bmSliceExtractor
  #  NAMES bmSliceExtractor.exe bmSliceExtractor
  #  PATHS ${BatchMake_DIR}/bin/relwithdebinfo
  #  ${BatchMake_DIR}/bin/release
  #  ${BatchMake_DIR}/bin/debug
  #  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
  #  ${BatchMake_DIR}/bin)
  set( bmGridSend ${BatchMake_GRID_SEND_EXE} )
  set( bmGridStore ${BatchMake_GRID_STORE_EXE} )
  set( bmGridExtractor ${BatchMake_SLICE_EXTRACTOR_EXE} )

  find_path(Slicer_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  if(Slicer_BINARY_DIR)
    #
    # TODO The following code will have to be updated to copy the file at build time
    # using a command like:
    #  execute_process(${CMAKE_COMMAND} -E copy ...) and using
    # CMAKE_CFG_INTDIR instead of CMAKE_BUILD_TYPE
    #
    if(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    else(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif(WIN32)

    # Copy the wrapped application for BatchMake.
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmSliceExtractor.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm")
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridSend.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm")
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridStore.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm")

  endif(Slicer_BINARY_DIR)

  if (Slicer_SOURCE_DIR)
    set(RUNTIME_SUPPORT_FILES
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm" )
    install(FILES ${RUNTIME_SUPPORT_FILES}
      DESTINATION ${Slicer_INSTALL_PLUGINS_SHARE_DIR})

  endif(Slicer_SOURCE_DIR)
endif( BatchMake_DIR)

set(batchmake_modules
#  EMSegmentBatchMakeModule
  GaussianBlurBatchMakeModule
  RegisterImagesBatchMakeModule
  ResampleVolumeBatchMakeModule
  )
foreach(module ${batchmake_modules})
  add_subdirectory(${module})
endforeach(module)
