project(SlicerLibs)

# Enable this option to avoid unnecessary re-compilation associated with command line module
set(GENERATECLP_USE_MD5 ON)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------

set(dirs )

list(APPEND dirs
  ITKFactoryRegistration
  )
set(SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES
  ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES} ITKFactoryRegistration
  CACHE INTERNAL "SlicerExecutionModel extra executable target libraries" FORCE
  )

if(Slicer_BUILD_vtkAddon)
  list(APPEND dirs
    vtkAddon
    )
endif()

list(APPEND dirs
  vtkTeem
  vtkITK
  vtkSegmentationCore
  MRML/Core
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND dirs MRML/CLI)
endif()
list(APPEND dirs
  RemoteIO
  MRML/Logic
  MRML/DisplayableManager
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND dirs MRML/IDImageIO)
endif()
list(APPEND dirs
  MRML/Widgets
  )

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

if(Slicer_BUILD_CLI_SUPPORT)
  # Python support within ModuleDescriptionParser is used only within ModuleFactory.cxx
  # Considering the ModuleFactory isn't used anymore to discover modules, let's disable the
  # associated python support so that the python library aren't a dependency of
  # either ModuleDescriptionParser or program/library depending on ModuleDescriptionParser.
  set(ModuleDescriptionParser_USE_PYTHON OFF)
endif()

# Qt designer plugins
set(MRML_BUILD_QT_DESIGNER_PLUGINS ${Slicer_BUILD_QT_DESIGNER_PLUGINS})

# ITKFactories directories
set(MRMLIDImageIO_ITKFACTORIES_DIR ${Slicer_ITKFACTORIES_DIR})
set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})

# Name of the environment variable that contains the application home directory.
set(MRML_APPLICATION_HOME_DIR_ENV "SLICER_HOME")

# Name of the environment variable that contains the requested OpenGL profile.
# Accepted values are "default" (same as not specifying a value) or
# to force a specific profile: "no", "core", or  "compatibility".
set(MRML_APPLICATION_OPENGL_PROFILE_ENV "SLICER_OPENGL_PROFILE")

# Name of the subdirectory within the application directory where shared files are stored.
set(MRML_APPLICATION_SHARE_SUBDIR ${Slicer_SHARE_DIR})

# vtkITK contains tests that uses MRML's test data.
set(MRML_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MRML/Core/Testing/TestData)

set(VTKITK_BUILD_DICOM_SUPPORT ${Slicer_BUILD_DICOM_SUPPORT})

set(MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT ${Slicer_BUILD_WEBENGINE_SUPPORT})

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  string(REPLACE "/" "" dir_noslash ${dir})
  if(dir_noslash MATCHES "Widgets$")
    set(dir_noslash "q${dir_noslash}")
  endif()
  set(${dir_noslash}_INSTALL_BIN_DIR ${Slicer_INSTALL_LIB_DIR})
  set(${dir_noslash}_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})
  set(${dir_noslash}_INSTALL_SHARE_DIR ${Slicer_INSTALL_ROOT}share/${dir_noslash})
  set(${dir_noslash}_INSTALL_NO_DEVELOPMENT ${Slicer_INSTALL_NO_DEVELOPMENT})
  set(${dir_noslash}_EXPORT_FILE ${Slicer_BINARY_DIR}/SlicerTargets.cmake)
  set(${dir_noslash}_FOLDER Core-Libs/${dir_noslash})

  # Check if library sources were specified as a remote module
  if(DEFINED ${dir}_SOURCE_DIR AND Slicer_BUILD_${dir})
    message(STATUS "Configuring library: ${dir} [${dir}_SOURCE_DIR: ${${dir}_SOURCE_DIR}]")
    add_subdirectory(
      ${${dir}_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/${dir}
      )
  else()
    message(STATUS "Configuring library: ${dir}")
    add_subdirectory(${dir})
  endif()

endforeach()

#-----------------------------------------------------------------------------
# Generate list of include directories
#-----------------------------------------------------------------------------
set(include_dirs)
foreach(dir ${dirs})
  string(REPLACE "/" "" dir_noslash ${dir})
  if(dir_noslash MATCHES "Widgets$")
    set(dir_noslash "q${dir_noslash}")
  endif()
  list(APPEND include_dirs ${${dir_noslash}_INCLUDE_DIRS})
endforeach()

# Set variable Slicer_Libs_INCLUDE_DIRS
set(Slicer_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "Slicer Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Set MRML_LIBRARIES
#-----------------------------------------------------------------------------
# Anticipating the move of MRML related libraries to a standalone project,
# let's group these libraries.
# Note also that since MRMLIDImageIO is an ITK plugin it is NOT added to that list.
set(_mrml_libraries MRMLCore MRMLLogic MRMLDisplayableManager)
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND _mrml_libraries MRMLCLI)
endif()
set(MRML_LIBRARIES ${_mrml_libraries} CACHE INTERNAL "MRML libraries" FORCE)

#-----------------------------------------------------------------------------
# Set variable Slicer_Libs_VTK_WRAPPED_LIBRARIES
#-----------------------------------------------------------------------------
set(_vtk_wrapped_libraries
  vtkSegmentationCore
  vtkTeem
  vtkITK
  MRMLCore
  MRMLLogic
  )

if(Slicer_BUILD_vtkAddon)
  list(APPEND _vtk_wrapped_libraries
    vtkAddon
    )
endif()
if(Slicer_USE_PYTHONQT AND VTK_WRAP_PYTHON)
  list(APPEND _vtk_wrapped_libraries MRMLDisplayableManager)
endif()
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND _vtk_wrapped_libraries MRMLCLI)
endif()
set(Slicer_Libs_VTK_WRAPPED_LIBRARIES
  ${_vtk_wrapped_libraries} CACHE INTERNAL "Slicer Libs VTK wrapped" FORCE)
