#!/bin/bash
#
#
#
SLICER_SRC=/localcache/Users/johnsonhj/src/Slicer
CHECK=misc-const-correctness
cd $SLICER_SRC
find ${SLICER_SRC}  ${SLICER_SRC}-clang19/Slicer-build -name '*.cpp' -o -name "*.cxx" \
  | xargs -P 42 -n1 /usr/bin/clang-tidy-19 \
                            -p=${SLICER_SRC}-clang19/Slicer-build \
                            --fix \
                            --checks=${CHECK}  \
                            -header-filter=".*" \
                            --header-filter=".*"


cat > /tmp/${CHECK}.msg << EOF
COMP: Prefer explicit const designation

Add const to variables that do not change.

Detect local variables which could be declared as const but are not. Declaring
variables as const is recommended by many coding guidelines, such
as: ES.25 from the C++ Core Guidelines.

EOF
