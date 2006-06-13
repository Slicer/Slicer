Project(ModuleDescriptionParser)

SET(ModuleDescriptionParser_SRCS
  ModuleParameter.cxx
  ModuleParameterGroup.cxx
  ModuleDescription.cxx
  ModuleDescriptionParser.cxx
)

ADD_LIBRARY(ModuleDescriptionParser ${ModuleDescriptionParser_SRCS})

TARGET_LINK_LIBRARIES(ModuleDescriptionParser ITKEXPAT)

SUBDIRS(Testing)
