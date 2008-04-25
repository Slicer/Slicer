project(GUITesting)
#####################

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(vtkITK REQUIRED)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(CLP EventBrokerTest)

set(${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries(${CLP}
  ${Slicer3_LIBRARIES}
  )

include_directories(
  ${Slicer3_INCLUDE_DIRS}
  )

set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")
add_test(EventBrokerTest 
  ${Slicer3_BINARY_DIR}/Slicer3 --launch EventBrokerTest --logFileName ${TEMP}/EventLog.dot --graphFile ${TEMP}/EventBroker.dot)
