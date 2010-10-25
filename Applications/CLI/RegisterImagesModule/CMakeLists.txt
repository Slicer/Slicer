#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------


PROJECT ( RegisterImagesModule )

  SET( RegisterImagesModule_INCLUDE_PATH
       ${RegisterImagesModule_SOURCE_DIR} )

  ADD_SUBDIRECTORY( Applications )
  ADD_SUBDIRECTORY( MultiResApplications )
