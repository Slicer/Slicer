#-----------------------------------------------------------------------------
# SlicerCPackBundleVerify
#-----------------------------------------------------------------------------
#
# Standalone helper script that checks whether a macOS application bundle is
# self-contained, i.e. that every embedded Mach-O binary can be loaded on a
# machine that only has the bundle (plus the OS), with no dependency on the build
# tree or on libraries installed on the packaging machine.
#
# It is the validation counterpart to SlicerCPackBundleFixup.cmake.in: the fixup
# script embeds dependencies and rewrites install names / rpaths, and this script
# verifies the result.
#
# What it checks
# --------------
# For every Mach-O file in the bundle (executables, *.dylib, *.so and framework
# binaries) it parses the load commands ("otool -l") and reports:
#
#   FATAL (fail the verification):
#   1. Frameworks whose actual binary is missing (an empty ".framework"
#      skeleton), which happens when the fixup fails to copy the library.
#   2. Dependent libraries (LC_LOAD_DYLIB / LC_REEXPORT_DYLIB /
#      LC_LOAD_UPWARD_DYLIB) that cannot be resolved, or that resolve to a file
#      *outside* the bundle. Resolution follows the same rules as dyld: @rpath is
#      expanded against the binary's own LC_RPATH entries as well as the main
#      executable's (dyld builds its run-path list from the whole load chain,
#      which always starts at the main executable, so plugins that carry no rpath
#      of their own still load correctly), and @loader_path / @executable_path are
#      expanded relative to the binary / main executable. Absolute system
#      libraries (/usr/lib, /System) are allowed.
#
#   WARNING (reported, but do not fail):
#   3. Non-relocatable LC_RPATH entries, i.e. absolute paths that are not system
#      locations (build-tree leaks such as ".../CTK-build/.../lib" or a
#      developer's Qt directory, and foreign paths baked into bundled third-party
#      Python wheels). A stale rpath is harmless on its own -- dyld skips paths
#      that do not exist -- so this is a hygiene signal, not a loadability error;
#      genuinely unsatisfied dependencies are caught by check 2 instead.
#   4. Dependencies on optional external backends under /usr/local or /opt (e.g.
#      the ODBC / PostgreSQL / Mimer Qt SQL drivers), and unresolved weak
#      dependencies. These are not bundled by design and only disable the
#      corresponding optional plugin.
#
# This intentionally does NOT use BundleUtilities' verify_app(): that routine
# treats unresolved @rpath references as non-fatal warnings and therefore passes
# bundles that are missing entire frameworks.
#
# When to run it
# --------------
# Run it *after packaging*, on the fixed-up bundle. The bundle only becomes
# self-contained once the CPack fixup script has run (as part of "make install"
# or "make package" / cpack). Do NOT run it on the freshly built application in
# the build tree: at that point the bundle still links against the build tree, so
# verification is expected to fail.
#
# How to run it
# -------------
# The recommended way is the 'packageverify' convenience target (added by
# SlicerCPack.cmake), which runs this script against the staged bundle with the
# required variable filled in automatically. It verifies the already-built
# package, so run 'make package' first:
#
#   make package
#   make packageverify        # or: cmake --build <build> --target packageverify
#
# Alternatively, invoke it directly in script mode, providing the directory that
# contains the bundle:
#
#   cmake \
#     -DSlicer_INSTALL_DIR=/path/to/dir-containing-the-app \
#     -P /path/to/Slicer/CMake/SlicerCPackBundleVerify.cmake
#
# Slicer_INSTALL_DIR may either directly contain the ".app" bundle or contain the
# CPack staging subdirectory that holds it (e.g. the DragNDrop directory
# "<build>/_CPack_Packages/<system>/DragNDrop/"). The bundle itself is located by
# globbing, so its exact (date-dependent) name does not need to be known.
#
# On success the script reports the number of binaries verified; on failure it
# lists every problem found and aborts with a fatal error.
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16.3)

# Sanity checks
if(NOT DEFINED Slicer_INSTALL_DIR)
  message(FATAL_ERROR "Variable Slicer_INSTALL_DIR is expected to be defined !")
endif()
if(NOT IS_DIRECTORY "${Slicer_INSTALL_DIR}")
  message(FATAL_ERROR "Slicer_INSTALL_DIR is set to a nonexistent directory ! "
    "Has the package been built (e.g. 'make package') ? [${Slicer_INSTALL_DIR}]")
endif()

