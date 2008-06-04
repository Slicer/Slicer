#!/bin/sh

#
# This is a simple example of a command line module written as a shell script.
#
# In this case, two other command line modules are used in sequence to implement
# a multi-step workflow.
#

if [ "x$1" = "x--xml" ]
then
  echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
  echo "<executable>"
    echo "<category> Demonstration</category>"
    echo "<title> Hello World Shell Chain </title>"
    echo "<description> Example chain written in sh </description>"
    echo "<version> 1.0</version>"
    echo "<documentation-url></documentation-url>"
    echo "<license></license>"
    echo "<contributor> Steve Pieper, Ph.D. </contributor>"
    echo "<acknowledgements> This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. </acknowledgements>"
   echo "<parameters>"
      echo "<label>Input/Output</label>"
      echo "<description>Input/output parameters</description>"
      echo "<image>"
        echo "<name>InputVolume</name>"
        echo "<label>Input Volume</label>"
        echo "<channel>input</channel>"
        echo "<index>0</index>"
        echo "<default>None</default>"
        echo "<description>Input volume</description>"
      echo "</image>"
      echo "<image>"
        echo "<name>OutputVolume</name>"
        echo "<label>Output Volume</label>"
        echo "<channel>output</channel>"
        echo "<index>1</index>"
        echo "<default>None</default>"
        echo "<description>Output filtered</description>"
      echo "</image>"
  echo "</parameters>"
  echo "</executable>"
  exit 0
fi

VOL1=${1}
VOL2=${2}
VOLTEMP=${TMPDIR}/temp.nrrd

./Slicer3 --launch \
  ./lib/Slicer3/Plugins/CurvatureAnisotropicDiffusion \
      $VOL1 $VOLTEMP \
      --conductance 1 --timeStep 0.0625 --iterations 1

./Slicer3 --launch \
  ./lib/Slicer3/Plugins/GradientAnisotropicDiffusion \
    $VOLTEMP $VOL2 \
    --conductance 1 --timeStep 0.0625 --iterations 1
