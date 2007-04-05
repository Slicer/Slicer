
#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFreeSurferLabelsToBirnLexLabels { label } {
    
    set labelTable "C:/cygwin/home/wjp/slicer3Beta/Slicer3/Modules/QueryAtlas/Tcl/FreeSurferLabels2BirnLexLabels.txt"
    set fp [ open $labelTable r ]

    set newLabel ""
    while { ! [eof $fp ] } {
        gets $fp line
        set tst [ string first $label $line ]
        if  { $tst > 0 } {
            #--- get second term in line
            set newLabel [ lindex $line 1 ]
            #--- get rid of underscores
            regsub -all -- "_" $newLabel " " newLabel
            break
        }
    }
    close $fp
    return $newLabel
}