# -----------------------------------------------------------------------------
# Locate the application bundle to verify. Slicer_INSTALL_DIR may either directly
# contain the ".app" bundle or contain the CPack staging subdirectory that holds
# it. Globbing avoids having to reconstruct the exact (date-dependent) bundle and
# package names.
# -----------------------------------------------------------------------------
file(GLOB app_candidates
  "${Slicer_INSTALL_DIR}/*.app"
  "${Slicer_INSTALL_DIR}/*/*.app"
  )
list(REMOVE_DUPLICATES app_candidates)

list(LENGTH app_candidates app_count)
if(app_count EQUAL 0)
  message(FATAL_ERROR "No '.app' bundle found under directory ! "
    "Has the package been built (e.g. 'make package') ? [${Slicer_INSTALL_DIR}]")
elseif(app_count GREATER 1)
  string(REPLACE ";" "\n  " _app_list "${app_candidates}")
  message(FATAL_ERROR "Expected exactly one '.app' bundle under [${Slicer_INSTALL_DIR}] "
    "but found ${app_count}:\n  ${_app_list}\n"
    "Remove stale packages (e.g. clean the _CPack_Packages directory) and retry.")
endif()

list(GET app_candidates 0 app_path)
get_filename_component(app_real "${app_path}" REALPATH)
set(exe_dir "${app_real}/Contents/MacOS")

find_program(OTOOL_EXECUTABLE otool)
if(NOT OTOOL_EXECUTABLE)
  message(FATAL_ERROR "Could not find 'otool'; this script requires the macOS/Xcode command line tools.")
endif()

message(STATUS "Verifying bundle is self-contained: ${app_path}")

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------

# Return TRUE in ${out} if ${f} is a Mach-O file (checked via its magic number).
function(_bv_is_macho f out)
  if(IS_DIRECTORY "${f}")
    set(${out} FALSE PARENT_SCOPE)
    return()
  endif()
  file(READ "${f}" _magic LIMIT 4 HEX)
  # feedface/feedfacf: 32/64-bit; cafebabe/bebafeca: fat; c[ef]faedfe: byte-swapped.
  if(_magic MATCHES "^(feedface|feedfacf|cafebabe|bebafeca|cefaedfe|cffaedfe)")
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()

# Substitute the @loader_path / @executable_path tokens in ${path}.
function(_bv_expand_at path loader_dir out)
  string(REPLACE "@loader_path" "${loader_dir}" _p "${path}")
  string(REPLACE "@executable_path" "${exe_dir}" _p "${_p}")
  set(${out} "${_p}" PARENT_SCOPE)
endfunction()

# Parse "otool -l" for ${binary} and return its LC_RPATH paths (${out_rpaths}),
# its non-weak dependent libraries (${out_deps}) and its weak dependent libraries
# (${out_weak}).
function(_bv_parse binary out_rpaths out_deps out_weak)
  execute_process(
    COMMAND "${OTOOL_EXECUTABLE}" -l "${binary}"
    OUTPUT_VARIABLE _out
    ERROR_QUIET
    RESULT_VARIABLE _rv
    )
  set(_rpaths "")
  set(_deps "")
  set(_weak "")
  if(_rv EQUAL 0)
    string(REPLACE "\n" ";" _lines "${_out}")
    set(_cur "")
    foreach(_ln IN LISTS _lines)
      if(_ln MATCHES "cmd (LC_[A-Z_]+)")
        set(_cur "${CMAKE_MATCH_1}")
      elseif(_ln MATCHES "path (.+) \\(offset")
        if(_cur STREQUAL "LC_RPATH")
          list(APPEND _rpaths "${CMAKE_MATCH_1}")
        endif()
      elseif(_ln MATCHES "name (.+) \\(offset")
        if(_cur STREQUAL "LC_LOAD_DYLIB"
            OR _cur STREQUAL "LC_REEXPORT_DYLIB"
            OR _cur STREQUAL "LC_LOAD_UPWARD_DYLIB")
          list(APPEND _deps "${CMAKE_MATCH_1}")
        elseif(_cur STREQUAL "LC_LOAD_WEAK_DYLIB")
          list(APPEND _weak "${CMAKE_MATCH_1}")
        endif()
        # LC_ID_DYLIB name (the library's own install name) is intentionally ignored.
      endif()
    endforeach()
  endif()
  set(${out_rpaths} "${_rpaths}" PARENT_SCOPE)
  set(${out_deps} "${_deps}" PARENT_SCOPE)
  set(${out_weak} "${_weak}" PARENT_SCOPE)
