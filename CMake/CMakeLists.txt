file(GLOB cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
install(
  FILES ${cmake_files}
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}/CMake COMPONENT Development
  )
