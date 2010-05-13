
#-----------------------------------------------------------------------------
# PythonQT
#

set(proj PythonQt)

find_program(Slicer3_PATCH_EXECUTABLE patch
       "C:/Program Files/GnuWin32/bin"
       "C:/Program Files (x86)/GnuWin32/bin")
mark_as_advanced(Slicer3_PATCH_EXECUTABLE)
if (NOT Slicer3_PATCH_EXECUTABLE)
  message(FATAL_ERROR "Building PythonQt requires a patch program.  Please set Slicer3_PATCH_EXECUTABLE!")
endif()

set(pythonqt_src_dir "${CMAKE_CURRENT_BINARY_DIR}/${proj}")
set(pythonqt_patches_dir "${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/Patches/PythonQt")
set(pythonqt_patch_script "${CMAKE_CURRENT_BINARY_DIR}/Slicer3-build/CMake/Slicer3PatchPythonQt.cmake")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../CMake/Slicer3PatchPythonQt.cmake.in ${pythonqt_patch_script} @ONLY)

ExternalProject_Add(${proj}
  DEPENDS ${PythonQt_DEPENDENCIES}
  SVN_REPOSITORY "https://pythonqt.svn.sourceforge.net/svnroot/pythonqt/trunk"
  SOURCE_DIR PythonQt
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${pythonqt_patch_script}
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    ${vtk_PYTHON_ARGS}
  INSTALL_COMMAND "")
    
