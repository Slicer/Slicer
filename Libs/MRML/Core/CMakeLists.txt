project(MRMLCore)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(${ITK_VERSION_MAJOR} GREATER 3)
  set(${CMAKE_PROJECT_NAME}_ITK_COMPONENTS
    ITKCommon
    ITKIOImageBase
    ITKIOSpatialObjects
    ITKIOTransformBase
    ITKTransform
    )
  find_package(ITK COMPONENTS ${${CMAKE_PROJECT_NAME}_ITK_COMPONENTS})
else()
  find_package(ITK REQUIRED)
endif()
if(${ITK_VERSION_MAJOR} GREATER 3)
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
  list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
  list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
endif()
include(${ITK_USE_FILE})

option(MRML_USE_Teem "Build MRML with Teem support." ON)
mark_as_advanced(MRML_USE_Teem)
if(MRML_USE_Teem)
  find_package(Teem REQUIRED)
  include(${Teem_USE_FILE})
  option(MRML_USE_vtkTeem "Build MRML with vtkTeem support." ON)
  mark_as_advanced(MRML_USE_vtkTeem)
endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# MRML scenes that are older than MRML_SUPPORT_VERSION won't be supported.
# Or said differently, MRML_SUPPORT_VERSION is the earliest version supported
# for a MRML scene.
# The format is: 0xXXYYZZ where XX is the major version, YY the minor and
# ZZ the patch version.
# For example, set MRML_SUPPORT_VERSION to 0x040000 if you don't want to
# support scenes of Slicer 3.
# Supports Slicer3 and Slicer4 scenes by default.
set(MRML_SUPPORT_VERSION 0x030000 CACHE STRING "MRML scene version to support " )
mark_as_advanced(MRML_SUPPORT_VERSION)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkITK_INCLUDE_DIRS}
  ${FreeSurfer_INCLUDE_DIRS}
  )
if(MRML_USE_Teem)
  list(APPEND include_dirs ${vtkTeem_INCLUDE_DIRS})
endif()
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkMRMLConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(MRMLCore_SRCS
  vtkEventBroker.cxx
  vtkImageAccumulateDiscrete.cxx
  vtkImageBimodalAnalysis.cxx
  vtkDataFileFormatHelper.cxx
  vtkMRMLLogic.cxx
  vtkMRMLAbstractViewNode.cxx
  vtkMRMLCameraNode.cxx
  vtkMRMLChartNode.cxx
  vtkMRMLChartViewNode.cxx
  vtkMRMLClipModelsNode.cxx
  vtkMRMLColorNode.cxx
  vtkMRMLColors.cxx
  vtkMRMLColorTableNode.cxx
  vtkMRMLColorTableStorageNode.cxx
  vtkMRMLCoreTestingMacros.cxx
  vtkMRMLCrosshairNode.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkMRMLDiffusionWeightedVolumeNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLFiducial.cxx
  vtkMRMLFiducialListNode.cxx
  vtkMRMLFiducialListStorageNode.cxx
  vtkMRMLFreeSurferModelOverlayStorageNode.cxx
  vtkMRMLFreeSurferModelStorageNode.cxx
  vtkMRMLFreeSurferProceduralColorNode.cxx
  vtkMRMLHierarchyNode.cxx
  vtkMRMLHierarchyStorageNode.cxx
  vtkMRMLDisplayableHierarchyNode.cxx
  vtkMRMLInteractionNode.cxx
  vtkMRMLLabelMapVolumeDisplayNode.cxx
  vtkMRMLLinearTransformNode.cxx
  vtkMRMLModelDisplayNode.cxx
  vtkMRMLModelHierarchyNode.cxx
  vtkMRMLModelNode.cxx
  vtkMRMLModelStorageNode.cxx
  vtkMRMLNode.cxx
  vtkMRMLParser.cxx
  vtkMRMLProceduralColorNode.cxx
  vtkMRMLROIListNode.cxx
  vtkMRMLROINode.cxx
  vtkMRMLScalarVolumeDisplayNode.cxx
  vtkMRMLScalarVolumeNode.cxx
  vtkMRMLScene.cxx
  vtkMRMLSceneViewNode.cxx
  vtkMRMLSceneViewStorageNode.cxx
  vtkMRMLScriptedModuleNode.cxx
  vtkMRMLScriptedModuleNode.h
  vtkMRMLSelectionNode.cxx
  vtkMRMLSliceCompositeNode.cxx
  vtkMRMLSliceNode.cxx
  vtkMRMLSnapshotClipNode.cxx
  vtkMRMLStorableNode.cxx
  vtkMRMLStorageNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLTransformStorageNode.cxx
  vtkMRMLTransformableNode.cxx
  vtkMRMLUnitNode.cxx
  vtkMRMLUnstructuredGridDisplayNode.cxx
  vtkMRMLUnstructuredGridNode.cxx
  vtkMRMLUnstructuredGridStorageNode.cxx
  vtkMRMLVectorVolumeDisplayNode.cxx
  vtkMRMLViewNode.cxx
  vtkMRMLVolumeArchetypeStorageNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNode.cxx
  vtkMRMLGlyphableVolumeDisplayPropertiesNode.cxx
  vtkMRMLVolumeHeaderlessStorageNode.cxx
  vtkMRMLVolumeNode.cxx
  vtkObservation.cxx
  vtkObserverManager.cxx
  vtkMRMLLayoutNode.cxx
  # Classes for remote data handling:
  vtkCacheManager.cxx
  vtkDataIOManager.cxx
  vtkDataTransfer.cxx
  vtkMRMLBSplineTransformNode.cxx
  vtkMRMLGridTransformNode.cxx
  vtkMRMLNonlinearTransformNode.cxx
  vtkPermissionPrompter.cxx
  vtkURIHandler.cxx
  vtkTagTableCollection.cxx
  vtkTagTable.cxx
  vtkMRMLdGEMRICProceduralColorNode.cxx
  vtkMRMLPETProceduralColorNode.cxx
  # Classes for 2D Plotting
  vtkMRMLDoubleArrayNode.cxx
  vtkMRMLDoubleArrayStorageNode.cxx
  # Note: when you add new node types, add
  # register code to the vtkMRMLScene constructor
  )

