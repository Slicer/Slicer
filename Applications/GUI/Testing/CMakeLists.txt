project(GUITesting)
#####################

# --------------------------------------------------------------------------
# Command line parsing tests
#
add_test(Slicer3CLTest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --help)
add_test(Slicer3CLTest2 ${Slicer3_BINARY_DIR}/Slicer3 --xml --test-mode)
add_test(Slicer3CLTest3 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode -f ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(Slicer3CLTest4 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --eval "puts testing ,. exit 0")
add_test(Slicer3CLTest5 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --exec "puts testing ,. exit 0")
add_test(Slicer3CLTest6 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(Slicer3CLTest7 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
add_test(Slicer3ScrollTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

# --------------------------------------------------------------------------
# MRML Tests
#
add_test(Slicer3MRMLUndo ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer3_HOME}/share/MRML/Testing/testUndo.tcl)
add_test(Slicer3MRMLVolume ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer3_HOME}/share/MRML/Testing/testVolume.tcl)


# --------------------------------------------------------------------------
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
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  )

include_directories(
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")
add_test(EventBrokerTest 
  ${Slicer3_BINARY_DIR}/Slicer3 --launch EventBrokerTest --logFileName ${TEMP}/EventLog.dot --graphFile ${TEMP}/EventBroker.dot)
