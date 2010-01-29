#!/bin/sh
# convert Slicer .h files to use three slashes instead of two in doxygen comments
# also remove Description and NAME lines
for x in *.h
do
  echo "working on $x"
  perl -pi -e 's/\/\/ /\/\/\/ /g' $x
  perl -pi -e 's/  \/\/\/ Description:/  \/\/\/ /g' $x
  perl -pi -e 's/\.SECTION Description//g' $x
  perl -pi -e 's/\.NAME//g' $x
done
