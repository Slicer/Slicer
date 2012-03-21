
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
endif()

if(DEFINED VTK_SOURCE_DIR AND NOT EXISTS ${VTK_SOURCE_DIR})
  message(FATAL_ERROR "VTK_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(VTK_DEPENDENCIES "")
if (Slicer_USE_PYTHONQT)
  list(APPEND VTK_DEPENDENCIES python)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(VTK)
set(proj VTK)

if(NOT DEFINED VTK_DIR OR NOT DEFINED VTK_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  set(VTK_WRAP_TCL OFF)
  set(VTK_WRAP_PYTHON OFF)

  if (Slicer_USE_PYTHONQT)
    set(VTK_WRAP_PYTHON ON)
  endif()

  set(VTK_PYTHON_ARGS)
  if(Slicer_USE_PYTHONQT)
    set(VTK_PYTHON_ARGS
      -DVTK_INSTALL_PYTHON_USING_CMAKE:BOOL=ON
      -DPYTHON_EXECUTABLE:PATH=${slicer_PYTHON_EXECUTABLE}
      -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
      -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
      )
  endif()

  set(VTK_QT_ARGS)
  if(NOT APPLE)
    set(VTK_QT_ARGS
      #-DDESIRED_QT_VERSION:STRING=4 # Unused
      -DVTK_USE_GUISUPPORT:BOOL=ON
      -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
      -DVTK_USE_QT:BOOL=ON
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      )
  else()
    set(VTK_QT_ARGS
      -DVTK_USE_CARBON:BOOL=OFF
      -DVTK_USE_COCOA:BOOL=ON # Default to Cocoa, VTK/CMakeLists.txt will enable Carbon and disable cocoa if needed
      -DVTK_USE_X:BOOL=OFF
      #-DVTK_USE_RPATH:BOOL=ON # Unused
      #-DDESIRED_QT_VERSION:STRING=4 # Unused
      -DVTK_USE_GUISUPPORT:BOOL=ON
      -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
      -DVTK_USE_QT:BOOL=ON
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      )
  endif()

  # Disable Tk when Python wrapping is enabled
  if (Slicer_USE_PYTHONQT)
    list(APPEND VTK_QT_ARGS -DVTK_USE_TK:BOOL=OFF)
  endif()

  set(slicer_TCL_LIB)
  set(slicer_TK_LIB)
  set(slicer_TCLSH)
  set(VTK_TCL_ARGS)
  if(VTK_WRAP_TCL)
    if(WIN32)
      set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/tcl84.lib)
      set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/tk84.lib)
      set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh.exe)
    elseif(APPLE)
      set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtcl8.4.dylib)
      set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtk8.4.dylib)
      set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh84)
    else()
      set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtcl8.4.so)
      set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtk8.4.so)
      set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh84)
    endif()
    set(VTK_TCL_ARGS
      -DTCL_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
      -DTK_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
      -DTCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
      -DTK_LIBRARY:FILEPATH=${slicer_TK_LIB}
      -DTCL_TCLSH:FILEPATH=${slicer_TCLSH}
      )
  endif()

  set(CUSTOM_BUILD_COMMAND)
  if(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
    # Use ${MAKE} as build command to propagate parallel make option
    SET(CUSTOM_BUILD_COMMAND BUILD_COMMAND "$(MAKE)")
  endif()

  if(UNIX)
    configure_file(SuperBuild/VTK_build_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/VTK_build_step.cmake
      @ONLY)
    set(CUSTOM_BUILD_COMMAND BUILD_COMMAND ${CMAKE_COMMAND}
      -DMAKE_COMMAND=$(MAKE)
      -P ${CMAKE_CURRENT_BINARY_DIR}/VTK_build_step.cmake)
  endif()

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  set(${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY "github.com/Slicer/VTK.git" CACHE STRING "Repository from which to get VTK" FORCE)
  set(${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG "7574536583011a4a216d754cf1adec7edef505b5" CACHE STRING "VTK git tag to use" FORCE)

  mark_as_advanced(${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY ${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    GIT_REPOSITORY "${git_protocol}://${${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY}"
    GIT_TAG ${${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG}
    UPDATE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    ${CUSTOM_BUILD_COMMAND}
    CMAKE_ARGS
      ${ep_common_compiler_args}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DVTK_USE_PARALLEL:BOOL=ON
      -DVTK_DEBUG_LEAKS:BOOL=${Slicer_USE_VTK_DEBUG_LEAKS}
      -DVTK_LEGACY_REMOVE:BOOL=OFF
      -DVTK_LEGACY_SILENT:BOOL=ON
      -DVTK_WRAP_TCL:BOOL=${VTK_WRAP_TCL}
      #-DVTK_USE_RPATH:BOOL=ON # Unused
      ${VTK_TCL_ARGS}
      -DVTK_WRAP_PYTHON:BOOL=${VTK_WRAP_PYTHON}
      -DVTK_INSTALL_LIB_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      ${VTK_PYTHON_ARGS}
      ${VTK_QT_ARGS}
      ${VTK_MAC_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${VTK_DEPENDENCIES}
    )
  set(VTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(VTK_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  # The project is provided using VTK_DIR and VTK_SOURCE_DIR, nevertheless since other
  # project may depend on VTK, let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${VTK_DEPENDENCIES}")
endif()

