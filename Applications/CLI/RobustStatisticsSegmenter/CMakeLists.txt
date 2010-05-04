project(RobustStatisticsSegmentation)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

# copied from Peter's code, but I don't know if I need this line
link_directories(${ModuleDescriptionParser_BINARY_DIR})


find_package(ITK REQUIRED)
include(${USE_ITK_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )
#####################

set(CLP SFLSRobustStat3DTest)
set(RSS_SOURCE SFLSRobustStat3DTest.cxx SFLSRobustStat3DTestConsole.cxx labelMapPreprocessor.h SFLS.h SFLSSegmentor3D.h SFLSSegmentor3D.txx SFLSRobustStatSegmentor3DLabelMap_single.h SFLSRobustStatSegmentor3DLabelMap_single.txx)

generateclp(RSS_SOURCE SFLSRobustStat3DTest.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)

# add_executable( ${CLP} ${RSS_SOURCE})
# target_link_libraries( ${CLP} ITKCommon ITKBasicFilters ITKIO itkvnl)



add_library(${CLP}Lib SHARED ${RSS_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(${CLP}Lib ITKCommon ITKBasicFilters ITKIO itkvnl)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

include_directories(
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR}
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR})




if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    ${CLP}
    ${CLP}Lib
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)


# set(TARGETS ${CLP})
# foreach(targ ${TARGETS})
#   slicer3_set_plugins_output_path(${targ})
# endforeach(targ ${TARGETS})
# slicer3_install_plugins(${TARGETS})


# for testing
add_executable( SFLSRobustStat3DTestConsole SFLSRobustStat3DTestConsole.cxx)
target_link_libraries( SFLSRobustStat3DTestConsole ITKCommon ITKBasicFilters ITKIO itkvnl)

enable_testing()
set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )
# message (STATUS "cmake current source dir = '${CMAKE_CURRENT_SOURCE_DIR}'")
# message (STATUS "cmake current bin dir = '${CMAKE_CURRENT_BINARY_DIR}'")
# message (STATUS "slicer3 exe = '${Slicer3_EXE}'")
add_test(RSStest ${Slicer3_EXE} --launch SFLSRobustStat3DTestConsole ${CMAKE_CURRENT_SOURCE_DIR}/grayscale.nrrd ${CMAKE_CURRENT_SOURCE_DIR}/grayscale-label.nrrd ${CMAKE_CURRENT_BINARY_DIR}/rss-test-seg.nrrd 50 0.1 0.2)
