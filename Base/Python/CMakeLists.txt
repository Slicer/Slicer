#
# Based on ParaView/VTK/Utilities/vtkTclTest2Py/CMakeLists.txt and
#          ParaView/VTK/Wrapping/Python/CMakeLists.txt
#

SET(Slicer_PYTHON_FILES
  slicer/__init__
  slicer/logic
  slicer/mrml
  slicer/teem
  slicer/freesurfer
  slicer/remoteio
  slicer/slicerqt
  slicer/vtk/__init__
  )

SET(copied_python_files)

SET(Slicer_PYTHON_MODULE_BINARY_DIR "${Slicer3_BINARY_DIR}/bin/Python")

FOREACH(file ${Slicer_PYTHON_FILES})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}.py")
  SET(tgt "${Slicer_PYTHON_MODULE_BINARY_DIR}/${file}.py")
  SET(copied_python_files ${copied_python_files} ${tgt})
  ADD_CUSTOM_COMMAND(DEPENDS ${src}
                      COMMAND ${CMAKE_COMMAND} -E copy ${src} ${tgt}
                      OUTPUT ${tgt}
                      COMMENT "source copy")
ENDFOREACH(file)

ADD_CUSTOM_TARGET(CopyPythonFiles
                  ALL
                  DEPENDS ${copied_python_files})



# Byte compile the Python files.
SET(compile_all_script "${CMAKE_CURRENT_BINARY_DIR}/compile_all_slicer.py")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/compile_all_slicer.py.in"
               ${compile_all_script}
               @ONLY IMMEDIATE)

ADD_CUSTOM_COMMAND(
  COMMAND ${PYTHON_EXECUTABLE}
  ARGS  "${compile_all_script}"
  DEPENDS ${copied_python_files}  ${compile_all_script}
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/slicer_python_compile_complete"
  )

ADD_CUSTOM_TARGET(slicer_pyc ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/slicer_python_compile_complete")


# Install the slicer python module directory
INSTALL(DIRECTORY "${Slicer3_BINARY_DIR}/bin/Python"
  DESTINATION "${Slicer3_INSTALL_BIN_DIR}" COMPONENT Runtime
  USE_SOURCE_PERMISSIONS)