endfunction()

# Resolve a single dependency ${dep} loaded by a binary whose directory is
# ${loader_dir}, given the binary's ${rpaths}. Sets ${out_status} to one of:
#   INSIDE   - resolved to a file inside the bundle (good)
#   SYSTEM   - an allowed absolute system library (/usr/lib, /System)
#   EXTERNAL - an absolute path to an optional third-party location (/usr/local,
#              /opt); these are backends such as the ODBC/PostgreSQL/Mimer Qt SQL
#              drivers that are not bundled by design and whose absence only
#              disables the corresponding plugin (reported as a warning)
#   OUTSIDE  - resolved, but to a file outside the bundle (not self-contained)
#   MISSING  - could not be resolved at all
function(_bv_resolve dep loader_dir rpaths out_status)
  set(_resolved "")
  if(dep MATCHES "^@rpath/")
    string(REGEX REPLACE "^@rpath/" "" _rest "${dep}")
    # Search the binary's own LC_RPATH entries first, ...
    foreach(_rp IN LISTS rpaths)
      _bv_expand_at("${_rp}" "${loader_dir}" _rpdir)
      if(EXISTS "${_rpdir}/${_rest}")
        set(_resolved "${_rpdir}/${_rest}")
        break()
      endif()
    endforeach()
    # ... then the main executable's run-path directories (see note where
    # global_rpath_dirs is computed).
    if(_resolved STREQUAL "")
      foreach(_gp IN LISTS global_rpath_dirs)
        if(EXISTS "${_gp}/${_rest}")
          set(_resolved "${_gp}/${_rest}")
          break()
        endif()
      endforeach()
    endif()
  elseif(dep MATCHES "^@(loader_path|executable_path)/")
    _bv_expand_at("${dep}" "${loader_dir}" _cand)
    if(EXISTS "${_cand}")
      set(_resolved "${_cand}")
    endif()
  elseif(dep MATCHES "^/")
    if(dep MATCHES "^(/usr/lib/|/System/)")
      set(${out_status} "SYSTEM" PARENT_SCOPE)
      return()
    elseif(dep MATCHES "^(/usr/local/|/opt/)")
      set(${out_status} "EXTERNAL" PARENT_SCOPE)
      return()
    endif()
    if(EXISTS "${dep}")
      set(_resolved "${dep}")
    endif()
  endif()

  if(_resolved STREQUAL "")
    set(${out_status} "MISSING" PARENT_SCOPE)
    return()
  endif()
  get_filename_component(_rr "${_resolved}" REALPATH)
  string(FIND "${_rr}" "${app_real}/" _idx)
  if(_idx EQUAL 0)
    set(${out_status} "INSIDE" PARENT_SCOPE)
  else()
    set(${out_status} "OUTSIDE" PARENT_SCOPE)
  endif()
endfunction()

# -----------------------------------------------------------------------------
# Determine the "global" run-path directories contributed by the main bundle
# executable. dyld builds the run path list used to resolve @rpath from the whole
# chain of images leading to a given library, which always starts at the main
# executable. Many bundled plugins (e.g. Python extension modules) rely on the
# main executable's LC_RPATH rather than carrying their own, so accounting for it
# here avoids false "unresolved" reports for libraries that do load correctly.
# Only relocatable (@-prefixed) rpaths are used; absolute build-tree rpaths are
# reported separately and must not count towards resolution.
# -----------------------------------------------------------------------------
set(global_rpath_dirs "")
file(GLOB _mainexe_candidates "${app_real}/Contents/MacOS/*")
foreach(_mc IN LISTS _mainexe_candidates)
  _bv_is_macho("${_mc}" _im)
  if(_im)
    _bv_parse("${_mc}" _mrpaths _mdeps _mweak)
    foreach(_rp IN LISTS _mrpaths)
      if(_rp MATCHES "^@")
        _bv_expand_at("${_rp}" "${exe_dir}" _gp)
        list(APPEND global_rpath_dirs "${_gp}")
      endif()
    endforeach()
  endif()
endforeach()
if(global_rpath_dirs)
  list(REMOVE_DUPLICATES global_rpath_dirs)
endif()

# -----------------------------------------------------------------------------
# Collect the list of Mach-O files to verify.
# -----------------------------------------------------------------------------
set(macho_files "")

# Shared libraries and Python/loadable-module objects (always Mach-O).
file(GLOB_RECURSE _libs "${app_real}/*.dylib" "${app_real}/*.so")
list(APPEND macho_files ${_libs})

