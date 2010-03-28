#!/bin/bash

#
# A script to easily compare the output of old test results and new.
#
if [ "$1" == "" ]
then
  echo "USAGE:  testCheck.sh <test num>"
else
  cmd="test$1.sh"
  out="test$1.out"
  $cmd
  if cmp -s tmp.out $out
  then
    echo "$cmd OK -- output identical"
  else
    echo "$cmd Problem!" 
    echo
    echo "TEST ======================================================"
    echo
    cat tmp.out
    echo "EXPECTED =================================================="
    echo
    cat $out
    echo
    echo "DIFF ======================================================"
    echo "diff tmp.out $out"
    echo "< = test(new) = tmp.out" 
    echo "> = expected  = $out" 
    echo
    diff tmp.out $out
    echo
  fi
fi
