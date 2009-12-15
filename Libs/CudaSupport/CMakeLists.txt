PROJECT(CudaSupport)

IF(CUDA_SUPPORT_ENABLED)

#INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

# --------#
# Sources #
#---------#
 SET(CudaSupport_SRCS
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

INCLUDE_DIRECTORIES(
  ${FOUND_CUT_INCLUDE}
  ${FOUND_CUDA_NVCC_INCLUDE}
  ${CUDA_INSTALL_PREFIX}/include
)

CUDA_INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} 
                          ${FOUND_CUT_INCLUDE}
                          ${FOUND_CUDA_NVCC_INCLUDE}
)

INCLUDE_REGULAR_EXPRESSION("^.h$")

# --------------------------------------------------------------------------
# Wrapping

##DISABLED THE TIKL Wrapper
INCLUDE_DIRECTORIES(
  ${CudaSupport_SOURCE_DIR}
  ${CudaSupport_BINARY_DIR}
  )
#INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(CudaSupport
#         CudaSupport_TCL_SRCS 
#         "${CudaSupport_SRCS}" 
#         "")

# --------------------------------------------------------------------------
# Build the library
SET(CudaSupport_LIBRARIES
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkCommon
  vtkImaging
  vtkFiltering
  )
IF (VTK_WRAP_TCL)
  SET(CudaSupport_LIBRARIES
    ${CudaSupport_LIBRARIES}
    vtkCommonTCL
    vtkImagingTCL
    vtkFilteringTCL
    )
ENDIF (VTK_WRAP_TCL)
## Set the Libraries we link
SET(CudaSupport_LIBRARIES
  ${CudaSupport_LIBRARIES}
  ${CUDA_TARGET_LINK}
 )

SET(srcs
  ${CudaSupport_SRCS}
  )
IF (VTK_WRAP_TCL)
  SET(srcs
    ${srcs}
    ${CudaSupport_TCL_SRCS}
    )
ENDIF (VTK_WRAP_TCL)
ADD_LIBRARY(CudaSupport
  ${srcs}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS CudaSupport
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(CudaSupport
  ${CudaSupport_LIBRARIES}
  ${CudaSupport_CudaLibrary}
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

#IF(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
#ENDIF(BUILD_TESTING)


CONFIGURE_FILE(
  ${CudaSupport_SOURCE_DIR}/CudappSupportConfigure.h.in 
  ${CudaSupport_BINARY_DIR}/CudappSupportConfigure.h
)

ENDIF(CUDA_SUPPORT_ENABLED)
