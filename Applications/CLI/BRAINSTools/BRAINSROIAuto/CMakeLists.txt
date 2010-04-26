project(BRAINSROIAuto)
set(LOCAL_PROJECT_NAME BRAINSROIAuto)
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

set(CLP BRAINSROIAuto)
set(${CLP}_SOURCE BRAINSROIAuto.cxx )
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSROIAuto.xml "${${CLP}_SOURCE}" "")
target_link_libraries( ${CLP} BRAINSCommonLib ITKIO ITKAlgorithms ${OPTIONAL_DEBUG_LINK_LIBRARIES})

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    BRAINSROIAuto
    )
  slicer3_install_plugins(${TARGETS})
endif (Slicer3_SOURCE_DIR)

if(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${BRAINSROIAuto_BINARY_DIR}/bin)
endif(BRAINS_BUILD)

add_executable(BRATestImage TestImage.cxx)
target_link_libraries(BRATestImage
${ITK_LIBRARIES}
)
set_target_properties(BRATestImage PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${BRAINSROIAuto_BINARY_DIR})

add_executable(BRATestCompare BRATestCompare.cxx)
target_link_libraries(BRATestCompare
${ITK_LIBRARIES}
)
set_target_properties(BRATestCompare PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${BRAINSROIAuto_BINARY_DIR})

add_test(BRATestImage BRATestImage 
${BRAINSROIAuto_BINARY_DIR}/sphere.nii.gz
${BRAINSROIAuto_BINARY_DIR}eneratedMaskSphere.nii.gz
)
add_test(RunBRAINSROIAuto ${TESTING_BINARY_DIR}/BRAINSROIAuto 
--inputVolume ${BRAINSROIAuto_BINARY_DIR}/sphere.nii.gz
--outputClippedVolumeROI ${BRAINSROIAuto_BINARY_DIR}/clippedSphere.nii.gz
--outputROIMaskVolume ${BRAINSROIAuto_BINARY_DIR}/maskSphere.nii.gz)

add_test(CompareMasks BRATestCompare
${BRAINSROIAuto_BINARY_DIR}eneratedMaskSphere.nii.gz
${BRAINSROIAuto_BINARY_DIR}/maskSphere.nii.gz)
