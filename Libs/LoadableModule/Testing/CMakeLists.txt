Project(LoadableModuleDescriptionParserTesting)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET ( PARSER1TEST_SOURCE LMParser1Test )
ADD_EXECUTABLE ( LMParser1Test ${PARSER1TEST_SOURCE})
TARGET_LINK_LIBRARIES ( LMParser1Test LoadableModule )

SET ( LoadableModuleFactoryTest_SOURCE LoadableModuleFactoryTest )
ADD_EXECUTABLE ( LoadableModuleFactoryTest ${LoadableModuleFactoryTest_SOURCE})
TARGET_LINK_LIBRARIES ( LoadableModuleFactoryTest LoadableModule )

INCLUDE_DIRECTORIES(
  ${LoadableModuleDescriptionParser_SOURCE_DIR}
  )

SET (TEST_DATA ${LoadableModuleDescriptionParserTesting_SOURCE_DIR}/TestData)
ADD_TEST(LMParser1Test1 ${EXECUTABLE_OUTPUT_PATH}/LMParser1Test ${TEST_DATA}/ParserTest1.xml)

ADD_TEST(LMParser1Test2 ${EXECUTABLE_OUTPUT_PATH}/LMParser1Test ${TEST_DATA}/ParserTest2.xml)

# TODO: hangs on windows
# ADD_TEST(LoadableModuleFactoryTest ${EXECUTABLE_OUTPUT_PATH}/LoadableModuleFactoryTest ${Slicer3_BINARY_DIR}/${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/)

