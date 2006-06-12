Project(ModuleDescriptionParser)

ADD_LIBRARY(ModuleDescriptionParser ModuleDescriptionParser.cxx)

TARGET_LINK_LIBRARIES(ModuleDescriptionParser ITKEXPAT)

SUBDIRS(Testing)
