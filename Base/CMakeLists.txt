
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildBaseQtLibrary.cmake)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/QTCore
  ${CMAKE_CURRENT_BINARY_DIR}/QTCore
  )

add_subdirectory(Logic)

if(Slicer_BUILD_GUI)
  add_subdirectory(QTCore)
  add_subdirectory(QTGUI)
  if(Slicer_BUILD_CLI_SUPPORT)
    add_subdirectory(QTCLI)
  endif()
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  add_subdirectory(CLI)
endif()

if(VTK_WRAP_PYTHON)
  add_subdirectory(Python)
endif()

# Since "QTApp" depends on "Modules/Core", the folder is included in the top-level CMakeLists.txt
# after "Modules/Core" folder.