if(MRML_USE_Teem)
  if(MRML_USE_vtkTeem)
    set(MRMLCore_SRCS
      ${MRMLCore_SRCS}
      vtkMRMLDiffusionImageVolumeNode.cxx
      vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
      vtkMRMLDiffusionTensorVolumeNode.cxx
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx
      vtkMRMLNRRDStorageNode.cxx
      vtkMRMLTensorVolumeNode.cxx
      vtkMRMLVectorVolumeNode.cxx
      )
  endif(MRML_USE_vtkTeem)
endif(MRML_USE_Teem)

# Abstract/pure virtual classes

set_source_files_properties(
  vtkMRMLNode.cxx
  vtkMRMLVolumeNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  ABSTRACT
  )

string(REGEX REPLACE "\\.cxx" ".h" MRMLCore_SRCS_HEADERS "${MRMLCore_SRCS}")
source_group("Header Files" FILES ${MRMLCore_SRCS_HEADERS})

#-----------------------------------------------------------------------------
# Wrapping hints used by all Tcl, Python and Java wrapping.
# It can also be used by external wrapping tools.
set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(srcs ${MRMLCore_SRCS} ${MRMLCore_SRCS_HEADERS})

set(lib_name ${PROJECT_NAME})
add_library(${lib_name} ${srcs})


set(VTK_LIBRARIES
  #vtkCommon # Provided by vtkITK, FreeSurfer
  #vtkFiltering # Provided by vtkITK, FreeSurfer
  vtkGraphics
  vtkHybrid
  vtkInfovis
  #vtkIO # Provided by FreeSurfer
  #vtkImaging # Provided by vtkITK
  )

set(libs
  vtkITK
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  FreeSurfer
  )
if(MRML_USE_vtkTeem)
  list(APPEND libs vtkTeem)
endif()
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${MRMLCore_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
  # Export target
  export(TARGETS ${lib_name}Python ${lib_name}PythonD APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
endif()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install Test Data
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  #file(GLOB_RECURSE MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
  #file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
  #file(GLOB_RECURSE XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.xml")
  #file(GLOB_RECURSE GZFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.gz")
  #file(GLOB_RECURSE NHDRFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nhdr")
  #file(GLOB_RECURSE NRRDFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nrrd")
  #file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

  #foreach(file
  #    ${MRMLFILES}
  #    ${TCLFILES}
  #    ${XMLFILES}
  #    ${GZFILES}
  #    ${NHDRFILES}
  #    ${NRRDFILES}
  #    ${VTKFILES}
  #    )
  #  message(file:${file})
  #  configure_file(
  #    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
  #    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
  #    COPYONLY
  #    )
  #endforeach()

  install(DIRECTORY
    Testing
    DESTINATION share/${PROJECT_NAME} COMPONENT Development
    PATTERN "*.cxx" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN "*.tcl" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    )
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
