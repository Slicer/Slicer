#!/bin/sh
# the next line restarts using tclsh \
#    exec tclsh "$0" "$@"

set CLP "SPECTRE"

# add the mipav classes
#set pathToMipavPlugins "/cygdrive/c/Program\ Files/mipav"
set pathToMipavPlugins "c:/Program Files/mipav"
if {$argv != "--xml"} {
puts "path to mip av plugins $pathToMipavPlugins"
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

set jarList [glob-r $pathToMipavPlugins *.jar]
# add the top level mipav dir
lappend jarList {c:/Program Files/mipav}
# now add the path to the spectre edu classes, this will move to Applications/CLI/Java/SPECTRE
lappend jarList {c:/Documents and Settings/nicole/mipav/plugins}
if {$argv != "--xml"} {
  puts "Jar list = $jarList"
}
foreach j $jarList {
  lappend cpath $j
}
set joinChar ";"
set cpath [join $cpath $joinChar]

if {$argv != "--xml"} {
 puts "cpath = $cpath"
 puts "argv = \"$argv\""
}

if {[info exists ::env(CLASSPATH)] != 0} {
   if {$argv != "--xml"} {
      puts "Current classpath = $::env(CLASSPATH)"
   }
}

set ret [catch "exec java -Xmx700m -classpath \"$cpath\"  edu.jhu.ece.iacl.jist.cli.run edu.jhu.ece.iacl.plugins.segmentation.skull_strip.MedicAlgorithmSPECTRE2009 $argv 2> fileError.txt" res]
puts $res
# exit $ret
if {$argv != "--xml"} {
puts "Return value = $ret"
}
#exit 0
exit $ret
