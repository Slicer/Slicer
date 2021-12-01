#!/bin/bash
set -e
set -x
set -o pipefail
set -o

cp -r $GITHUB_WORKSPACE /usr/src/Slicer

/usr/src/Slicer-build/BuildSlicer.sh
package_filepath=$(head -n1 /usr/src/Slicer-build/Slicer-build/PACKAGE_FILE.txt)
echo "package_filepath [${package_filepath}]"

mv ${package_filepath} $GITHUB_WORKSPACE/

package=$(basename $package_filepath)
echo "package [${package}]"

echo "::set-output name=package::$package"
