#
#  Check if VTK was configured with QT, 
#  if so, use it, 
#  otherwise, complain. 
#
OPTION(Slicer3_USE_QT  "Use Qt as an extra GUI library" OFF)

MACRO(Slicer3_SETUP_QT)

IF(Slicer3_USE_QT)
  SET(minimum_required_qt_version "4.6")
  IF(VTK_USE_QVTK)

    # Check if QT_QMAKE_EXECUTABLE is set
    IF(NOT QT_QMAKE_EXECUTABLE)
      MESSAGE(FATAL_ERROR "error: Set QT_QMAKE_EXECUTABLE: version of Qt >= ${minimum_required_qt_version} *required* to build Slicer")
    ENDIF()
    
    FIND_PACKAGE(Qt4)
    
    IF(QT4_FOUND)
      # Check version, note that ${QT_VERSION_PATCH} could also be used
      IF(NOT "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}" STREQUAL "${minimum_required_qt_version}")
        MESSAGE(FATAL_ERROR "error: Slicer requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} ...")
      ENDIF()
      # Enable modules
      SET(QT_USE_QTNETWORK ON)
      # Includes Qt headers
      INCLUDE(${QT_USE_FILE})
    ELSE(QT4_FOUND)
      MESSAGE(FATAL_ERROR "error: Qt >= ${minimum_required_qt_version} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable.")
    ENDIF(QT4_FOUND)
    
  ELSE(VTK_USE_QVTK)
     MESSAGE("error: VTK was not configured to use QT, you probably need to recompile it with VTK_USE_GUISUPPORT ON, VTK_USE_QVTK ON, DESIRED_QT_VERSION 4 and QT_QMAKE_EXECUTABLE set appropriatly. Note that Qt >= ${minimum_required_qt_version} is *required*")
  ENDIF(VTK_USE_QVTK)
ENDIF(Slicer3_USE_QT)

ENDMACRO(Slicer3_SETUP_QT)


