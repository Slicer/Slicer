
#-----------------------------------------------------------------------------
# Get and build itk

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Insight)

if(NOT DEFINED ITK_DIR)
#  message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/ITK.git"
    GIT_TAG "origin/slicer-4.0"
    UPDATE_COMMAND ""
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DCMAKE_SKIP_RPATH:BOOL=ON
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DITK_USE_REVIEW:BOOL=ON
      -DITK_USE_REVIEW_STATISTICS:BOOL=ON
      -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
      -DITK_USE_PORTABLE_ROUND:BOOL=ON
      -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON
      -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
      -DITK_LEGACY_REMOVE:BOOL=ON
    INSTALL_COMMAND ""
    DEPENDS 
      ${Insight_DEPENDENCIES}
    )
  set(ITK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  # The project is provided using ITK_DIR, nevertheless since other project may depend on ITK, 
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${Insight_DEPENDENCIES}")
endif()
