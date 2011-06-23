project(BRAINSCommonLibProject)
set(LOCAL_PROJECT_NAME BRAINSCommonLib)
cmake_minimum_required(VERSION 2.8)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

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

## ITKv4 requires that ITKReview is explicitly linked against
if(ITK_VERSION_MAJOR EQUAL 4 )
  set(ITKREVIEWLIB ITKReview)
endif(ITK_VERSION_MAJOR EQUAL 4 )


add_library(BRAINSCommonLib ${BRAINSCommonLib_SRCS})
target_link_libraries(BRAINSCommonLib ITKAlgorithms ITKIO ${ITKREVIEWLIB})


if (Slicer_SOURCE_DIR)

  # install each target in the production area (where it would appear in an
  # installation) and install each target in the developer area (for running
  # from a build)
  set(TARGETS
    BRAINSCommonLib
    )
  slicer3_install_plugins(${TARGETS})

else (Slicer_SOURCE_DIR)

  install(TARGETS BRAINSCommonLib
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib/BRAINSCommonLib
      ARCHIVE DESTINATION lib/BRAINSCommonLib
      )

  if(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)
    file(GLOB __files1 "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    file(GLOB __files2 "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
    install(FILES ${__files1} ${__files2} ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib.h
      DESTINATION ${CMAKE_INSTALL_PREFIX}/include/BRAINSCommonLib
      COMPONENT Development)
  endif(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BRAINSCommonLibInstallConfig.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibInstallConfig.cmake
                 @ONLY)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibInstallConfig.cmake
          DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/BRAINSCommonLib
          COMPONENT Development
          RENAME BRAINSCommonLibConfig.cmake)

   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UseBRAINSCommonLib.cmake.in
     ${CMAKE_CURRENT_BINARY_DIR}/UseBRAINSCommonLib.cmake @ONLY)

   # install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/UseBRAINSCommonLib.cmake.in
   #         DESTINATION lib/BRAINSCommonLib
   #         COMPONENT Development
   #         RENAME UseBRAINSCommonLib.cmake)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/UseBRAINSCommonLib.cmake
     DESTINATION lib/BRAINSCommonLib
     COMPONENT Development)

endif (Slicer_SOURCE_DIR)

##HACK NEED BETTER TESTS add_directory( TestLargestForegroundFilledMaskImageFilter )
##HACK NEED BETTER TESTS add_directory( Test_FindCenterOfBrainFilter )
if(BUILD_TESTING)
  add_subdirectory(TestSuite)
endif()