# Framework binaries, and detection of empty ".framework" skeletons.
set(missing_framework_binaries "")
file(GLOB _framework_dirs "${app_real}/Contents/Frameworks/*.framework")
foreach(_fw IN LISTS _framework_dirs)
  get_filename_component(_fwname "${_fw}" NAME_WE)
  set(_fwbin "")
  foreach(_cand "${_fw}/Versions/Current/${_fwname}" "${_fw}/${_fwname}")
    if(EXISTS "${_cand}" AND NOT IS_DIRECTORY "${_cand}")
      # EXISTS follows symlinks, so a dangling symlink is correctly rejected.
      set(_fwbin "${_cand}")
      break()
    endif()
  endforeach()
  if(_fwbin STREQUAL "")
    # Fall back to any Mach-O file under Versions/*/ (non-standard binary name).
    file(GLOB _vbins "${_fw}/Versions/*/*")
    foreach(_vb IN LISTS _vbins)
      _bv_is_macho("${_vb}" _im)
      if(_im)
        set(_fwbin "${_vb}")
        break()
      endif()
    endforeach()
  endif()
  if(_fwbin STREQUAL "")
    get_filename_component(_fwshort "${_fw}" NAME)
    list(APPEND missing_framework_binaries "${_fwshort}")
  else()
    list(APPEND macho_files "${_fwbin}")
  endif()
endforeach()

# Executables that carry no extension (main app, tools, CLI modules, ...).
file(GLOB_RECURSE _maybe_exes
  "${app_real}/Contents/MacOS/*"
  "${app_real}/Contents/bin/*"
  "${app_real}/Contents/libexec/*"
  )
foreach(_f IN LISTS _maybe_exes)
  if(NOT IS_DIRECTORY "${_f}")
    _bv_is_macho("${_f}" _im)
    if(_im)
      list(APPEND macho_files "${_f}")
    endif()
  endif()
endforeach()

if(macho_files)
  list(REMOVE_DUPLICATES macho_files)
endif()

# -----------------------------------------------------------------------------
# Verify each Mach-O file. Findings are aggregated by dependency / rpath value so
# that a library missing from N binaries is reported once with a count.
# -----------------------------------------------------------------------------
set(unresolved_names "")   # dependency name -> reported once
set(unresolved_counts "")  # parallel list of counts
set(unresolved_examples "")# parallel list of one example binary
set(unresolved_status "")  # parallel list of MISSING/OUTSIDE

set(weak_names "")
set(external_names "")

set(badrpath_values "")
set(badrpath_counts "")
set(badrpath_examples "")

# Aggregate a hard finding for dependency ${name} (status ${status}) seen in ${binrel}.
macro(_bv_add_finding name status binrel)
  list(FIND unresolved_names "${name}" _fi)
  if(_fi EQUAL -1)
    list(APPEND unresolved_names "${name}")
    list(APPEND unresolved_counts 1)
    list(APPEND unresolved_examples "${binrel}")
    list(APPEND unresolved_status "${status}")
  else()
    list(GET unresolved_counts ${_fi} _c)
    math(EXPR _c "${_c} + 1")
    list(REMOVE_AT unresolved_counts ${_fi})
    list(INSERT unresolved_counts ${_fi} ${_c})
  endif()
endmacro()

macro(_bv_add_badrpath value binrel)
  list(FIND badrpath_values "${value}" _fi)
  if(_fi EQUAL -1)
    list(APPEND badrpath_values "${value}")
    list(APPEND badrpath_counts 1)
    list(APPEND badrpath_examples "${binrel}")
  else()
    list(GET badrpath_counts ${_fi} _c)
    math(EXPR _c "${_c} + 1")
    list(REMOVE_AT badrpath_counts ${_fi})
    list(INSERT badrpath_counts ${_fi} ${_c})
  endif()
endmacro()

