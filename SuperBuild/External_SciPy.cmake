
slicer_include_once()

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
