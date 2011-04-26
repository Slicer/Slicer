project(ModuleDescriptionParserTesting)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# --------------------------------------------------------------------------
# Build Parser1Test executable
# --------------------------------------------------------------------------
set(PARSER1TEST_SOURCE Parser1Test.cxx )
add_executable(Parser1Test ${PARSER1TEST_SOURCE})
target_link_libraries(Parser1Test ModuleDescriptionParser )

# --------------------------------------------------------------------------
# Build ModuleFactoryTest executable
# --------------------------------------------------------------------------
#set(ModuleFactoryTest_SOURCE ModuleFactoryTest.cxx)
#add_executable(ModuleFactoryTest ${ModuleFactoryTest_SOURCE})
#target_link_libraries(ModuleFactoryTest ModuleDescriptionParser )

#include_directories(
#  ${ModuleDescriptionParser_SOURCE_DIR}
#  )

# --------------------------------------------------------------------------
# Add tests
# --------------------------------------------------------------------------
set(TEST_DATA ${ModuleDescriptionParserTesting_SOURCE_DIR}/TestData)
add_test(
  NAME Parser1Test1 
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:Parser1Test> ${TEST_DATA}/ParserTest1.xml
  )

add_test(
  NAME Parser1Test2 
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:Parser1Test> ${TEST_DATA}/ParserTest2.xml)

#add_test(
#  NAME ModuleFactoryTest 
#  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:ModuleFactoryTest> 
#    ${Slicer_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/
#  )

