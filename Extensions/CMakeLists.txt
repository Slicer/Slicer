################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

INCLUDE(SlicerFunctionExtractExtensionDescription)

# Collect extension description file (*.s4ext)
FILE(GLOB_RECURSE s4extfiles "*.s4ext")

FOREACH(file ${s4extfiles})
  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX Extension)
  
  #foreach(v SCM SCMURL DEPENDS HOMEPAGE CATEGORY STATUS DESCRIPTION)
  #  message(${v}:${Extension_SEXT_${v}})
  #endforeach()
  
  # Extract file basename
  get_filename_component(extension_name ${file} NAME_WE)
  IF("${extension_name}" STREQUAL "")
    MESSAGE(WARNING "Failed to extract extension name associated with file: ${file}")
  ELSE()
    MESSAGE(STATUS "Configuring extension: ${extension_name} (${file})")
    IF("${Extension_SEXT_SCM}" STREQUAL "" AND "${Extension_SEXT_SCMURL}" STREQUAL "")
      MESSAGE(WARNING "Failed to extract extension information associated file: ${file}")
    ELSE()

      SET(sext_add_project True)
      SET(sext_scm_options)
      IF(${Extension_SEXT_SCM} STREQUAL "git")
        SET(extension_source_dir ${extension_name})
        SET(sext_scm_options "GIT_REPOSITORY ${Extension_SEXT_SCMURL}
GIT_TAG \"origin/master\"")
      ELSEIF(${Extension_SEXT_SCM} STREQUAL "svn")
        SET(extension_source_dir ${extension_name})
        SET(sext_scm_options "SVN_REPOSITORY ${Extension_SEXT_SCMURL}
SVN_REVISION -r \"HEAD\"")
      ELSEIF(${Extension_SEXT_SCM} STREQUAL "local")
        SET(extension_source_dir ${Extension_SEXT_SCMURL})
      ELSE()
        SET(sext_add_project False)
        MESSAGE(WARNING "Unknown type of SCM [${Extension_SEXT_SCM}] associated with extension named ${extension_name} - See file ${file}")
      ENDIF()
      IF(sext_add_project)
        # Add an external project
        set(proj Slicer-extension-${extension_name})
        ExternalProject_Add(${proj}
          DOWNLOAD_COMMAND ""
          ${sext_scm_options}
          INSTALL_COMMAND ""
          SOURCE_DIR ${extension_source_dir}
          BINARY_DIR ${extension_name}-build
          CMAKE_GENERATOR ${gen}
          CMAKE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DBUILD_TESTING:BOOL=OFF
            -DSlicer_DIR:PATH=${Slicer_BINARY_DIR}/Slicer-build
            -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
            -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}
            #-DCVS_EXECUTABLE:FILEPATH=${CVS_EXECUTABLE}
          DEPENDS
            Slicer
          )
      ENDIF()
    ENDIF()
  ENDIF()
ENDFOREACH()
