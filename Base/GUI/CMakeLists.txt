project(SlicerBaseGUI)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()


# --------------------------------------------------------------------------
# Install support files

file(GLOB TCLFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Tcl/*.tcl")
foreach(tclfile ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${tclfile}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${tclfile}
    COPYONLY)
endforeach()

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Tcl
  )

file(GLOB IMAGEFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/ImageData/*.png")
foreach(file ${IMAGEFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${file}
    COPYONLY
    )
endforeach()

install(
  FILES ${IMAGEFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Tcl/ImageData
  )
