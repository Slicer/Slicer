#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"


#
# slicerput - sp 2005-09-23 
# - communicates with slicerd
# - reads nrrd streams from stdin and puts them into slicer
#

array set nrrd_to_vtk_types { char 2 "unsigned char" 3 short 4 ushort 5 int 6 uint 7 float 10 double 11 }

set name [lindex $argv 0]
if { $name == "" } {
    set name ""
}


gets stdin magic
if { ! [string match "NRRD*" $magic] } {
    puts stderr "bad magic number $magic"
    exit -1
}

set dimensions "none"
set space_origin "none"
set space_directions "none"

# components: hard coded. Set to be 1.
set components 1
set scalar_type $nrrd_to_vtk_types(short)
set measurement_frame "none"
set kinds "none"
set isTensor "?"

while { [gets stdin line] > 0 } {
    switch -glob -- $line {
        "type:*" {
            set scalar_type $nrrd_to_vtk_types([lrange $line 1 end])
        }
        "dimension:*" {
            if { [lindex $line 1] != 3 } {
                puts stderr "only 3 dimensional images supported"
            }
        }
        "sizes:*" {
            set dimensions [lrange $line 1 end]
        }
        "space directions:*" {
            set space_directions [lrange $line 2 end]
        }
        "kinds:*" {
            set kinds [lrange $line 1 end]
        }
        "space origin:*" {
            set space_origin [lrange $line 2 end]
        }
        "measurement frame:*" {
            set measurement_frame [lrange $line 2 end]
        }
        "space:*" {
            set space [lindex $line 1]
        }
    }
}

if { [lindex $kinds 0] == "3D-masked-symmetric-matrix" } {
    # tensor
    set isTensor 1
    #set space_directions [lrange $space_directions 1 end]
    #puts stderr "This is a tensor! space_directions $space_directions"
} else {
    # assume scalar  
    set isTensor 0
}

puts stderr "put"
puts stderr "image $name"
puts stderr "space $space"
puts stderr "dimensions $dimensions"
puts stderr "space_origin $space_origin"
puts stderr "space_directions $space_directions"
puts stderr "kinds $kinds"
puts stderr "components $components"
puts stderr "scalar_type $scalar_type"

if {!($space eq "left-posterior-superior" || \
    $space eq "right-anterior-superior")} {
        puts stderr "\nSpace \"$space\" is not supported."
        puts stderr "Please choose a volume that is in RAS or LPS.\n"
        return
}

set sock [socket localhost 18943]
puts $sock "put"
puts $sock "image $name"
puts $sock "space $space"
puts $sock "dimensions $dimensions"
puts $sock "space_origin $space_origin"
puts $sock "space_directions $space_directions"
puts $sock "kinds $kinds"
puts $sock "components $components"
puts $sock "scalar_type $scalar_type"

if {$isTensor} {
  puts stderr "measurement_frame: $measurement_frame"
  puts $sock "measurement_frame $measurement_frame"
}
    
flush $sock

fconfigure stdin -translation binary -encoding binary
fconfigure $sock -translation binary -encoding binary
set imagedata [read -nonewline stdin]
puts "This is slicerput on stdout: read [string length $imagedata] bytes"
puts -nonewline $sock $imagedata
flush $sock
close $sock

exit 0
