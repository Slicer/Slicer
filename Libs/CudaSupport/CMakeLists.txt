project(CudaSupport)

if(CUDA_SUPPORT_ENABLED)

#include(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

# --------#
# Sources #
#---------#
 set(CudaSupport_SRCS
  ### BASIC CUDA RUNTIME API  CLASSES ###
    CudappBase.h
    CudappBase.cxx
    CudappSupport.h
    CudappSupport.cxx

    CudappDevice.h
    CudappDevice.cxx
    CudappEvent.h
    CudappEvent.cxx
    CudappStream.h
    CudappStream.cxx
##    CudappOpenGLBufferObject.cxx

  #### CUDA MEMORY RUNTIME CLASSES ####
    CudappMemoryBase.h
    CudappMemoryBase.cxx
    CudappMemory.h
    CudappMemory.cxx
    CudappDeviceMemory.h
    CudappDeviceMemory.cxx
    CudappHostMemory.h
    CudappHostMemory.cxx
    CudappLocalMemory.h
    CudappLocalMemory.cxx

    CudappMemoryPitch.h
    CudappMemoryPitch.cxx
    CudappMemoryArray.h
    CudappMemoryArray.cxx

 #### Specialized Classes ##
)

include_directories(
  ${FOUND_CUT_INCLUDE}
  ${FOUND_CUDA_NVCC_INCLUDE}
  ${CUDA_INSTALL_PREFIX}/include
)

CUDA_INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}
                          ${FOUND_CUT_INCLUDE}
                          ${FOUND_CUDA_NVCC_INCLUDE}
)

include_regular_expression("^.h$")

# --------------------------------------------------------------------------
# Wrapping

##DISABLED THE TIKL Wrapper
include_directories(
  ${CudaSupport_SOURCE_DIR}
  ${CudaSupport_BINARY_DIR}
  )
#include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(CudaSupport
#         CudaSupport_TCL_SRCS
#         "${CudaSupport_SRCS}"
#         "")

# --------------------------------------------------------------------------
# Build the library
set(CudaSupport_LIBRARIES
  SlicerBaseLogic
  SlicerBaseGUI
  MRML
  vtkCommon
  vtkImaging
  vtkFiltering
  )
if(VTK_WRAP_TCL)
  set(CudaSupport_LIBRARIES
    ${CudaSupport_LIBRARIES}
    vtkCommonTCL
    vtkImagingTCL
    vtkFilteringTCL
    )
endif(VTK_WRAP_TCL)
## Set the Libraries we link
set(CudaSupport_LIBRARIES
  ${CudaSupport_LIBRARIES}
  ${CUDA_TARGET_LINK}
 )

set(srcs
  ${CudaSupport_SRCS}
  )
if(VTK_WRAP_TCL)
  set(srcs
    ${srcs}
    ${CudaSupport_TCL_SRCS}
    )
endif(VTK_WRAP_TCL)
add_library(CudaSupport
  ${srcs}
  )

if(BUILD_SHARED_LIBS)
  install(TARGETS CudaSupport
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(CudaSupport
  ${CudaSupport_LIBRARIES}
  ${CudaSupport_CudaLibrary}
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
#if(BUILD_TESTING)
#  add_subdirectory(Testing)
#endif()


configure_file(
  ${CudaSupport_SOURCE_DIR}/CudappSupportConfigure.h.in
  ${CudaSupport_BINARY_DIR}/CudappSupportConfigure.h
  )

endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
