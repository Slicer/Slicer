project(BRAINSCommonLibProject)
set(LOCAL_PROJECT_NAME BRAINSCommonLib)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
#include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

include(GenerateBRAINSCommonLibConfig.cmake)
if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${ARGV})
  endif(NOT ${KEY})
endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)

###
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SETIFEMPTY(CMAKE_BUNDLE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

###
#CHECKIFSLICER3BUILD()  ## Call the convenience macro

if(NOT ITK_FOUND)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)


#-----------------------------------------------------------------------------
# Output directories.
#

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSCommonLib.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(BRAINSCommonLib_SRCS GenericTransformImage.cxx BRAINSFitHelper.cxx Slicer3LandmarkIO.cxx)

add_library(BRAINSCommonLib ${BRAINSCommonLib_SRCS})
target_link_libraries(BRAINSCommonLib ITKAlgorithms ITKIO)


if (Slicer_SOURCE_DIR)

  # install each target in the production area (where it would appear in an
  # installation) and install each target in the developer area (for running
  # from a build)
  set(TARGETS
    BRAINSCommonLib
    )
  slicer3_install_plugins(${TARGETS})

else (Slicer_SOURCE_DIR)

  INSTALL(TARGETS BRAINSCommonLib
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib/BRAINSCommonLib
      ARCHIVE DESTINATION lib/BRAINSCommonLib
      )

  IF(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)
    FILE(GLOB __files1 "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    FILE(GLOB __files2 "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
    INSTALL(FILES ${__files1} ${__files2} ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib.h
      DESTINATION ${CMAKE_INSTALL_PREFIX}/include/BRAINSCommonLib
      COMPONENT Development)
  ENDIF(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BRAINSCommonLibInstallConfig.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibInstallConfig.cmake
                 @ONLY)
 
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibInstallConfig.cmake
          DESTINATION lib/BRAINSCommonLib
          COMPONENT Development
          RENAME BRAINSCommonLibConfig.cmake)

   INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/UseBRAINSCommonLib.cmake.in
           DESTINATION lib/BRAINSCommonLib
           COMPONENT Development
           RENAME UseBRAINSCommonLib.cmake)

endif (Slicer_SOURCE_DIR)

##HACK NEED BETTER TESTS add_directory( TestLargestForegroundFilledMaskImageFilter )
##HACK NEED BETTER TESTS add_directory( Test_FindCenterOfBrainFilter )
if(BUILD_TESTING)
  add_subdirectory(TestSuite)
endif(BUILD_TESTING)

