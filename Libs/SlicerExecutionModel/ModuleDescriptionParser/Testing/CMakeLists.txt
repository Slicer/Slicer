project(ModuleDescriptionParserTesting)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

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
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:Parser1Test> ${TEST_DATA}/ParserTest1.xml
  )

add_test(
  NAME Parser1Test2 
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:Parser1Test> ${TEST_DATA}/ParserTest2.xml)

#add_test(
#  NAME ModuleFactoryTest 
#  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:ModuleFactoryTest> 
#    ${Slicer_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/
#  )

