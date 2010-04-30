#!/bin/sh
#exec tclsh "$0" "$@"
puts "# of arguments $argc ; commandline = $argv"
if { $argc != 4 } {
    puts "TestFCOB.tcl progdir baselinedatadir outputdatadir imagefile"
    exit 1
}

proc compare { a b } {
    set diff [ expr { abs( [ expr { $a - $b } ] ) } ]
    if { $diff > 0.0001 } {
        return 0
    } else {
        return 1
    }
    
}

# test program for FindCenterOfBrain
set progdir [lindex $argv 0]
set baselinedata [lindex $argv 1]
set outputdata [lindex $argv 2]
set imagefile [lindex $argv 3]

set command "$progdir/FindCenterOfBrain --inputVolume $imagefile"
set command "$command --generateDebugImages"
set command "$command --debugTrimmedImage $outputdata/trimmed.nii.gz"
set command "$command --debugClippedImageMask $outputdata/clipped.nii.gz"
set command "$command --debugAfterGridComputationsForegroundImage $outputdata/aftergridcomp.nii.gz"
set command "$command --debugGridImage $outputdata/grid.nii.gz"
set command "$command --debugDistanceImage $outputdata/distance.nii.gz"

puts "running $command"

set fcob [open "| $command " ]

while { [gets $fcob line] >= 0 } {
    puts "Line = $line"
    if { [ string match "*Center Of Brain*" $line ] } {
        puts "CENTER OF BRAIN SEEN Line = $line"
        set start [ string first : $line  ]
        set start [ expr $start + 2 ]
        set center [ string range $line $start end ]
        puts "CENTER OF BRAIN : $center"
        set listlength [ llength $center ]
        puts "LENGTH: $listlength"
        set x [lindex $center 0 ]
        puts "x = $x"
        set y [lindex $center 1 ]
        puts "y = $y"
        set z [lindex $center 2 ]
        puts "z = $z"
        if { [ compare $x 97.7757 ] && [ compare $y -89.2801 ] && [ compare $z -98.4934 ] } {
            puts "consistent computation of center of brain"
            break
        } else {
            exit 1
        }
    }
}

# test intermediate files
set names [ list trimmed grid clipped aftergridcomp ]
for { set i 0 } { $i < [llength $names] } { incr i } {

    set current [ lindex $names $i ]

    set compare_result \
        [ catch { exec ImageCompare $baselinedata/FCOB-$current.nii.gz \
                      $outputdata/$current.nii.gz } result ]
    puts $result
    if { $compare_result != 0 } {
        puts  "$baselinedata/FCOB-$current.nii.gz and $outputdata/$current.nii.gz differ"
        exit $compare_result
    } else {
        puts "$baselinedata/FCOB-$current.nii.gz and $outputdata/$current.nii.gz match"
    }
    
}
exit 0

# set compare_result \
# [ catch { exec ImageCompare $baselinedata/FCOB-trimmed.nii.gz \
#               $outputdata/trimmed.nii.gz } result ]
# puts $result
# if { $compare_result != 0 } {
#     puts 
#     exit $compare_result
# } else {
#     puts "$baselinedata/FCOB-trimmed.nii.gz and $outputdata/trimmed.nii.gz differ match"
# }
