# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#
#  W A R N I N G
#  -------------
#
# This file is not part of the Slicer API.  It exists purely as an
# implementation detail.  This CMake module may change from version to
# version without notice, or even be removed.
#
# We mean it.
#

#.rst:
# FindVcvars
# ----------
#
# Finds the "vcvars" batch script associated
# with the microsoft compiler version stored in the
# CMake variable :variable:`MSVC_VERSION`.
#
# Setting ``Vcvars_ANY_VERSION`` to ``1`` allows to find "vcvars"
# batch script of any version of Visual Studio installed on the system.
#
# This will define the following variables:
#
# .. variable:: Vcvars_BATCH_FILE
#
#   Path to `vcvars32.bat`, `vcvarsamd64.bat` or `vcvars64.bat`.
#
# .. variable:: Vcvars_WRAPPER_BATCH_FILE
#
#   Path to a generated wrapper script allowing to execute program after
#   setting environment defined by `Vcvars_BAT`.
#
#   For example, it can be used within :module:`ExternalProject` steps
#   specifying command like this::
#
#     set(cmd_wrapper)
#     if(MSVC)
#       find_package(Vcvars REQUIRED)
#       set(cmd_wrapper ${Vcvars_WRAPPER_BATCH_FILE})
#     endif()
#
#     ExternalProject_Add(AwesomeProject
#       [...]
#       BUILD_COMMAND ${cmd_wrapper} <command> arg1 arg2 [...]
#       [...]
#       )
#

#
# Default
#
if(NOT DEFINED Vcvars_ANY_VERSION)
  set(Vcvars_ANY_VERSION 0)
endif()
if(NOT DEFINED Vcvars_BATCH_FILE)
  set(Vcvars_BATCH_FILE "Vcvars_BATCH_FILE-NOTFOUND")
endif()
if(NOT DEFINED Vcvars_WRAPPER_BATCH_FILE)
  set(Vcvars_WRAPPER_BATCH_FILE "Vcvars_WRAPPER_BATCH_FILE-NOTFOUND")
endif()

#
# Possible improvements:
#
#  * Support lookup of "Microsoft Visual C++ Compiler for Python 2.7 (x86, amd64)"
#  * Provide a CMake function for easy lookup
#

# Sanity checks
if(NOT DEFINED CMAKE_SIZEOF_VOID_P)
  message(FATAL_ERROR "CMAKE_SIZEOF_VOID_P variable is not defined !")
endif()

set(_vs_versions_installer "")
set(_vs_versions_registry "")

if(Vcvars_ANY_VERSION)

  foreach(vs RANGE 15 15 -1) # change the first number to the largest supported version
    list(APPEND _vs_versions_installer ${vs})
  endforeach()

  set(_vs_versions_registry
    "14.0"
    "12.0"
    "11.0"
    "10.0"
    "9.0"
    "8.0"
    "7.1"
    "7.0"
    "6.0"
    )
else()

  # Exit if current generator is different from VisualStudio and vcvars is *not* REQUIRED
  if(NOT MSVC AND NOT Vcvars_FIND_REQUIRED)
    return()
  endif()

  # Sanity checks
  if(MSVC AND NOT DEFINED MSVC_VERSION)
    message(FATAL_ERROR "MSVC variable is TRUE but MSVC_VERSION is not defined !")
  endif()

  if(MSVC_VERSION EQUAL 1910)     # VS 2017
    set(_vs_versions_installer "15")
  elseif(MSVC_VERSION EQUAL 1900)     # VS 2015
    set(_vs_versions_registry "14.0")
  elseif(MSVC_VERSION EQUAL 1800) # VS 2013
    set(_vs_versions_registry "12.0")
  elseif(MSVC_VERSION EQUAL 1700) # VS 2012
    set(_vs_versions_registry "11.0")
  elseif(MSVC_VERSION EQUAL 1600) # VS 2010
    set(_vs_versions_registry "10.0")
  elseif(MSVC_VERSION EQUAL 1500) # VS 2008
    set(_vs_versions_registry "9.0")
  elseif(MSVC_VERSION EQUAL 1400) # VS 2005
    set(_vs_versions_registry "8.0")
  elseif(MSVC_VERSION EQUAL 1310) # VS 2003
    set(_vs_versions_registry "7.1")
  elseif(MSVC_VERSION EQUAL 1300) # VS 2002
    set(_vs_versions_registry "7.0")
  elseif(MSVC_VERSION EQUAL 1200) # VS 6.0
    set(_vs_versions_registry "6.0")
  endif()
