PROJECT(VolumeRenderingCuda)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

## ONLY BUILD THIS IF YOU WANT CUDA SUPPORT TO BE ENABLED
IF(CUDA_SUPPORT_ENABLED)

OPTION(VOLUMERENDERINGCUDAMODULE "Would you like to build Volume Rendering with CUDA support" ON)
if(VOLUMERENDERINGCUDAMODULE)

INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/igtlutil
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkVolumeRenderingCudaConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkVolumeRenderingCudaConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

# --------------------------------------------------------------------------
# Sources
  SET(VolumeRenderingCuda_SRCS

  ### CUDA Volume Rendering Integration Classes ###
    vtkCudaVolumeMapper.cxx
    vtkCudaVolumeProperty.cxx    
    vtkCudaVolumeInformationHandler.cxx
    vtkCudaRendererInformationHandler.cxx
    vtkVolumeRenderingCudaFactory.cxx
    vtkVolumeRenderingCudaGUI.cxx
    vtkVolumeRenderingCudaLogic.cxx
  
  
  #### CUDA Pipeline Classes ####
    vtkCudaImageDataFilter.cxx
    vtkCudaImageData.cxx

  ## Special Classes
  vtkCudaMemoryTexture.cxx
    
  ## WIDGETS ##
    #vtkKWTypeChooserBox.cxx
  )
  SET(VolumeRenderingCuda_ADDITIONAL_HEADERS
    # Cuda Classes
      cudaTypeRange.h
      CUDA_renderBase.h
      CUDA_renderSlice.h
      CUDA_renderSingleSlice.h
      CUDA_typeRange.h
      cudaRendererInformation.h
      cudaVolumeInformation.h
      )
  
  SET(VolumeRenderingCuda_CUDA_SRCS
      CUDA_renderBase.cu
      CUDA_renderSlice.cu
      CUDA_renderSingleSlice.cu      
      CUDA_renderRayCastComposite.h
      CUDA_renderRayCastCompositeShaded.h
      CUDA_renderRayCastMIP.h
      CUDA_renderRayCastIsosurface.h
      CUDA_interpolation.h
      CUDA_matrix_math.h
  )

#-----------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${FOUND_CUT_INCLUDE}
  ${FOUND_CUDA_NVCC_INCLUDE}
  ${CUDA_INSTALL_PREFIX}/include
  ${CudaSupport_SOURCE_DIR}
  ${CudaSupport_BINARY_DIR}
  ${VolumeRenderingCuda_SOURCE_DIR}
  ${VolumeRenderingCuda_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
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
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(VolumeRenderingCuda
         VolumeRenderingCuda_TCL_SRCS 
         "${VolumeRenderingCuda_SRCS}" 
         "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(VolumeRenderingCuda_SRCS VolumeRenderingCuda.txt)

# --------------------------------------------------------------------------
# Build the library

set(lib_name VolumeRenderingCuda)

## Set the Libraries we link
SET(VolumeRenderingCuda_LIBRARIES
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
  ${VolumeRenderingCuda_SRCS}
  ${VolumeRenderingCuda_ADDITIONAL_HEADERS}
  ${VolumeRenderingCuda_TCL_SRCS}
  ${VolumeRenderingCuda_CUDA_SRCS}
  )

TARGET_LINK_LIBRARIES(${lib_name}
  ${VolumeRenderingCuda_LIBRARIES}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS ${lib_name}
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

#IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(Examples)
#ENDIF(BUILD_EXAMPLES)

CONFIGURE_FILE(
  ${VolumeRenderingCuda_SOURCE_DIR}/vtkVolumeRenderingCudaConfigure.h.in 
  ${VolumeRenderingCuda_BINARY_DIR}/vtkVolumeRenderingCudaConfigure.h
)
CONFIGURE_FILE(
  ${VolumeRenderingCuda_SOURCE_DIR}/presets.xml
  ${VolumeRenderingCuda_BINARY_DIR}/presets.xml
  COPYONLY
)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
)

slicer3_set_modules_output_path(${lib_name})
slicer3_install_modules(${lib_name})

ENDIF(VOLUMERENDERINGCUDAMODULE)
ENDIF(CUDA_SUPPORT_ENABLED)
