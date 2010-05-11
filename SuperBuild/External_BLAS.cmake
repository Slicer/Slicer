#-----------------------------------------------------------------------------
# Get and build netlib (blas and lapack)

set(proj blas)

set(blas_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/BLAS")
set(blas_BUILD_IN_SOURCE 0)
set(blas_CONFIGURE "pwd")
set(blas_BUILD "")

if(WIN32)
else()
  set(blas_BUILD_IN_SOURCE 1)
#  set(blas_CONFIGURE sh configure)
#  set(blas_BUILD ${CMAKE_Fortran_COMPILER} -O3 -fno-second-underscore -fPIC -m64 -c *.f)
#  set(blas_INSTAL make install)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY ${blas_SVN}
  SOURCE_DIR netlib/BLAS
  CMAKE_GENERATOR ${gen}
  INSTALL_COMMAND ""
)
