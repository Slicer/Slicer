PROJECT(VolumeRenderingCuda1)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

## ONLY BUILD THIS IF YOU WANT CUDA SUPPORT TO BE ENABLED
IF(CUDA_SUPPORT_ENABLED)

INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources
  SET(VolumeRenderingCuda1_SRCS
    vtkCudaVolumeMapper.cxx
    vtkCudaVolumeInformationHandler.cxx
    vtkCudaRendererInformationHandler.cxx
    vtkCudaMemoryTexture.cxx
  )
  SET(VolumeRenderingCuda1_ADDITIONAL_HEADERS
      vtkVolumeRenderingCuda.h
      vtkVolumeRenderingCudaWin32Header
      CUDA_renderBase.h
      CUDA_typeRange.h
      cudaRendererInformation.h
      cudaVolumeInformation.h
      )
  
  SET(VolumeRenderingCuda1_CUDA_SRCS
      CUDA_renderBase.cu
      CUDA_renderRayCastComposite.h
      CUDA_renderRayCastCompositeShaded.h
      CUDA_interpolation.h
      CUDA_matrix_math.h
      CUDA_zbuffer_math.h
  )

#-----------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${FOUND_CUT_INCLUDE}
  ${FOUND_CUDA_NVCC_INCLUDE}
  ${CUDA_INSTALL_PREFIX}/include
  ${CudaSupport_SOURCE_DIR}
  ${CudaSupport_BINARY_DIR}
  ${VolumeRenderingCuda1_SOURCE_DIR}
  ${VolumeRenderingCuda1_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

 CUDA_INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} 
                           ${FOUND_CUT_INCLUDE}
                           ${FOUND_CUDA_NVCC_INCLUDE}
                           ${VTK_INCLUDE_DIRS}
                           ${CudaSupport_SOURCE_DIR}

 )

# --------------------------------------------------------------------------
# Build the library

set(lib_name VolumeRenderingCuda1)

## Set the Libraries we link
SET(VolumeRenderingCuda1_LIBRARIES
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ${KWWidgets_LIBRARIES}
  ${CUDA_TARGET_LINK}
  CudaSupport
 )

CUDA_ADD_LIBRARY(${lib_name}
  ${VolumeRenderingCuda1_SRCS}
  ${VolumeRenderingCuda1_ADDITIONAL_HEADERS}
  ${VolumeRenderingCuda1_TCL_SRCS}
  ${VolumeRenderingCuda1_CUDA_SRCS}
  )

TARGET_LINK_LIBRARIES(${lib_name}
  ${VolumeRenderingCuda1_LIBRARIES}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS ${lib_name}
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

slicer3_set_modules_output_path(${lib_name})
slicer3_install_modules(${lib_name})

ELSE(CUDA_SUPPORT_ENABLED)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources
  SET(VolumeRenderingCuda1_SRCS

  ### CUDA Volume Rendering Integration Classes ###
    vtkCudaVolumeMapperDummy.cxx
  )

#-----------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${VolumeRenderingCuda1_SOURCE_DIR}
  ${VolumeRenderingCuda1_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

 INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} 
                      ${VTK_INCLUDE_DIRS}
 )

# --------------------------------------------------------------------------
# Build the library

set(lib_name VolumeRenderingCuda1)

## Set the Libraries we link
SET(VolumeRenderingCuda1_LIBRARIES
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ${KWWidgets_LIBRARIES}
 )

ADD_LIBRARY(${lib_name}
  ${VolumeRenderingCuda1_SRCS}
  ${VolumeRenderingCuda1_TCL_SRCS}
  )

TARGET_LINK_LIBRARIES(${lib_name}
  ${VolumeRenderingCuda1_LIBRARIES}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS ${lib_name}
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

slicer3_set_modules_output_path(${lib_name})
slicer3_install_modules(${lib_name})

ENDIF(CUDA_SUPPORT_ENABLED)
