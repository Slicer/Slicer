
# Sanity checks
set(expected_defined_vars Slicer_WC_REVISION QT_VERSION_MAJOR QT_VERSION_MINOR CMAKE_GENERATOR Slicer_EXTENSIONS_TRACK_QUALIFIER)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

set(expected_existing_vars GIT_EXECUTABLE Subversion_SVN_EXECUTABLE Slicer_DIR Slicer_EXTENSION_DESCRIPTION_DIR Slicer_CMAKE_DIR Slicer_LOCAL_EXTENSIONS_DIR)
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

# Convert to absolute path
foreach(varname Slicer_DIR Slicer_EXTENSION_DESCRIPTION_DIR Slicer_CMAKE_DIR Slicer_LOCAL_EXTENSIONS_DIR)
  if(NOT IS_ABSOLUTE ${${varname}})
    set(${varname} "${CMAKE_CURRENT_BINARY_DIR}/${${varname}}")
  endif()
endforeach()

include(SlicerFunctionExtractExtensionDescription)

if(Slicer_UPLOAD_EXTENSIONS)
  include(SlicerBlockUploadExtensionPrerequisites) # Common to all extensions
endif()

#-----------------------------------------------------------------------------
# Collect extension description file (*.s4ext)
#-----------------------------------------------------------------------------
file(GLOB_RECURSE s4extfiles "${Slicer_EXTENSION_DESCRIPTION_DIR}/*.s4ext")

# Get the dependency information of each extension
set(EXTENSION_LIST)
foreach(file ${s4extfiles})
  message(STATUS "Extension:${file}")
  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX EXTENSION)
  #message(DEPENDS:${EXTENSION_SEXT_DEPENDS})
  # Extract file basename
  get_filename_component(EXTENSION_NAME ${file} NAME_WE)
  if("${EXTENSION_NAME}" STREQUAL "")
    message(WARNING "Failed to extract extension name associated with file: ${file}")
  else()
    list(APPEND EXTENSION_LIST ${EXTENSION_NAME})
    string(REGEX REPLACE "^NA$" "" EXTENSION_SEXT_DEPENDS "${EXTENSION_SEXT_DEPENDS}")
    set(EXTENSION_${EXTENSION_NAME}_DEPENDS ${EXTENSION_SEXT_DEPENDS})
  endif()
endforeach()

# Sort extensions
include(TopologicalSort)
topological_sort(EXTENSION_LIST "EXTENSION_" "_DEPENDS")

