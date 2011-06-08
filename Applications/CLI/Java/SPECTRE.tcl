#!/bin/sh
# the next line restarts using tclsh \
#    exec tclsh "$0" "$@"

set CLP "SPECTRE"

# set the path to the MIPAV install directory
# set the full path to the SPECTRE plugins directory
if {[info exists ::env(SLICER_HOME)] == 1} {
    set pathToMipav "$::env(SLICER_HOME)/../Slicer3/Applications/CLI/Java/SPECTRE/mipav"
    set pathToSPECTREPlugins "$::env(SLICER_HOME)/lib/Slicer3/Plugins/SPECTRE_2010-04-01.jar"
} else {
    set pathToMipav "/projects/birn/nicole/Slicer3VTKHead/Slicer3/Applications/CLI/Java/SPECTRE/mipav"
    set pathToSPECTREPlugins "/projects/birn/nicole/Slicer3FC10/Slicer3/Applications/CLI/Java/SPECTRE_2010-04-01.jar"
}
proc glob-r {{dir .} args} {
     set res {}
     foreach i [lsort [glob -nocomplain -dir $dir *]] {
       if {[file isdirectory $i]} {
         eval [list lappend res] [eval [linsert $args 0 glob-r $i]]
       } else {
         if {[llength $args]} {
           foreach arg $args {
             if {[string match $arg $i]} {
               lappend res $i
               break
             }
           }
         } else {
             lappend res $i
         }
       }
     }
     return $res
 } ;# JH

set jarList [glob-r $pathToMipav *.jar]
# add the top level mipav dir
lappend jarList $pathToMipav
# now add the path to the spectre edu classes
lappend jarList $pathToSPECTREPlugins

if {$argv != "--xml"} {
#  puts "Jar list = $jarList"
}
foreach j $jarList {
  lappend cpath $j
}
switch $tcl_platform(os) {
    "SunOS" -
    "Linux" -
    "Darwin" { set joinChar ":" }
    default { set joinChar ";" }
}
set cpath [join $cpath $joinChar]

if {$argv != "--xml"} {
 puts "Path to MIPAV $pathToMipav"
 puts "Path to SPECTRE plugins $pathToSPECTREPlugins"
 puts "cpath = $cpath"
 puts "argv = \"$argv\""
}

set ret [catch "exec  \"${pathToMipav}/jre/bin/java\" -Xms400m -Xmx700m -classpath \"$cpath\"  edu.jhu.ece.iacl.jist.cli.run edu.jhu.ece.iacl.plugins.segmentation.skull_strip.MedicAlgorithmSPECTRE $argv 2> fileError.txt" res]
puts $res
# exit $ret
if {$argv != "--xml"} {
puts "Return value = $ret"
}
#exit 0
exit $ret
