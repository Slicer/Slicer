project( HelloWorld )

cmake_minimum_required(VERSION 2.6)

find_package(Slicer3 REQUIRED)
include(${Slicer3_USE_FILE})

link_directories(${ModuleDescriptionParser_BINARY_DIR})

slicer3_set_default_plugins_output_path()

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )
link_directories(${SlicerBaseCLI_BINARY_DIR})


set (CLP HelloWorld)
set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml)

add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKCommon)

enable_testing()
include(CTest)
set(UPDATE_TYPE svn)

add_test (HelloWorldTest1 ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${CLP} --help)

