#!/bin/sh
#echo "Working on $argv"
# perl -pi -e 's/\/\/ /\/\/\/ /g' $0
# perl -pi -e 's/  \/\/\/ Description:/  \/\/\/ /g' $0
for x in *.h
do
  echo "working on $x"
  perl -pi -e 's/\/\/ /\/\/\/ /g' $x
  perl -pi -e 's/  \/\/\/ Description:/  \/\/\/ /g' $x
  perl -pi -e 's/\.SECTION Description//g' $x
  perl -pi -e 's/\.NAME//g' $x
done
