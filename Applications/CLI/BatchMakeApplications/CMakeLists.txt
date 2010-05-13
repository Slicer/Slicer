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

  find_path(Slicer3_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  if(Slicer3_BINARY_DIR)
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

  endif(Slicer3_BINARY_DIR)

  if (Slicer3_SOURCE_DIR)
    set(RUNTIME_SUPPORT_FILES 
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm" )
    install(FILES ${RUNTIME_SUPPORT_FILES} 
      DESTINATION ${Slicer3_INSTALL_PLUGINS_SHARE_DIR})

  endif(Slicer3_SOURCE_DIR)
endif( BatchMake_DIR)

SET(batchmake_modules
#  EMSegmentBatchMakeModule
  GaussianBlurBatchMakeModule
  RegisterImagesBatchMakeModule
  ResampleVolumeBatchMakeModule
  )
FOREACH(module ${batchmake_modules})
  ADD_SUBDIRECTORY(${module})
ENDFOREACH(module)
