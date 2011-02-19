project(DetectingTumorGrowth)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

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
