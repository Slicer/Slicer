project(BRAINSCommonLibProject)
set(LOCAL_PROJECT_NAME BRAINSCommonLib)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

include(GenerateBRAINSCommonLibConfig.cmake)

###
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SETIFEMPTY(CMAKE_BUNDLE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

###
CHECKIFSLICER3BUILD()  ## Call the convenience macro

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

set(BRAINSCommonLib_SRCS GenericTransformImage.cxx BRAINSFitHelper.cxx)

add_library(BRAINSCommonLib ${BRAINSCommonLib_SRCS})
target_link_libraries(BRAINSCommonLib ITKAlgorithms ITKIO)

INSTALL(TARGETS BRAINSCommonLib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

IF(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)
  FILE(GLOB __files1 "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  FILE(GLOB __files2 "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
  INSTALL(FILES ${__files1} ${__files2} ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/BRAINSCommonLib
    COMPONENT Development)
ENDIF(NOT BRAINSCommonLibProject_INSTALL_NO_DEVELOPMENT)

