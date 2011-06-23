project(DetectingTumorGrowth)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

add_executable(DetectGrowth DetectGrowth.cxx)
#slicer3_set_plugins_output_path(DetectGrowth)
target_link_libraries(DetectGrowth ${ITK_LIBRARIES})
#slicer3_install_plugins(DetectGrowth)
install(TARGETS DetectGrowth RUNTIME
        DESTINATION bin COMPONENT Development
)

add_executable(applyDeformationITK applyDeformationITK.cxx)
#slicer3_set_plugins_output_path(applyDeformationITK)
target_link_libraries(applyDeformationITK ${ITK_LIBRARIES})
#slicer3_install_plugins(applyDeformationITK)
install(TARGETS applyDeformationITK RUNTIME
        DESTINATION bin COMPONENT Development
)


add_executable(DetectGrowthSegmentation DetectGrowthSegmentation.cxx)
#slicer3_set_plugins_output_path(DetectGrowthSegmentation)
target_link_libraries(DetectGrowthSegmentation ${ITK_LIBRARIES})
install(TARGETS DetectGrowthSegmentation RUNTIME
        DESTINATION bin COMPONENT Development
)

#slicer3_install_plugins(DetectGrowthSegmentation)
