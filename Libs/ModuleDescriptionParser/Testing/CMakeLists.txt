Project(ModuleDescriptionParserTesting)

SET ( PARSER1TEST_SOURCE Parser1Test )
ADD_EXECUTABLE ( Parser1Test ${PARSER1TEST_SOURCE})
TARGET_LINK_LIBRARIES ( Parser1Test ModuleDescriptionParser )

SET ( ModuleFactoryTest_SOURCE ModuleFactoryTest )
ADD_EXECUTABLE ( ModuleFactoryTest ${ModuleFactoryTest_SOURCE})
TARGET_LINK_LIBRARIES ( ModuleFactoryTest ModuleDescriptionParser )

INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_SOURCE_DIR}
  )

SET (TEST_DATA ${ModuleDescriptionParserTesting_SOURCE_DIR}/TestData)
ADD_TEST(Parser1Test1 ${EXECUTABLE_OUTPUT_PATH}/Parser1Test ${TEST_DATA}/ParserTest1.xml)

ADD_TEST(Parser1Test2 ${EXECUTABLE_OUTPUT_PATH}/Parser1Test ${TEST_DATA}/ParserTest2.xml)

ADD_TEST(ModuleFactoryTest ${EXECUTABLE_OUTPUT_PATH}/ModuleFactoryTest ${Slicer3_BINARY_DIR}/${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/)

