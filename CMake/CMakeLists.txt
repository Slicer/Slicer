FILE(GLOB cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
INSTALL(
  FILES ${cmake_files}
  DESTINATION ${Slicer_INSTALL_LIB_DIR}/CMake COMPONENT Development
  )
