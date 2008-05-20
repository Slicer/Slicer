project ( RegisterImagesModule )

# Disable MSVC 8 warnings
if(WIN32)
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif(DISABLE_MSVC8_DEPRECATED_WARNINGS)
endif(WIN32)

set ( RegisterImages_SOURCE 
  RegisterImages.cxx
  )

generateclp(RegisterImages_SOURCE RegisterImages.xml)

add_executable(RegisterImages ${RegisterImages_SOURCE})
target_link_libraries(RegisterImages ITKIO ITKCommon ITKStatistics ITKNumerics)

add_library(RegisterImagesModule SHARED ${RegisterImages_SOURCE})
set_target_properties (RegisterImagesModule PROPERTIES
  COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(RegisterImagesModule ITKIO ITKCommon ITKStatistics
  ITKNumerics)

if (Slicer3_SOURCE_DIR)

  set(TARGET_MODULES RegisterImagesModule RegisterImages)

  foreach(target ${TARGET_MODULES})
    install(TARGETS ${target}
      RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
      )
  endforeach(target)

endif (Slicer3_SOURCE_DIR)
