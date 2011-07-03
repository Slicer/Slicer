project(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Third party library
# --------------------------------------------------------------------------
find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_INCLUDE_DIRS}
  ${MRMLCLI_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${ModuleDescriptionParser_INCLUDE_DIRS}
  ${vtkTeem_INCLUDE_DIRS}
  ${FreeSurfer_INCLUDE_DIRS}
  ${RemoteIO_INCLUDE_DIRS}
  ${LibArchive_INCLUDE_DIR}
  )

if(Slicer_USE_PYTHONQT)
  list(APPEND include_dirs
    ${PYTHON_INCLUDE_DIR}
    )
endif()

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Update Slicer_Base_INCLUDE_DIRS
# --------------------------------------------------------------------------
set(Slicer_Base_INCLUDE_DIRS ${Slicer_Base_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "Slicer Base includes" FORCE)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkSlicerBaseLogicConfigure.h)
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
set(SlicerBaseLogic_SRCS
  # Logic classes (data management and calculation)
  vtkSlicerApplicationLogic.cxx
  vtkSlicerModuleLogic.cxx
  vtkSlicerTask.cxx
  vtkSlicerModelsLogic.cxx
  vtkSlicerModelHierarchyLogic.cxx
  vtkSlicerFiducialsLogic.cxx
  vtkSlicerColorLogic.cxx
  vtkDataIOManagerLogic.cxx
  # slicer's vtk extensions (filters)
  vtkSlicerExtensionsLogic.cxx
  vtkArchive.cxx
  vtkImageSlicePaint.cxx
  vtkImageFillROI.cxx
  vtkSlicerGlyphSource2D.cxx
  vtkSlicerROILogic.cxx
  vtkImageConnectivity.cxx
  vtkImageLabelChange.cxx
  vtkImageErode.cxx
  vtkSlicerTransformLogic.cxx
  vtkPichonFastMarching.cxx
  vtkPichonFastMarchingPDF.cxx
  vtkImageRectangularSource.cxx
  vtkSystemInformation.cxx
  vtkImageStash.cxx
  )

if(Slicer_USE_PYTHONQT)
  list(APPEND SlicerBaseLogic_SRCS
    vtkSlicerScriptedLoadableModuleLogic.cxx
    )
endif()

set_source_files_properties(
  vtkPichonFastMarchingPDF.cxx
  vtkArchive.cxx
  WRAP_EXCLUDE
)

# --------------------------------------------------------------------------
## Create an instantiator for these classes
# set(VTK_USE_INSTANTIATOR_NEW 1)
# VTK_MAKE_INSTANTIATOR3("${lib_name}Instantiator"
#   Instantiator_SRCS
#   "${SlicerBaseLogic_SRCS}"
#   "VTK_SLICER_BASE_LOGIC_EXPORT"
#   ${Slicer_BINARY_DIR}
#   "vtkSlicerBaseLogic.h")

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name SlicerBaseLogic)

#set(srcs ${SlicerBaseLogic_SRCS} ${Instantiator_SRCS})
set(srcs ${SlicerBaseLogic_SRCS})

add_library(${lib_name} ${srcs})

set(libs
  MRML
  MRMLCLI
  MRMLLogic
  vtkITK
  ${OPENTRACKER_LIB}
  FreeSurfer
  ${VTK_LIBRARIES}
  RemoteIO
  ${LibArchive_LIBRARY}
  )

if(Slicer_USE_PYTHONQT)
  list(APPEND libs
    vtkCommonPythonD # For vtkPythonUtils
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
endif()

target_link_libraries(${lib_name} ${libs})
set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${lib_name})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${SlicerBaseLogic_SRCS}
    KIT_PYTHON_LIBRARIES MRMLLogicPythonD
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    )
endif()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------

# Should this location change, update Base/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${PROJECT_NAME}/${colorfile}
    COPYONLY
    )
endforeach()
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources/ColorFiles
  )
