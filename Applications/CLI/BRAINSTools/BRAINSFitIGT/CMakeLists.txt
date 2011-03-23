project(BRAINSFitIGT)
set(LOCAL_PROJECT_NAME BRAINSFitIGT)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

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

find_package(BRAINSCommonLib NO_MODULE REQUIRED)
include(${BRAINSCommonLib_USE_FILE})

#-----------------------------------------------------------------------------
# Output directories.
#
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBRAINS3BuildMacros.cmake)
#SETOPTIONALDEBUGIMAGEVIEWER()

# No need to add with nothing to compile. add_subdirectory(BRAINSFitIGT_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSFitIGTCommonLib.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSFitIGTCommonLib.h
  )


###
set(CLP BRAINSFitIGT)
set(${CLP}_SOURCE BRAINSFitIGT.cxx  ${BRAINSFitIGT_SOURCE_DIR}/BRAINSFitIGTPrimary.cxx)
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSFitIGTPrimary.xml "${${CLP}_SOURCE}" "")
target_link_libraries( ${CLP} BRAINSCommonLib ITKIO ITKAlgorithms ${OPTIONAL_DEBUG_LINK_LIBRARIES})

INSTALL(TARGETS BRAINSFitIGT DESTINATION bin)

# not yet
#if(BUILD_TESTING)
#  add_subdirectory(TestSuite)
#endif(BUILD_TESTING)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BRAINSFitIGT Image Registration")
set(CPACK_PACKAGE_VENDOR "Surgical Planning Lab, Brigham and Women's Hospital")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_EXECUTABLES "BRAINSFitIGT")
if(APPLE)
set(CPACK_GENERATOR "TGZ;PackageMaker")
elseif(LINUX)
set(CPACK_GENERATOR "TGZ;RPM")
endif()

include(CPack)
