project(BRAINSFit)
set(LOCAL_PROJECT_NAME BRAINSFit)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
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


# No need to add with nothing to compile. add_subdirectory(BRAINSFit_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

###
set(ALLBRAINSFIT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSFitPrimary.cxx)
add_library(BRAINSFITCOMMONLIB ${ALLBRAINSFIT_SOURCE} BRAINSFitPrimaryCLP.h)
target_link_libraries(BRAINSFITCOMMONLIB BRAINSCommonLib ITKIO ITKAlgorithms ${OPTIONAL_DEBUG_LINK_LIBRARIES})

set(CLP BRAINSFit)
set(${CLP}_SOURCE BRAINSFit.cxx )
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSFitPrimary.xml "${${CLP}_SOURCE}" BRAINSFITCOMMONLIB)
target_link_libraries( ${CLP} BRAINSFITCOMMONLIB BRAINSCommonLib ITKIO ITKAlgorithms ${OPTIONAL_DEBUG_LINK_LIBRARIES})

#if(BUILD_TESTING)
  #  add_subdirectory(TestData)
#endif()

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    BRAINSFit
    BRAINSFITCOMMONLIB 
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "4")
set(CPACK_PACKAGE_VERSION_PATCH "1")

set(CPACK_PACKAGE_NAME "${LOCAL_PROJECT_NAME}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_NAME}-A program for medical image registration with mutual information metric.")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-dev")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${${LOCAL_PROJECT_NAME}_SOURCE_DIR}/ReadMe.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${${LOCAL_PROJECT_NAME}_SOURCE_DIR}/Copyright.txt")

set(CPACK_PACKAGE_DEFAULT_LOCATION "/opt/${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_VENDOR "NAMIC Tool Developed at The University of Iowa")

set(CPACK_SOURCE_GENERATOR "TGZ;TZ")
#set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/\\.svn/;\\.swp$;\\.#;/#;\\.*~")
set(CPACK_SOURCE_IGNORE_FILES "")
set(CPACK_PACKAGE_EXECUTABLES "${LOCAL_PROJECT_NAME}";"A program for registering medical images with mutual information.")

if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.nitrc.org")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.nitrc.org")
  set(CPACK_NSIS_CONTACT "hans-johnson@uiowa.edu")
  set(CPACK_NSIS_MODIFY_PATH ON)
else(WIN32 AND NOT UNIX)
  set(CPACK_STRIP_FILES OFF)
  set(CPACK_SOURCE_STRIP_FILES OFF)
endif(WIN32 AND NOT UNIX)

include(CPack)

