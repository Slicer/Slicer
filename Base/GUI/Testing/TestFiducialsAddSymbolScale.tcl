# test adding fiducials to a new list and changing the glyph symbol and scale while still adding more fiducials

# numToAdd int number of fiducials to add
proc TestFiducialAddSymbolScale { {numToAdd 30} } {
    # check to see if we were sourced into slicer
    if {[info exists ::slicer3::MRMLScene] == 1} {
        set fidLogic [ $::slicer3::FiducialsGUI GetLogic ]
        set fidList [$fidLogic AddFiducialList]
    } else {
        # can get here via automated add_test -f script.tcl
        # but: this doesn't engage the fiducial list widget and is misleadingly fast
        catch "sc Delete"
        vtkMRMLScene sc
        set fidList [vtkMRMLFiducialListNode New]
        sc AddNode $fidList
    }

    $fidList SetVisibility 1

#    $fidList DisableModifiedEventOn
    set r 0
    set a 0
    set s 0
    set t1 0
    set t2 0

    puts "Index\tTime to add fid\tDelta between adds"
    puts "i\tt\tdt"

    set halfAdded [expr $numToAdd / 2]
    set glyphChanged 0
    set glyphScale [$fidList GetSymbolScale]
    while { $r < $numToAdd} {
        if { $r > $halfAdded && !$glyphChanged } {
            # reset the glyph type
            puts "Resetting glyph type to [$fidList GetGlyphTypeAsString 13]"
            $fidList SetGlyphType 13
            incr glyphChanged
        }
        set str [time "$fidList AddFiducialWithXYZ $r $a $s 0"]        
        set t2 [lindex $str 0]        
        set dt [expr $t2 - $t1]
        set fidIndex [expr [$fidList GetNumberOfFiducials] - 1]
        $fidList SetNthFiducialLabelText $fidIndex $r
        puts "$r\t[lindex $str 0]\t$dt"
        $fidList SetSymbolScale $glyphScale
        incr r
        set a [expr $a + 1]
        incr s
        set t1 $t2
        set glyphScale [expr $glyphScale + 1]
        # move it a little
        set a [expr $a + (2 * $glyphScale)]
        $fidList SetNthFiducialXYZ $fidIndex $r $a $s
    }
 #   $fidList DisableModifiedEventOff
 #   $fidList Modified
    puts "Testing adding $numToAdd fiducials okay"

    # clean up
    if {[info exists ::slicer3::MRMLScene] == 1} {
        $::slicer3::MRMLScene RemoveNode $fidList
        $fidList Delete
    } else {
        sc RemoveNode $fidList
        $fidList Delete
        sc Delete
    }
    return 0
}

proc runtest {} {
    set ret [ catch {
         $::slicer3::Application TraceScript TestFiducialAddSymbolScale
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update 
after idle runtest



