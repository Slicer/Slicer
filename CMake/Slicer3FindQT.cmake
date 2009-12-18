#
#  Check if VTK was configured with QT, 
#  if so, use it, 
#  otherwise, complain. 
#
OPTION(Slicer3_USE_QT  "Use Qt as an extra GUI library" OFF)

MACRO(Slicer3_SETUP_QT)

IF(Slicer3_USE_QT)
  ## QT
  IF(VTK_USE_QVTK)
    FIND_PACKAGE(Qt4 4.6 EXACT)
    IF(QT4_FOUND)
      SET(QT_USE_QTNETWORK ON)
      INCLUDE(${QT_USE_FILE})
      SET(Slicer3_USE_QT ON)
      ADD_DEFINITIONS(-DSlicer3_USE_QT)
    ELSE(QT4_FOUND)
      MESSAGE("Qt4 was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable")
    ENDIF(QT4_FOUND)
  ELSE(VTK_USE_QVTK)
     MESSAGE("VTK was not configured to use QT, no QT-based code will be compiled in Slicer")
     SET(Slicer3_USE_QT OFF)
  ENDIF(VTK_USE_QVTK)
ENDIF(Slicer3_USE_QT)

ENDMACRO(Slicer3_SETUP_QT)


