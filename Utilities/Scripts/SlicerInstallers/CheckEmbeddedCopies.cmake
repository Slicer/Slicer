#-----------------------------------------------------------------------------
# Verify that the copies of the inner scripts embedded in the outer installers
# are byte-identical to the files installed beside them.
#
# install.sh and install.ps1 each carry a copy of every inner script, because
# they have to be able to set up a Slicer package built before those scripts
# shipped in it. The copies are generated upstream
# (github.com/mauigna06/slicer-installer, tools/sync-embedded.sh) and vendored
# here together with the scripts themselves, so without this check nothing in
# this tree would notice the two disagreeing -- and the result would be two
# different versions of the same script in one release.
#
# Usage:
#   cmake -DSOURCE_DIR:PATH=<this directory> -P CheckEmbeddedCopies.cmake
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is not set")
endif()

set(problems "")

macro(_report msg)
  list(APPEND problems "${msg}")
endmacro()

#-----------------------------------------------------------------------------
# install.sh embeds each script in a quoted heredoc:
#
#   write_inner_slicer_language() {
#     cat > "$1" <<'SLICER_INNER_SCRIPT_EOF'
#   ...script...
#   SLICER_INNER_SCRIPT_EOF
#   }
#
# so the copy is recovered by taking everything between the heredoc opener and
# the line that closes it. The generator guarantees the delimiter never occurs
# inside a script, which is what makes that unambiguous.
#-----------------------------------------------------------------------------
set(_delimiter "SLICER_INNER_SCRIPT_EOF")

if(NOT EXISTS "${SOURCE_DIR}/install.sh")
  _report("install.sh is missing from ${SOURCE_DIR}")
else()
  file(READ "${SOURCE_DIR}/install.sh" _install_sh)

  foreach(_name IN ITEMS slicer-language slicer-deps)
    string(REPLACE "-" "_" _fn "${_name}")
    set(_opener "write_inner_${_fn}() {\n  cat > \"$1\" <<'${_delimiter}'\n")

    string(FIND "${_install_sh}" "${_opener}" _start)
    if(_start EQUAL -1)
      _report("install.sh carries no embedded copy of ${_name}")
      continue()
    endif()

    string(LENGTH "${_opener}" _opener_length)
    math(EXPR _body_start "${_start} + ${_opener_length}")
    string(SUBSTRING "${_install_sh}" ${_body_start} -1 _rest)

    # The script ends with a newline, so the closing delimiter is the first
    # occurrence of the delimiter at the start of a line after it.
    string(FIND "${_rest}" "${_delimiter}\n" _stop)
    if(_stop EQUAL -1)
      _report("install.sh: the embedded copy of ${_name} is not terminated")
      continue()
    endif()
    string(SUBSTRING "${_rest}" 0 ${_stop} _embedded)

    if(NOT EXISTS "${SOURCE_DIR}/${_name}")
      _report("${_name} is missing from ${SOURCE_DIR}")
      continue()
    endif()
    file(READ "${SOURCE_DIR}/${_name}" _actual)

    if(NOT _embedded STREQUAL _actual)
      _report("install.sh's embedded copy of ${_name} differs from ${_name}")
    endif()
  endforeach()
endif()

#-----------------------------------------------------------------------------
# install.ps1 cannot use a heredoc: a PowerShell single-quoted here-string
# always ends at a line starting with '@, and slicer-language.ps1 contains
# exactly that. It embeds one single-quoted string per line instead:
#
#   $innerLanguagePs1 = @(
#       'line one'
#       'it''s escaped by doubling'
#   ) -join "`n"
#
# Rather than decode that -- ambiguous for a line ending in a quote -- re-encode
# slicer-language.ps1 the same way and compare. Escaping the quotes before
# breaking the lines makes both steps plain whole-string substitutions.
#-----------------------------------------------------------------------------
if(NOT EXISTS "${SOURCE_DIR}/install.ps1")
  _report("install.ps1 is missing from ${SOURCE_DIR}")
elseif(NOT EXISTS "${SOURCE_DIR}/slicer-language.ps1")
  _report("slicer-language.ps1 is missing from ${SOURCE_DIR}")
else()
  file(READ "${SOURCE_DIR}/install.ps1" _install_ps1)
  file(READ "${SOURCE_DIR}/slicer-language.ps1" _actual)

  set(_ps_opener "$innerLanguagePs1 = @(\n")
  set(_ps_closer ") -join \"`n\"\n")

  string(FIND "${_install_ps1}" "${_ps_opener}" _start)
  string(FIND "${_install_ps1}" "${_ps_closer}" _stop)
  if(_start EQUAL -1 OR _stop EQUAL -1)
    _report("install.ps1 carries no embedded copy of slicer-language.ps1")
  else()
    string(LENGTH "${_ps_opener}" _opener_length)
    math(EXPR _body_start "${_start} + ${_opener_length}")
    math(EXPR _body_length "${_stop} - ${_body_start}")
    string(SUBSTRING "${_install_ps1}" ${_body_start} ${_body_length} _embedded)

    # Drop the trailing newline, escape the quotes, then turn every remaining
    # line break into the end of one quoted string and the start of the next.
    string(REGEX REPLACE "\n$" "" _expected "${_actual}")
    string(REPLACE "'" "''" _expected "${_expected}")
    string(REPLACE "\n" "'\n    '" _expected "${_expected}")
    set(_expected "    '${_expected}'\n")

    if(NOT _embedded STREQUAL _expected)
      _report("install.ps1's embedded copy of slicer-language.ps1 differs from slicer-language.ps1")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
if(problems)
  string(REPLACE ";" "\n  " problems "${problems}")
  message(FATAL_ERROR
    "The embedded copies of the inner scripts are out of date:\n"
    "  ${problems}\n\n"
    "These copies are generated. Do not edit them here: make the change in\n"
    "github.com/mauigna06/slicer-installer, run tools/sync-embedded.sh, and\n"
    "copy install.sh, install.ps1 and the inner scripts across together.\n")
endif()

message(STATUS "The embedded copies of the inner scripts are up to date.")
