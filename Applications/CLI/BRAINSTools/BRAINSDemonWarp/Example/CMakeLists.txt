project(SimpleDiffeomorphicRegistration)

if (WIN32)
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
endif (WIN32)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------


if(0)
if(BUILD_SHARED_LIBS)
## The dynamicly loadable slicer module can be built
##only when ITK shared libraries are used.
#
# CREATE A loadable module for Slicer
add_library(BRAINSDemonWarpSlicerModule SHARED ${BRAINSDemonWarp_SOURCE})
set_target_properties(BRAINSDemonWarpSlicerModule PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(BRAINSDemonWarpSlicerModule ${ITK_LIBRARIES} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
endif(BUILD_SHARED_LIBS)

add_library(BRAINSDemonWarpLIB itkSimpleDiffeomorphicRegistration.cxx)
target_link_libraries(BRAINSDemonWarpLIB ${ITK_LIBRARIES} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
install(TARGETS BRAINSDemonWarpLIB
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

include_directories(${BRAINSDemonWarp_SOURCE_DIR} ${BRAINSDemonWarp_SOURCE_DIR}/Common)
set(SimpleDiffeomorphicRegistration_SOURCE lib_test.cxx)
add_executable( SimpleDiffeomorphicRegistrationCLP ${SimpleDiffeomorphicRegistration_SOURCE} )
target_link_libraries(SimpleDiffeomorphicRegistrationCLP BRAINSDemonWarpLIB)

add_test(
   NAME ValidateLIB 
   COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SimpleDiffeomorphicRegistrationCLP>
     ${BRAINSDemonWarp_SOURCE_DIR}/TestingBRAINSDemonWarp/ANON0006_20_T1_dbg_splayed.nii.gz
     ${BRAINSDemonWarp_SOURCE_DIR}/TestingBRAINSDemonWarp/ANON0006_20_T1_sag_twisted.nii.gz
     ${BRAINSDemonWarp_BINARY_DIR}/orientedImagesDemons_lib_test.nii.gz
   )


endif()