endif()

# Query the VS Installer tool for locations of VS 2017 and above.
set(_vs_installer_paths "")
foreach(vs IN LISTS _vs_versions_installer)
  cmake_host_system_information(RESULT _vs_dir QUERY VS_${vs}_DIR)
  if(_vs_dir)
    list(APPEND _vs_installer_paths "${_vs_dir}/VC/Auxiliary/Build")
  endif()
endforeach()

# Registry keys for locations of VS 2015 and below
set(_hkeys
  "HKEY_USERS"
  "HKEY_CURRENT_USER"
  "HKEY_LOCAL_MACHINE"
  "HKEY_CLASSES_ROOT"
  )

set(_suffixes
  ""
  "_Config"
  )

set(_reg_paths)
foreach(_vs_version IN LISTS _vs_versions_registry)
  foreach(_hkey IN LISTS _hkeys)
    foreach(_suffix IN LISTS _suffixes)
      set(_vc "VC")
      if(_vs_version STREQUAL "6.0")
        set(_vc "Microsoft Visual C++")
      endif()
      list(APPEND _reg_paths
        "[${_hkey}\\SOFTWARE\\Microsoft\\VisualStudio\\${_vs_version}${_suffix}\\Setup\\${_vc};ProductDir]"
        )
    endforeach()
  endforeach()
endforeach()

if(NOT Vcvars_BATCH_FILE AND "${CMAKE_SIZEOF_VOID_P}" EQUAL 4)

  set(_paths ${_vs_installer_paths})
  foreach(_reg_path IN LISTS _reg_paths)
    list(APPEND _paths "${_reg_path}/bin")
  endforeach()

  find_program(Vcvars_BATCH_FILE NAMES vcvars32.bat
    DOC "Visual Studio vcvars32.bat"
    PATHS ${_paths}
    )

elseif(NOT Vcvars_BATCH_FILE AND "${CMAKE_SIZEOF_VOID_P}" EQUAL 8)

  set(_paths ${_vs_installer_paths})
  foreach(_reg_path IN LISTS _reg_paths)
    list(APPEND _paths "${_reg_path}/bin/amd64")
  endforeach()

  find_program(Vcvars_BATCH_FILE NAMES vcvarsamd64.bat vcvars64.bat
    DOC "Visual Studio vcvarsamd64.bat"
    PATHS ${_paths}
    )

endif()

# Cleanup
unset(_vs_installer_paths)
unset(_vs_versions_installer)
unset(_vs_versions_registry)
unset(_paths)

# Configure wrapper script
set(Vcvars_WRAPPER_BATCH_FILE)
if(Vcvars_BATCH_FILE)

  set(_in "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/Vcvars_wrapper.bat.in")
  get_filename_component(_basename ${Vcvars_BATCH_FILE} NAME_WE)
  set(_out "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_basename}_wrapper.bat")
  file(WRITE ${_in} "call \"@Vcvars_BATCH_FILE@\"
%*
")
  configure_file(${_in} ${_out} @ONLY)

  set(Vcvars_WRAPPER_BATCH_FILE ${_out})

  unset(_in)
  unset(_out)
endif()

# Outputs
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vcvars
  FOUND_VAR Vcvars_FOUND
  REQUIRED_VARS
    MSVC
    Vcvars_BATCH_FILE
    Vcvars_WRAPPER_BATCH_FILE
  )
