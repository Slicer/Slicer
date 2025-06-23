#!/bin/bash

COMMIT_INFO="COMP: Script to initiate updating to find_package(Python3)

Now that cmake FindPython3 can be universally
used, prefer to only use that set of variable
names for identifying the python executables.

Intial procesing with manual cleanup by:

ITK/Utilities/Maintenance/FindPython3_ModernizeCMake.sh

This maintenance script can be used to assist with moving from
the deprecated find_package(PythonInterp) and find_package(PythonLib)
to the preferred find_package(Python3) variable names.

------------------------------
NOTE: This script is intended to make it easier
       to identify old find_package(PythonInterp
       uses, but it can not be blindly used.
       each change must be carefully reviewed.
"

# Utility functions
usage() {
cat << EOF
Usage: $0
$COMMIT_INFO
EOF
}

die() {
  echo "$@" 1>&2; exit 1
}

while test $# -gt 0;
do
  opt="$1";
  case "$opt" in
    "-h"|"--help")
      shift;
      help=true
      break;;
    *)
      break;;
  esac
done

if test $help; then
  usage
  exit 1
fi

# Deprecated (since cmake 3.12) variables from the FindPythonInterp
# PYTHONINTERP_FOUND
# PYTHON_EXECUTABLE
# PYTHON_VERSION_STRING
# PYTHON_VERSION_MAJOR
# PYTHON_VERSION_MINOR
# PYTHON_VERSION_PATCH
# Additional deprecated (since cmake 3.12) variables from the FindPythonLibs
# PYTHONLIBS_FOUND
# PYTHON_LIBRARIES
# PYTHON_INCLUDE_PATH
# PYTHON_INCLUDE_DIRS
# PYTHON_DEBUG_LIBRARIES
# PYTHONLIBS_VERSION_STRING
# PYTHON_LIBRARY
# PYTHON_INCLUDE_DIR



cat > /tmp/temp.sed << EOF
s/PYTHONINTERP_FOUND/Python3_Interpreter_FOUND/g
s/PYTHON_EXECUTABLE/Python3_EXECUTABLE/g
s/PYTHON_VERSION_STRING/Python3_VERSION/g
s/PYTHON_VERSION_MAJOR/Python3_VERSION_MAJOR/g
s/PYTHON_VERSION_MINOR/Python3_VERSION_MINOR/g
s/PYTHON_VERSION_PATCH/Python3_VERSION_PATCH/g
s/PYTHONLIBS_FOUND/Python3_Development_FOUND/g
s/PYTHON_LIBRARIES/Python3_LIBRARIES/g
s/PYTHON_INCLUDE_PATH/Python3_XXXXXXX_INCLUDE_PATH_NO_MAPPING_FOR_THIS_PYTHON.h_PATH_FINDER/g
s/PYTHON_INCLUDE_DIRS/Python3_INCLUDE_DIRS/g
s/PYTHON_DEBUG_LIBRARIES/Python3_XXXXX_NO_MAPPING_FOR_THIS_DEBUG_LIBRARIES_FLAG/g
s/PYTHONLIBS_VERSION_STRING/Python3_VERSION/g
s/PYTHON_LIBRARY/Python3_XXXXXX_LIBARY__NO_MAPPING_FOR_THIS_/g
s/PYTHON_INCLUDE_DIR/Python3_XXXXXXX_INCLUDE_DIR_NO_MAPPING_FOR_THIS_PYTHON.h_PATH_FINDER/g
s/find_package *( *PythonInterp *REQUIRED *)/find_package(Python3 COMPONENTS Interpreter REQUIRED)/g
s/find_package *( *PythonInterp *)/find_package(Python3 COMPONENTS Interpreter)/g
s/find_package *( *PythonLibs *)/find_package(Python3 COMPONENTS Development)/g
s/find_package *( *PythonInterp/find_package(Python3 COMPONENTS Interpreter/g
s/find_package *( *PythonLibs/find_package(Python3 COMPONENTS Development/g
s/below_restore_pattern_match_changes_to_original/values/g
s/SIMPLE_Python3_EXECUTABLE/SIMPLE_PYTHON_EXECUTABLE/g
s/ITK_WRAP_Python3_XXXXXX_LIBARY__NO_MAPPING_FOR_THIS_/ITK_WRAP_PYTHON_LIBRARY/g
s/VTK_COMMON_Python3_XXXXXX_LIBARY__NO_MAPPING_FOR_THIS_/VTK_COMMON_PYTHON_LIBRARY/g
s/VTK_COMMON_Python3_XXXXXX_LIBARY__NO_MAPPING_FOR_THIS_/VTK_COMMON_PYTHON_LIBRARY/g
s/Python3_XXXXXX_LIBARY__NO_MAPPING_FOR_THIS_/PYTHON_LIBRARY/g
s/Python3_XXXXXXX_INCLUDE_DIR_NO_MAPPING_FOR_THIS_PYTHON.h_PATH_FINDER/PYTHON_INCLUDE_DIR/g
s/KIT_Python3_LIBRARIES/KIT_PYTHON_LIBRARIES/g
s/Python3_XXXXXXX_INCLUDE_PATH_NO_MAPPING_FOR_THIS_PYTHON.h_PATH_FINDER/PYTHON_INCLUDE_PATH/g
s/Python3_XXXXX_NO_MAPPING_FOR_THIS_DEBUG_LIBRARIES_FLAG/PYTHON_DEBUG_LIBRARIES/g
EOF

if [ $(uname) == "Darwin" ] ; then
  SEDBIN=gsed
else
  SEDBIN=sed
fi

# Do the replacement
find . -type f \( -name 'CMakeLists.txt' -o -name '*\.cmake*' \) -exec ${SEDBIN} -i''  -f /tmp/temp.sed {} \;

cat > /tmp/git_message.txt << EOF
${COMMIT_INFO}
EOF
