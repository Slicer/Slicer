
project(qtstyleplugins)

# Sources
set(KIT_SRCS
  src/qhexstring_p.h
  src/qstylecache_p.h
  src/qstylehelper.cpp
  src/qstylehelper_p.h
  src/qcleanlooksstyle.cpp
  src/qcleanlooksstyle.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  src/qcleanlooksstyle.h
  )

QT5_WRAP_CPP(KIT_MOC_CXX ${KIT_MOC_SRCS})

source_group("Generated" FILES
  ${KIT_MOC_CXX}
  )

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------

# NA

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/*")
  install(
    FILES ${headers}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
set(targets_to_export ${lib_name})

add_library(${lib_name} STATIC
  ${KIT_SRCS}
  ${KIT_MOC_CXX}
  )

set(KIT_LIBRARIES
  Qt5::Widgets
  )

target_link_libraries(${lib_name} ${KIT_LIBRARIES})

target_include_directories(${lib_name}
  PUBLIC include
  PRIVATE src
  )

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${lib_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------

# NA

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${targets_to_export} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------

# NA

# --------------------------------------------------------------------------
# Qt Designer Plugins
# --------------------------------------------------------------------------

# NA

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------

# NA

