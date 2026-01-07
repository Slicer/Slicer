#!/bin/bash
# Clazy can automatically fix many Qt5-to-Qt6 porting issues using Fix‑It style
# rewrites—but with some important caveats. Here’s how it works:
#
# Clazy includes several Qt6‑porting checks that provide automatic fix-it suggestions, including:
#   •  qt6-deprecated-api-fixes
#   •  qt6-header-fixes
#   •  qt6-qhash-signature
#   •  qt6-fwd-fixes
#   •  missing-qobject-macro
# ￼
# These checks can identify deprecated APIs and even offer automatic replacements.
#   Clazy generates .clazy.yaml files alongside your source files, containing replacement instructions.
#  •  Use clang-apply-replacements <path> to apply changes in bulk.
#  •  Conflicts or ambiguous fix‑its are reported and halted.
#
# ⚠️ Limitations and Notes
#  •  Not all Qt6 migration issues can be auto-fixed. Some require manual review.
#  •  Fix‑its may conflict if multiple rules apply to the same code line. Handle with caution.
#  •  After applying fixes, your code may only compile under Qt6, not with Qt5.  Use git diff tools
#     to add pre-processor alternate compilations paths to support both Qt5 and Qt6
#     #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#       menuOption.tabWidth = 0;
#     #endif

SCRIPT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
export SRC_DIR="$(dirname $(dirname ${SCRIPT_DIR}))"
export BLD_DIR=$(dirname ${SRC_DIR})/Qt5clazySlicer-build
export INNER_BLD_DIR=${BLD_DIR}/Slicer-build


usage() {
  echo "Usage:  $0 <checks_to_run> <cleanbuild|build>"
  echo "  where can be one of qt6-deprecated-api-fixes,qt6-header-fixes,qt6-qhash-signature,qt6-fwd-fixes,missing-qobject-macro"
  echo "For example:"
  echo "  $0 \"qt6-deprecated-api-fixes,qt6-header-fixes,qt6-qhash-signature,qt6-fwd-fixes,missing-qobject-macro\" cleanbuild"
  echo "  $0 \"qt6-deprecated-api-fixes\" cleanbuild"
  echo "  $0 \"qt6-header-fixes\" cleanbuild"
  echo "  $0 \"qt6-qhash-signature\" cleanbuild"
  echo "  $0 \"qt6-fwd-fixes\" cleanbuild"
  echo "  $0 \"missing-qobject-macro\" cleanbuild"

  exit -1
}


if [ $# -ne 2 ];then
  usage
fi
export CLAZY_CHECKS="$1"
export CLAZY_EXPORT_FIXES=ON
export CMAKE_EXPORT_COMPILE_COMMANDS=ON
export CMAKE_GENERATOR=Ninja

echo "
SRC_DIR=${SRC_DIR}
BLD_DIR=${BLD_DIR}
INNER_BLD_DIR=${INNER_BLD_DIR}
CLAZY_CHECKS=${CLAZY_CHECKS}
CLAZY_EXPORT_FIXES=${CLAZY_EXPORT_FIXES}
CMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
CMAKE_GENERATOR=${CMAKE_GENERATOR}
"


# use clazy as your compiler
#
if [ ! -f ${BLD_DIR}/CMakeCache.txt ] ;then
  cmake -GNinja \
        -S ${SRC_DIR} \
        -B ${BLD_DIR} \
        -DCMAKE_BUILD_TYPE:BOOL=Release \
        -DCMAKE_CXX_COMPILER=clazy
   cmake --build ${BLD_DIR}
fi
cd ${INNER_BLD_DIR}
buildmode="$2"
case "$buildmode" in
  "cleanbuild")
    echo "Starting clean build in ${INNER_BLD_DIR} ..."
    cmake --build ${INNER_BLD_DIR} --target clean
    cmake .
    cmake --build ${INNER_BLD_DIR}
    ;;
  "build")
    echo "Starting re-build in ${INNER_BLD_DIR} ..."
    cmake .
    cmake --build ${INNER_BLD_DIR}
    ;;
  *)
    echo "Unknown command: '$buildmode'"
    usage
    ;;
esac


echo "Now consider applying all found clazy regresssions"
echo "for clazy_yaml in \$(find ${INNER_BLD_DIR} -name \"*.clazy.yaml\"); do"
echo "  ${SCRIPT_DIR}/apply_one_clazy_yaml_dir.sh \${clazy_yaml}"
echo "done"