foreach(extension_name ${EXTENSION_LIST})
  # Set extension description filename using extension_name
  set(file ${Slicer_EXTENSION_DESCRIPTION_DIR}/${extension_name}.s4ext)

  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX EXTENSION)
  string(REGEX REPLACE "^NA$" "" EXTENSION_SEXT_DEPENDS "${EXTENSION_SEXT_DEPENDS}")
  set(EXTENSION_CATEGORY ${EXTENSION_SEXT_CATEGORY})
  set(EXTENSION_STATUS ${EXTENSION_SEXT_STATUS})
  set(EXTENSION_ICONURL ${EXTENSION_SEXT_ICONURL})
  set(EXTENSION_CONTRIBUTORS ${EXTENSION_SEXT_CONTRIBUTORS})
  set(EXTENSION_DESCRIPTION ${EXTENSION_SEXT_DESCRIPTION})
  set(EXTENSION_HOMEPAGE ${EXTENSION_SEXT_HOMEPAGE})
  set(EXTENSION_SCREENSHOTURLS ${EXTENSION_SEXT_SCREENSHOTURLS})
  set(EXTENSION_ENABLED ${EXTENSION_SEXT_ENABLED})

  #foreach(v SCM SCMURL SCMREVISION SVNUSERNAME SVNPASSWORD DEPENDS BUILD_SUBDIRECTORY HOMEPAGE
  #          CATEGORY CONTRIBUTORS ICONURL STATUS DESCRIPTION SCREENSHOTURLS ENABLED)
  #  message(${v}:${EXTENSION_SEXT_${v}})
  #endforeach()

  # Extract file basename
  get_filename_component(EXTENSION_NAME ${file} NAME_WE)
  if("${EXTENSION_NAME}" STREQUAL "")
    message(WARNING "Failed to extract extension name associated with file: ${file}")
  else()
    message(STATUS "Configuring extension: ${EXTENSION_NAME}")
    if("${EXTENSION_SEXT_SCM}" STREQUAL "" AND "${EXTENSION_SEXT_SCMURL}" STREQUAL "")
      message(WARNING "Failed to extract extension information associated to file: ${file}")
    else()
      set(sext_add_project True)
      set(sext_ep_options_repository)
      set(sext_ep_cmake_args)
      set(sext_revision ${EXTENSION_SEXT_SCMREVISION})
      if(${EXTENSION_SEXT_SCM} STREQUAL "git")
        set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
        if("${sext_revision}" STREQUAL "")
          set(sext_revision "origin/master")
        endif()
        set(sext_ep_options_repository
          GIT_REPOSITORY ${EXTENSION_SEXT_SCMURL} GIT_TAG ${sext_revision})
        list(APPEND sext_ep_cmake_args
           -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE})
      elseif(${EXTENSION_SEXT_SCM} STREQUAL "svn")
        set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
        if("${sext_revision}" STREQUAL "")
          set(sext_revision "HEAD")
        endif()
        set(sext_ep_options_repository
          SVN_REPOSITORY ${EXTENSION_SEXT_SCMURL} SVN_REVISION -r ${sext_revision})
        list(APPEND sext_ep_cmake_args
           -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE})
        if(NOT ${EXTENSION_SEXT_SVNUSERNAME} STREQUAL "")
           list(APPEND sext_ep_options_repository
             SVN_USERNAME "${EXTENSION_SEXT_SVNUSERNAME}"
             SVN_PASSWORD "${EXTENSION_SEXT_SVNPASSWORD}"
             SVN_TRUST_CERT 1
             )
        endif()
      elseif(${EXTENSION_SEXT_SCM} STREQUAL "local")
        set(sext_ep_options_repository DOWNLOAD_COMMAND "")
        set(EXTENSION_SOURCE_DIR ${EXTENSION_SEXT_SCMURL})
        if(NOT EXISTS ${EXTENSION_SOURCE_DIR})
          set(EXTENSION_SOURCE_DIR ${Slicer_LOCAL_EXTENSIONS_DIR}/${EXTENSION_SOURCE_DIR})
        endif()
      else()
        set(sext_add_project False)
        message(WARNING "Unknown type of SCM [${EXTENSION_SEXT_SCM}] associated with extension named ${EXTENSION_NAME} - See file ${file}")
      endif()
      if(sext_add_project)
        # Set external project DEPENDS parameter
        set(EXTENSION_DEPENDS)
        if(Slicer_SOURCE_DIR)
          set(EXTENSION_DEPENDS DEPENDS Slicer ${EXTENSION_DEPENDS})
        else()
          if(NOT "${EXTENSION_SEXT_DEPENDS}" STREQUAL "")
            set(EXTENSION_DEPENDS DEPENDS ${EXTENSION_SEXT_DEPENDS})
          endif()
        endif()
        set(EXTENSION_REBUILD_DEPENDS)
        if(NOT "${EXTENSION_SEXT_DEPENDS}" STREQUAL "")
          set(EXTENSION_REBUILD_DEPENDS DEPENDS)
          foreach(dep ${EXTENSION_SEXT_DEPENDS})
            list(APPEND EXTENSION_REBUILD_DEPENDS ${dep}-rebuild)
          endforeach()
        endif()
        if(Slicer_UPLOAD_EXTENSIONS)
          #-----------------------------------------------------------------------------
          # Slicer_UPLOAD_EXTENSIONS: TRUE
          #-----------------------------------------------------------------------------
          set(EXTENSION_SUPERBUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-build)
          set(EXTENSION_BUILD_SUBDIRECTORY ${EXTENSION_SEXT_BUILD_SUBDIRECTORY})
          include(SlicerBlockUploadExtension)
          # Add extension external project
          set(proj ${EXTENSION_NAME})
          ExternalProject_Add(${proj}
            ${sext_ep_options_repository}
            SOURCE_DIR ${EXTENSION_SOURCE_DIR}
            BINARY_DIR ${EXTENSION_NAME}-build
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${EXTENSION_UPLOAD_COMMAND}
            INSTALL_COMMAND ""
            ${EXTENSION_DEPENDS}
            )
          # This custom external project step forces the build and later
          # steps to run whenever a top level build is done...
          ExternalProject_Add_Step(${proj} forcebuild
            DEPENDEES configure
            DEPENDERS build
            ALWAYS 1
            )
          if(Slicer_SOURCE_DIR)
            # Add convenient external project allowing to build the extension
            # independently of Slicer
            ExternalProject_Add(${proj}-rebuild
              ${sext_ep_options_repository}
              SOURCE_DIR ${EXTENSION_SOURCE_DIR}
              BINARY_DIR ${EXTENSION_NAME}-build
              CONFIGURE_COMMAND ""
              BUILD_COMMAND ${EXTENSION_UPLOAD_COMMAND}
              INSTALL_COMMAND ""
              ${EXTENSION_REBUILD_DEPENDS}
              )
            # This custom external project step forces the build and later
            # steps to run whenever a top level build is done...
            ExternalProject_Add_Step(${proj}-rebuild forcebuild
              DEPENDEES configure
              DEPENDERS build
              ALWAYS 1
              )
            set_property(TARGET ${proj}-rebuild PROPERTY EXCLUDE_FROM_ALL TRUE)
          endif()
        else()

          list(APPEND sext_ep_cmake_args
            -D${EXTENSION_NAME}_BUILD_SLICER_EXTENSION:BOOL=ON
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DBUILD_TESTING:BOOL=${BUILD_TESTING}
            -DSlicer_DIR:PATH=${Slicer_DIR}
            -DSlicer_EXTENSIONS_TRACK_QUALIFIER:STRING=${Slicer_EXTENSIONS_TRACK_QUALIFIER}
            -DEXTENSION_BUILD_SUBDIRECTORY:STRING=${EXTENSION_SEXT_BUILD_SUBDIRECTORY}
            -DEXTENSION_ENABLED:BOOL=${EXTENSION_ENABLED}
            -DMIDAS_PACKAGE_URL:STRING=${MIDAS_PACKAGE_URL}
            -DMIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
            -DMIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}
            )

          #-----------------------------------------------------------------------------
          # Slicer_UPLOAD_EXTENSIONS: FALSE
          #-----------------------------------------------------------------------------
          # Add extension external project
          set(proj ${EXTENSION_NAME})
          ExternalProject_Add(${proj}
            ${sext_ep_options_repository}
            INSTALL_COMMAND ""
            SOURCE_DIR ${EXTENSION_SOURCE_DIR}
            BINARY_DIR ${EXTENSION_NAME}-build
            CMAKE_GENERATOR ${Slicer_EXTENSION_CMAKE_GENERATOR}
            CMAKE_ARGS
              ${sext_ep_cmake_args}
            ${EXTENSION_DEPENDS}
            )
          # This custom external project step forces the build and later
          # steps to run whenever a top level build is done...
          ExternalProject_Add_Step(${proj} forcebuild
            DEPENDEES configure
            DEPENDERS build
            ALWAYS 1
            )
          if(Slicer_SOURCE_DIR)
            # Add convenient external project allowing to build the extension
            # independently of Slicer
            ExternalProject_Add(${proj}-rebuild
              ${sext_ep_options_repository}
              INSTALL_COMMAND ""
              SOURCE_DIR ${EXTENSION_SOURCE_DIR}
              BINARY_DIR ${EXTENSION_NAME}-build
              CMAKE_GENERATOR ${Slicer_EXTENSION_CMAKE_GENERATOR}
              CMAKE_ARGS
                ${sext_ep_cmake_args}
              ${EXTENSION_REBUILD_DEPENDS}
              )
            # This custom external project step forces the build and later
            # steps to run whenever a top level build is done...
            ExternalProject_Add_Step(${proj}-rebuild forcebuild
              DEPENDEES configure
              DEPENDERS build
              ALWAYS 1
              )
            set_property(TARGET ${proj}-rebuild PROPERTY EXCLUDE_FROM_ALL TRUE)
          endif()
        endif()
      endif()
    endif()
  endif()
endforeach()
