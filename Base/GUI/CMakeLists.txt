PROJECT(SlicerBaseGUI)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it.

FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})

# The name of our targets (executable or libraries) will simply be based
# on the project name, with an extra prefix and suffix.

SET(TARGET_BASE_NAME "Slicer3${PROJECT_NAME}Test")

# We actually define a class in this example, and we want to be able to
# use its callbacks from our user interface. To do so, we need to create
# a library and wrap it automatically for the Tcl language.

SET(LIB_NAME "SlicerBaseGUI")
SET(LIB_SRCS 
  vtkSlicerStyle.cxx
  vtkSlicerColor.cxx
  vtkSlicerFont.cxx
  vtkSlicerApplication.cxx
  vtkSlicerComponentGUI.cxx
  vtkSlicerModuleGUI.cxx
  vtkSlicerGUICollection.cxx
  vtkSlicerSliceGUI.cxx
  vtkSlicerSliceGUICollection.cxx
  vtkSlicerSliceViewer.cxx
  vtkSlicerSliceControllerWidget.cxx
  vtkSlicerSlicesGUI.cxx
  vtkSlicerVolumesGUI.cxx
  vtkSlicerModelsGUI.cxx
  vtkSlicerDataGUI.cxx
  vtkSlicerGUILayout.cxx    
  vtkSlicerApplicationGUI.cxx
  vtkSlicerVolumeSelectGUI.cxx
  vtkSlicerNodeSelectorWidget.cxx
  vtkSlicerSliceControlGUI.cxx
  # Widgets
  vtkKWMatrix4x4.cxx
  vtkKWScriptedWidget.cxx
  vtkKWWindowLevelThresholdEditor.cxx
  )

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
 # none yet
 # vtkSlicerGUIUpdate.cxx
 # vtkSlicerLogicUpdate.cxx
 # vtkSlicerMRMLUpdate.cxx
 # WRAP_EXCLUDE
 # )

INCLUDE_DIRECTORIES(
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTKITK_SOURCE_DIR}/Libs/vtkITK
  ${VTK_INCLUDE_DIR}
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library.

ADD_LIBRARY(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${KWWidgets_LIBRARIES} SlicerBaseLogic )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} ${LIB_NAME})
ENDIF(BUILD_SHARED_LIBS)

# The name of our executable and the corresponding source file.

SET(EXE_NAME "${TARGET_BASE_NAME}")
SET(EXE_SRCS "${EXE_NAME}.cxx")

# On Win32 platforms, let's configure the KWWidgets sample resource file
# to get a nice application icon and some additional information.

IF(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  INCLUDE_DIRECTORIES(${VTK_TK_RESOURCES_DIR})
  SET(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  SET(KWWidgets_RESOURCE_APPLICATION_NAME "${EXE_NAME}")
  SET(KWWidgets_RESOURCE_FILE_NAME "${EXE_NAME}")
  CONFIGURE_FILE(${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.in ${RES_FILE})
ENDIF(WIN32 AND NOT BORLAND AND NOT CYGWIN)

# Create the executable, and link it against the KWWidgets libraries and our
# own library.

#### TODO: move the test application to Testing
#### ADD_EXECUTABLE(${EXE_NAME} WIN32 ${EXE_SRCS} ${RES_FILE})
#### TARGET_LINK_LIBRARIES(${EXE_NAME} SlicerBaseLogic SlicerBaseGUI MRML ${KWWidgets_LIBRARIES} ${LIB_NAME})
#### IF(BUILD_TESTING)
####   ADD_TEST(Test${EXE_NAME} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME} --test)
#### ENDIF(BUILD_TESTING)


# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

IF(NOT KWWidgets_SOURCE_DIR AND VTK_TCL_TK_COPY_SUPPORT_LIBRARY)
  IF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
    INCLUDE(${VTK_TCL_TK_MACROS_MODULE})
    VTK_COPY_TCL_TK_SUPPORT_FILES_TO_DIR(
      ${VTK_TCL_SUPPORT_LIBRARY_PATH} ${VTK_TK_SUPPORT_LIBRARY_PATH}
      "${PROJECT_BINARY_DIR}/lib")
  ENDIF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
ENDIF(NOT KWWidgets_SOURCE_DIR AND VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

# Install the example target. If you are using this CMakeLists.txt file
# to create your own application based on KWWidgets, you can most likely 
# omit this section or change the installation directory, unless you want
# to install your application right where KWWidgets is already installed.

#INSTALL_TARGETS(${KWWidgets_INSTALL_BIN_DIR} ${EXE_NAME})

# Register this example as a test. Our executable supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you can most likely omit this section too, unless
# your application supports that feature too.

INCLUDE_DIRECTORIES(${SlicerBaseGUI_BINARY_DIR})
CONFIGURE_FILE(
  ${SlicerBaseGUI_SOURCE_DIR}/vtkSlicerBaseGUIConfigure.h.in 
  ${SlicerBaseGUI_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
)

