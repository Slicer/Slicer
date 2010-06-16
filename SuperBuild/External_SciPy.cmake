
set(proj scipy)

ExternalProject_Add(${proj}
  DEPENDS ${scipy_DEPENDENCIES}
  SVN_REPOSITORY "http://svn.scipy.org/svn/scipy/branches/0.7.x"
  SOURCE_DIR python/scipy
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS 
    ${scipy_DEPENDENCIES}
)