list(LENGTH macho_files _n_binaries)
foreach(_bin IN LISTS macho_files)
  get_filename_component(_loader_dir "${_bin}" DIRECTORY)
  string(REPLACE "${app_real}/" "" _binrel "${_bin}")

  _bv_parse("${_bin}" _rpaths _deps _weak)

  # Non-relocatable rpath entries (build-tree / developer-machine leaks).
  foreach(_rp IN LISTS _rpaths)
    if(_rp MATCHES "^@")
      # @loader_path / @executable_path / @rpath -- relocatable, fine.
    elseif(_rp MATCHES "^(/usr/lib|/System/)")
      # Allowed system location.
    else()
      _bv_add_badrpath("${_rp}" "${_binrel}")
    endif()
  endforeach()

  # Regular dependencies must resolve inside the bundle.
  foreach(_dep IN LISTS _deps)
    _bv_resolve("${_dep}" "${_loader_dir}" "${_rpaths}" _status)
    if(_status STREQUAL "MISSING" OR _status STREQUAL "OUTSIDE")
      _bv_add_finding("${_dep}" "${_status}" "${_binrel}")
    elseif(_status STREQUAL "EXTERNAL")
      list(FIND external_names "${_dep}" _fi)
      if(_fi EQUAL -1)
        list(APPEND external_names "${_dep}")
      endif()
    endif()
  endforeach()

  # Weak dependencies, and optional external backends, are warnings only.
  foreach(_dep IN LISTS _weak)
    _bv_resolve("${_dep}" "${_loader_dir}" "${_rpaths}" _status)
    if(_status STREQUAL "MISSING" OR _status STREQUAL "OUTSIDE" OR _status STREQUAL "EXTERNAL")
      list(FIND weak_names "${_dep}" _fi)
      if(_fi EQUAL -1)
        list(APPEND weak_names "${_dep}")
      endif()
    endif()
  endforeach()
endforeach()

# -----------------------------------------------------------------------------
# Report
# -----------------------------------------------------------------------------
set(_errors 0)

list(LENGTH missing_framework_binaries _n_missing_fw)
if(_n_missing_fw GREATER 0)
  math(EXPR _errors "${_errors} + ${_n_missing_fw}")
  message("")
  message("ERROR: ${_n_missing_fw} framework(s) do not contain their binary "
    "(empty '.framework' skeleton -- the fixup did not embed the library):")
  foreach(_fw IN LISTS missing_framework_binaries)
    message("  - ${_fw}")
  endforeach()
endif()

list(LENGTH unresolved_names _n_unresolved)
if(_n_unresolved GREATER 0)
  math(EXPR _errors "${_errors} + ${_n_unresolved}")
  message("")
  message("ERROR: ${_n_unresolved} dependency(ies) do not resolve inside the bundle:")
  math(EXPR _last "${_n_unresolved} - 1")
  foreach(_i RANGE ${_last})
    list(GET unresolved_names ${_i} _nm)
    list(GET unresolved_status ${_i} _st)
    list(GET unresolved_counts ${_i} _ct)
    list(GET unresolved_examples ${_i} _ex)
    message("  - [${_st}] ${_nm}")
    message("      referenced by ${_ct} binary(ies), e.g. ${_ex}")
  endforeach()
endif()

list(LENGTH badrpath_values _n_badrpath)
if(_n_badrpath GREATER 0)
  message("")
  message("WARNING: ${_n_badrpath} non-relocatable LC_RPATH entry(ies) "
    "(absolute paths leaking the build/packaging machine or bundled Python "
    "wheels; non-fatal -- dyld ignores paths that do not exist):")
  math(EXPR _last "${_n_badrpath} - 1")
  foreach(_i RANGE ${_last})
    list(GET badrpath_values ${_i} _v)
    list(GET badrpath_counts ${_i} _ct)
    list(GET badrpath_examples ${_i} _ex)
    message("  - ${_v}")
    message("      present in ${_ct} binary(ies), e.g. ${_ex}")
  endforeach()
endif()

list(LENGTH external_names _n_external)
if(_n_external GREATER 0)
  message("")
  message("WARNING: ${_n_external} dependency(ies) on optional external libraries "
    "(/usr/local, /opt) that are not bundled by design "
    "(non-fatal; disables only the corresponding plugin, e.g. a Qt SQL driver):")
  foreach(_nm IN LISTS external_names)
    message("  - ${_nm}")
  endforeach()
endif()

list(LENGTH weak_names _n_weak)
if(_n_weak GREATER 0)
  message("")
  message("WARNING: ${_n_weak} unresolved weak dependency(ies) "
    "(non-fatal; only loaded if present at runtime):")
  foreach(_nm IN LISTS weak_names)
    message("  - ${_nm}")
  endforeach()
endif()

message("")
if(_errors GREATER 0)
  message(FATAL_ERROR "Bundle verification FAILED with ${_errors} error(s): "
    "the application bundle is not self-contained. See the report above.")
endif()

message(STATUS "Bundle verification PASSED: ${_n_binaries} Mach-O file(s) checked, "
  "all dependencies resolve inside the bundle.")
