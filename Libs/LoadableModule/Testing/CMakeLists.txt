project(LoadableModuleDescriptionParserTesting)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(PARSER1TEST_SOURCE LMParser1Test )
add_executable(LMParser1Test ${PARSER1TEST_SOURCE})
target_link_libraries(LMParser1Test LoadableModule )

set(LoadableModuleFactoryTest_SOURCE LoadableModuleFactoryTest )
add_executable(LoadableModuleFactoryTest ${LoadableModuleFactoryTest_SOURCE})
target_link_libraries(LoadableModuleFactoryTest LoadableModule )

include_directories(
  ${LoadableModuleDescriptionParser_SOURCE_DIR}
  )

set(TEST_DATA ${LoadableModuleDescriptionParserTesting_SOURCE_DIR}/TestData)
add_test(LMParser1Test1 ${EXECUTABLE_OUTPUT_PATH}/LMParser1Test ${TEST_DATA}/ParserTest1.xml)

add_test(LMParser1Test2 ${EXECUTABLE_OUTPUT_PATH}/LMParser1Test ${TEST_DATA}/ParserTest2.xml)

# commenting out until it runs faster via getbuildtest.tcl
#ADD_TEST(LoadableModuleFactoryTest ${EXECUTABLE_OUTPUT_PATH}/LoadableModuleFactoryTest ${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/)

