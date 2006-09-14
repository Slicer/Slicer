#
# Build the documentation
#
INCLUDE (Documentation)

IF (BUILD_DOCUMENTATION)
  SET(BUILD_DOXYGEN ON)
  SET(DOCUMENTATION_DOWNLOAD_VTK_TAGFILE ON)
  SET(DOCUMENTATION_HTML_TARZ ON)
  SET(DOCUMENTATION_DOWNLOAD_EXTRA_TAGFILE ON)
  SET(DOXYGEN_EXTRA_TAGFILENAME "ParaViewNightlyDoc.tag")
  SET(DOXYGEN_EXTRA_TAGFILE_REMOTE_DIR "http://www.paraview.org/doc/nightly")
  SET(DOXYGEN_EXTRA_TAGFILENAME1 "InsightDoxygen.tag")
  SET(DOXYGEN_EXTRA_TAGFILE_REMOTE_DIR1 "http://www.itk.org/Doxygen/")
  SET(DOXYGEN_PROJECT_NAME "Slicer3")
  SET(DOXYGEN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  SET(DOXYGEN_PROJECT_SOURCE_DIR "${Slicer3_SOURCE_DIR}")
  SET(VTK_DOXYGEN_HOME "${CMAKE_CURRENT_SOURCE_DIR}")
  #SET(DOXYGEN_CVSWEB_CHECKOUT "http://public.kitware.com/cgi-bin/cvsweb.cgi/~checkout~/Slicer3/")
  #SET(DOXYGEN_CVSWEB_CHECKOUT_SUFFIX "?cvsroot=Slicer3")
  SET(DOXYGEN_SOURCE_LOCATIONS_DIR "${Slicer3_SOURCE_DIR}")

  FILE(GLOB_RECURSE modules_glob RELATIVE "${Slicer3_SOURCE_DIR}"
    "${Slicer3_SOURCE_DIR}/*/*.h")
  SET(modules)
  SET(modules_paths)
  FOREACH(mod ${modules_glob})
    GET_FILENAME_COMPONENT(module_path "${mod}" PATH)
    SET(modules ${modules} "${module_path}")
    IF("${modules}" MATCHES ";")
      LIST(REMOVE_ITEM modules "${module_path}")
    ENDIF("${modules}" MATCHES ";")
    SET(modules ${modules} "${module_path}")
    SET(modules_paths ${modules_paths} "${module_path}")
  ENDFOREACH(mod)
  #LIST(SORT modules)
  #MESSAGE("Found modules: ${modules}")

  SET(DOXYGEN_SOURCE_LOCATIONS)
  FOREACH(module ${modules})
    SET(DOXYGEN_SOURCE_LOCATIONS
      "${DOXYGEN_SOURCE_LOCATIONS} \\
        \"${Slicer3_SOURCE_DIR}/${module}\"")
  ENDFOREACH(module)
  SET(DOXYGEN_PARSED_INPUT_DIRS)
  FOREACH(module ${modules})
    SET(DOXYGEN_PARSED_INPUT_DIRS
      "${DOXYGEN_PARSED_INPUT_DIRS} \\
 \"${CMAKE_CURRENT_BINARY_DIR}/dox/${module}\"")
  ENDFOREACH(module)

  SET(DOXYGEN_IGNORE_PREFIX "vtkKW vtkPV vtkSM vtk vtkITK vtkMRML vtkKW vtkSlicer itk")
  SET(DOXYGEN_TAG_FILES
    "\"@CMAKE_CURRENT_BINARY_DIR@/vtkNightlyDoc.tag=http://www.vtk.org/doc/nightly/html\"\\
    \"@CMAKE_CURRENT_BINARY_DIR@/InsightDoxygen.tag=http://www.itk.org/Doxygen/html\"\\
    \"@CMAKE_CURRENT_BINARY_DIR@/ParaViewNightlyDoc.tag=http://www.paraview.org/doc/nightly/html\"")

  #
  # Configure the script and the doxyfile, then add target
  #
  IF(NOT DOT_PATH)
    GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH)
  ENDIF(NOT DOT_PATH)

  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY IMMEDIATE)

  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/doc_makeall.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc_makeall.sh @ONLY IMMEDIATE)

  ADD_CUSTOM_TARGET(${DOXYGEN_PROJECT_NAME}DoxygenDoc
    ${BASH}
    ${CMAKE_CURRENT_BINARY_DIR}/doc_makeall.sh)

  # also a target for the combined documentation
  CONFIGURE_FILE( 
    ${CMAKE_CURRENT_SOURCE_DIR}/combined_doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/combined_doxyfile)

  CONFIGURE_FILE( 
    ${CMAKE_CURRENT_SOURCE_DIR}/doc_combined_makeall.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc_combined_makeall.sh)

  ADD_CUSTOM_TARGET(CombinedDoxygenDoc
    ${BASH}
    ${CMAKE_CURRENT_BINARY_DIR}/doc_combined_makeall.sh)


ENDIF (BUILD_DOCUMENTATION)

INCLUDE (${Slicer3_SOURCE_DIR}/Utilities/Doxygen/LocalUserOptions.cmake OPTIONAL)
