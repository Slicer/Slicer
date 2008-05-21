project(GenerateLM)

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(LoadableModule REQUIRED)
if(LoadableModule_FOUND)
  include(${LoadableModule_USE_FILE})
endif(LoadableModule_FOUND)

find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE})
endif(ITK_FOUND)

find_package(VTK REQUIRED)
if(VTK_FOUND)
  include(${VTK_USE_FILE})
endif(VTK_FOUND)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
set(GENERATELM_SOURCE GenerateLM.cxx)
generateclp(GENERATELM_SOURCE GenerateLM.xml)
add_executable(GenerateLM ${GENERATELM_SOURCE})
get_target_property(GenerateLM_EXE_PATH GenerateLM LOCATION)
target_link_libraries(GenerateLM
  LoadableModule
  itksys
  ITKEXPAT)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

include(GenerateGenerateLMConfig.cmake)

install(TARGETS GenerateLM RUNTIME 
  DESTINATION bin COMPONENT Development
  )
install(FILES ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME GenerateLMConfig.cmake
  )
install(FILES ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake_install
  DESTINATION lib/GenerateLM
  COMPONENT Development
  RENAME UseGenerateLM.cmake
  )
