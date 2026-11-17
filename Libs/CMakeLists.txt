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

if(Slicer_USE_IDImageIO)
  list(APPEND dirs
    MRML/IDImageIO
    )
endif()
list(APPEND dirs
  MRML/Widgets
  )

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

# Python support within ModuleDescriptionParser is used only within ModuleFactory.cxx
# Considering the ModuleFactory isn't used anymore to discover modules, let's disable the
# associated python support so that the python library aren't a dependency of
# either ModuleDescriptionParser or program/library depending on ModuleDescriptionParser.
set(ModuleDescriptionParser_USE_PYTHON OFF)

# Qt designer plugins
set(MRML_BUILD_QT_DESIGNER_PLUGINS ${Slicer_BUILD_QT_DESIGNER_PLUGINS})

# ITKFactories directories
if(Slicer_USE_IDImageIO)
  set(MRMLIDImageIO_ITKFACTORIES_DIR ${Slicer_ITKFACTORIES_DIR})
  set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
endif()

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
set(_mrml_libraries Slicer::MRMLCore Slicer::MRMLLogic Slicer::MRMLDisplayableManager Slicer::MRMLCLI)
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
  MRMLCLI
  vtkAddon
)

if(Slicer_USE_PYTHONQT AND VTK_WRAP_PYTHON)
  list(APPEND _vtk_wrapped_libraries MRMLDisplayableManager)
endif()

set(Slicer_Libs_VTK_WRAPPED_LIBRARIES
  ${_vtk_wrapped_libraries} CACHE INTERNAL "Slicer Libs VTK wrapped" FORCE)
