#!/bin/sh
# \
exec tclkitsh "$0" ${1+"$@"}

#
# make-launchers.tcl
#

set __comment {

    This script creates the stand-alone executables
    (starpacks) to startup slicer from an arbitrary directory.

    See www.equi4.com for info about starkits and starpacks.

    Note:
    * you'll need to have tclkitsh in your path somewhere
    * the tclkitsh you use to generate the launcher cannot be the same one as the 
      -runtime argument (cannot be circular).
    * this can be run on any platform to generate launchers for all the platforms
    * the 32 bit linux launcher can be used for both 32 and 64 bit installs
      (probably the same for win32 and win64)
    * solaris has been dropped from this list for now, but presumably solaris,
      irix, aix, and other 'old' systems can be added if needed.
}

cd [file dirname [info script]]

puts "making linux..."
exec tclkitsh tclkits/sdx.kit wrap Slicer3 -runtime tclkits/tclkit-linux-x86
file rename -force Slicer3 Slicer3Launchers/Slicer3-linux-x86

puts "making win32..."
exec tclkitsh tclkits/sdx.kit wrap Slicer3 -runtime tclkits/tclkit-win32.exe
file rename -force Slicer3 Slicer3Launchers/Slicer3-win32.exe

puts "making darwin-ppc..."
exec tclkitsh tclkits/sdx.kit wrap Slicer3 -runtime tclkits/tclkit-darwin-ppc
file rename -force Slicer3 Slicer3Launchers/Slicer3-darwin-ppc

puts "making darwin-x86..."
exec tclkitsh tclkits/sdx.kit wrap Slicer3 -runtime tclkits/tclkit-darwin-x86
file rename -force Slicer3 Slicer3Launchers/Slicer3-darwin-x86
