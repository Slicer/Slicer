PROJECT(BOUNDINGBOX)


# We define several classes in this example, and we want to be able to use
# their C++ methods as callbacks for our user interface. To do so, we need to 
# create a library and wrap it automatically for the Tcl language, which
# is used as a bridge between C++ objects at run-time. Note that an
# initialization function is automatically created in this library to allow
# classes and C++ methods to be used as commands and callbacks inside the Tcl
# interpreter; do *not* forget to call this function right after you 
# initialize the Tcl interpreter in your application. The name of this 
# function is built from the library name in lower-case (except for the first
# letter) and suffixed with "_Init" (for example: 
# KWR21ExampleLib => KwR21examplelib_Init)
# This whole process is required to use C++ methods as callbacks; it is not
# needed if you use VTK's C++ command/observer pattern directly, which is
# demonstrated in KWWidgets's C++ 'Callbacks' example.

# directories for vtkMIMX
INCLUDE_DIRECTORIES( ${BOUNDINGBOX_SOURCE_DIR}/../Common ${BOUNDINGBOX_BINARY_DIR}/../Common ) 
INCLUDE_DIRECTORIES( ${BOUNDINGBOX_SOURCE_DIR}/../Filter ${BOUNDINGBOX_BINARY_DIR}/../Filter ) 
INCLUDE_DIRECTORIES( ${BOUNDINGBOX_SOURCE_DIR}/../Widgets ${BOUNDINGBOX_BINARY_DIR}/../Widgets ) 

# --------------------------------------------------------------------------
# Include dirs
# MESSAGE(STATUS "QDecModle: Qdec binary dir = ${Qdec_BINARY_DIR}")
INCLUDE_DIRECTORIES(
  ${BOUNDINGBOX_SOURCE_DIR}
  ${BOUNDINGBOX_BINARY_DIR}
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

SET(LIB_NAME "BoundingBox")
SET(LIB_SRCS 
  vtkKWMimxApplication.cxx
  vtkKWMimxFEMeshMenuGroup.cxx
  vtkKWMimxGroupBase.cxx
  vtkKWMimxMainNotebook.cxx
  vtkKWMimxMainMenuGroup.cxx
  vtkKWMimxMenuWindow.cxx
  vtkKWMimxSurfaceMenuGroup.cxx
  vtkKWMimxViewWindow.cxx
  vtkKWMimxCreateBBFromBoundsGroup.cxx
  vtkKWMimxCreateBBMeshSeedGroup.cxx
  vtkKWMimxEditBBGroup.cxx
  vtkKWMimxCreateFEMeshFromBBGroup.cxx
  vtkKWMimxViewProperties.cxx
  vtkKWMimxSaveSTLSurfaceGroup.cxx
  vtkKWMimxSaveVTKSurfaceGroup.cxx
  vtkKWMimxSaveVTKBBGroup.cxx
  vtkKWMimxEditBBMeshSeedGroup.cxx
  vtkKWMimxSaveVTKFEMeshGroup.cxx
  vtkKWMimxDeleteObjectGroup.cxx
  vtkKWMimxEditFEMeshLaplacianSmoothGroup.cxx
  vtkMimxAbaqusFileWriter.cxx
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library. The library is built in static mode for convenience. 
# Check the 'Callbacks' example for more information about building it in
# shared mode, i.e. without the STATIC keyword (Win32 compilers requires
# an additional header file to setup DLL export symbols correctly).

ADD_LIBRARY(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} ${LIB_NAME})
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(${LIB_NAME}
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  ${KWWidgets_LIBRARIES} 
  mimxWidgets
  mimxFilter
  mimxCommon
  )


CONFIGURE_FILE(
  ${BOUNDINGBOX_SOURCE_DIR}/vtkBoundingBoxConfigure.h.in 
  ${BOUNDINGBOX_BINARY_DIR}/vtkBoundingBoxConfigure.h
)
