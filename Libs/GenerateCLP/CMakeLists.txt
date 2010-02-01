project(GenerateCLP)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


 
find_package(ModuleDescriptionParser REQUIRED)
if(ModuleDescriptionParser_FOUND)
  include(${ModuleDescriptionParser_USE_FILE})
endif(ModuleDescriptionParser_FOUND)

find_package(TCLAP REQUIRED)
if(TCLAP_FOUND)
  include(${TCLAP_USE_FILE})
endif(TCLAP_FOUND)

find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE})
endif(ITK_FOUND)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
set(GENERATECLP_SOURCE GenerateCLP.cxx)
add_executable(GenerateCLP ${GENERATECLP_SOURCE})
get_target_property(GenerateCLP_EXE_PATH GenerateCLP LOCATION)

target_link_libraries(GenerateCLP
  ModuleDescriptionParser
  itksys
  ITKEXPAT)

include(GenerateGenerateCLPConfig.cmake)

install(TARGETS GenerateCLP RUNTIME 
  DESTINATION bin COMPONENT Development
  )
install(FILES ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install
  DESTINATION lib/GenerateCLP
  COMPONENT Development
  RENAME GenerateCLPConfig.cmake
  )
install(FILES ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
  DESTINATION lib/GenerateCLP
  COMPONENT Development
  RENAME UseGenerateCLP.cmake
  )

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
# This is required so that GenerateLM can be run from an installed tree,
# where the binary is actually not in the same directory as the shared
# libraries it depends on.
#
set(GenerateCLP_FORWARD_DIR_BUILD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(GenerateCLP_FORWARD_DIR_INSTALL ".")
if(WIN32)
  set(GenerateCLP_FORWARD_PATH_BUILD 
    "\"${GenerateCLP_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,\"${ITK_DIR}/bin\" CONFIG_DIR_POST")
  set(GenerateCLP_FORWARD_PATH_INSTALL "\"\"")
else(WIN32)
  set(GenerateCLP_FORWARD_PATH_BUILD 
    "\"${GenerateCLP_FORWARD_DIR_BUILD}\",\"${ITK_DIR}/bin\"")
  set(GenerateCLP_FORWARD_PATH_INSTALL
    "\"${GenerateCLP_DIR}\",\"${ITK_DIR}\",\"${ModuleDescriptionParser_DIR}\"")
endif(WIN32)
set(GenerateCLP_FORWARD_EXE GenerateCLP)

if(NOT WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/GenerateCLPLauncher.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c
    @ONLY IMMEDIATE)
  add_executable(GenerateCLPLauncher
    ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLPLauncher.c)
  add_dependencies(GenerateCLPLauncher GenerateCLP)

  install(TARGETS GenerateCLPLauncher RUNTIME 
    DESTINATION bin COMPONENT Development
    )
endif(NOT WIN32)

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
