Project(ModuleDescriptionParser)

SET(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
)

INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_SOURCE_DIR} 
  ${ModuleDescriptionParser_BINARY_DIR}
)

ADD_LIBRARY(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})

TARGET_LINK_LIBRARIES(ModuleDescriptionParser ITKEXPAT)

CONFIGURE_FILE(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfigure.h.in 
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfigure.h
)

SUBDIRS(Testing)
