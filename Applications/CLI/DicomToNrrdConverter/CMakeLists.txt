project (DicomToNrrdConverter)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.0)
#-----------------------------------------------------------------------------


enable_testing()

if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

set(RUN_EXTENDED_LOCAL_TESTING OFF CACHE BOOL "Run extended tests?")

if (RUN_EXTENDED_LOCAL_TESTING)
  add_subdirectory(ExtendedTesting)
endif (RUN_EXTENDED_LOCAL_TESTING)

find_package(ITK REQUIRED)

if (NOT ITK_USE_REVIEW)
  message(FATAL_ERROR "ITK version was not built with ITK_USE_REVIEW true")
endif (NOT ITK_USE_REVIEW)

if (NOT ITK_USE_REVIEW_STATISTICS)
  message(FATAL_ERROR "ITK version was not built with ITK_USE_REVIEW_STATISTICS true")
endif (NOT ITK_USE_REVIEW_STATISTICS)

if (NOT ITK_USE_OPTIMIZED_REGISTRATION_METHODS)
  message(FATAL_ERROR "ITK version was not built with ITK_USE_OPTIMIZED_REGISTRATION_METHODS true")
endif (NOT ITK_USE_OPTIMIZED_REGISTRATION_METHODS)

if (Slicer_SOURCE_DIR)
  ##There's limited testing from within Slicer.  There's extensive testing when run
  ##at UIowa but the testing data isn't publicly accessible yet.  Other packages that
  ##use this tool will not run either set of tests
  add_subdirectory(Testing)
endif (Slicer_SOURCE_DIR)

set (CLP DicomToNrrdConverter)
set ( ${CLP}_SOURCE ${CLP}.cxx)

if (Slicer_SOURCE_DIR)
  generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
else (Slicer_SOURCE_DIR)
  find_package(GenerateCLP NO_MODULE REQUIRED)
  include(${GenerateCLP_USE_FILE})
  generateclp(${CLP}_SOURCE ${CLP}.xml)
endif (Slicer_SOURCE_DIR)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ${ITK_LIBRARIES})
set_target_properties(${CLP} PROPERTIES LABELS ${CLP})

if (Slicer_SOURCE_DIR)
  slicer3_set_plugins_output_path(${CLP})

  add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
  slicer3_set_plugins_output_path(${CLP}Lib)
  set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  target_link_libraries (${CLP}Lib ${ITK_LIBRARIES})
  set_target_properties(${CLP}Lib PROPERTIES LABELS ${CLP})

  set(TARGETS ${CLP}Lib ${CLP})
  slicer3_install_plugins(${TARGETS})
else (Slicer_SOURCE_DIR)

install(TARGETS
  DicomToNrrdConverter
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

endif (Slicer_SOURCE_DIR)
