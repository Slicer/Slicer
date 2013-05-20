
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(SciPy_DEPENDENCIES NUMPY)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SciPy)
set(proj SciPy)

#message(STATUS "${__indent}Adding project ${proj}")

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.scipy.org/svn/scipy/branches/0.7.x"
  SOURCE_DIR python/scipy
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${SciPy_DEPENDENCIES}
  )
