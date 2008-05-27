project(GenerateLM)

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(LoadableModule REQUIRED)
if(LoadableModule_FOUND)
  include(${LoadableModule_USE_FILE})
endif(LoadableModule_FOUND)

find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE})
endif(ITK_FOUND)

find_package(VTK REQUIRED)
if(VTK_FOUND)
  include(${VTK_USE_FILE})
endif(VTK_FOUND)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
set(GENERATELM_SOURCE GenerateLM.cxx)
generateclp(GENERATELM_SOURCE GenerateLM.xml)
add_executable(GenerateLM ${GENERATELM_SOURCE})
get_target_property(GenerateLM_EXE_PATH GenerateLM LOCATION)
target_link_libraries(GenerateLM
  LoadableModule
  itksys
  ITKEXPAT)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

include(GenerateGenerateLMConfig.cmake)

install(TARGETS GenerateLM RUNTIME 
  DESTINATION bin COMPONENT Development
  )
install(FILES ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME GenerateLMConfig.cmake
  )
install(FILES ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME UseGenerateLM.cmake
  )

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
# This is required so that GenerateLM can be run from an installed tree,
# where the binary is actually not in the same directory as the shared
# libraries it depends on.
#
set(GenerateLM_FORWARD_DIR_BUILD "${EXECUTABLE_OUTPUT_PATH}")
set(GenerateLM_FORWARD_DIR_INSTALL ".")
if(WIN32)
  set(GenerateLM_FORWARD_PATH_BUILD 
    "\"${GenerateLM_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,\"${VTK_DIR}/bin\" CONFIG_DIR_POST,\"${ITK_DIR}/bin\" CONFIG_DIR_POST")
  set(GenerateLM_FORWARD_PATH_INSTALL "")
else(WIN32)
  set(GenerateLM_FORWARD_PATH_BUILD 
    "\"${GenerateLM_FORWARD_DIR_BUILD}\",\"${VTK_DIR}/bin\",\"${ITK_DIR}/bin\"")
  set(GenerateLM_FORWARD_PATH_INSTALL
    "\"../lib/GenerateLM\",\"../lib/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}\",\"../lib/InsightToolkit\",\"../lib/LoadableModule\"")
endif(WIN32)
set(GenerateLM_FORWARD_EXE GenerateLM)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/GenerateLMLauncher.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/GenerateLMLauncher.c
  @ONLY IMMEDIATE)
add_executable(GenerateLMLauncher WIN32
  ${CMAKE_CURRENT_BINARY_DIR}/GenerateLMLauncher.c)
add_dependencies(GenerateLMLauncher GenerateLM)

install(TARGETS GenerateLMLauncher RUNTIME 
  DESTINATION bin COMPONENT Development
  )

