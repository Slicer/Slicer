#
# automatically detect if we build for Slicer3 or Slicer4
#

#
# as a result the CMake variable EM_Slicer4_FOUND and EM_Slicer3_FOUND will be set accordingly
# also, a message stating for which application we configure is printed
#

if(Slicer3_SOURCE_DIR)
  # this is Slicer3  
  SET(EM_Slicer3_FOUND ON)
  SET(EM_Slicer4_FOUND OFF)
  MESSAGE(STATUS "Configuring EMSegment for Slicer3..")
  
else(Slicer3_SOURCE_DIR)

  # this is Slicer4
  SET(EM_Slicer4_FOUND ON)
  SET(EM_Slicer3_FOUND OFF)
  MESSAGE(STATUS "Configuring EMSegment for Slicer4..")
  
endif(Slicer3_SOURCE_DIR)

  # we do not want CMake project definitions so we set the following variables manually
  #project(EMSegment)
  SET(PROJECT_NAME "EMSegment")
  SET(EMSegment_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  SET(EMSegment_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
