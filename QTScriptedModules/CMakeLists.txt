add_subdirectory(Scripts)
add_subdirectory(EditorLib)
add_subdirectory(DICOMLib)
#add_subdirectory(ChangeTrackerPy)

if(BUILD_TESTING AND PYTHON_EXECUTABLE)
  add_subdirectory(Testing)
endif()

# NOTE: ChangeTrackerPy source code is checkout using "External_ChangeTrackerPy.cmake".
#add_subdirectory(
#  ${ChangeTrackerPy_SOURCE_DIR}
#  ${CMAKE_CURRENT_BINARY_DIR}/ChangeTrackerPy
#  )
