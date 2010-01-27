# test out all the different glyphs by making a new list for each glyph type

proc TestFiducialsGlyphs { } {
    # check to see if we were sourced into slicer
    if {[info exists ::slicer3::MRMLScene] == 1} {
        set fidLogic [ $::slicer3::FiducialsGUI GetLogic ]
        set fidList [$fidLogic AddFiducialList]
        set sc $::slicer3::MRMLScene
    } else {
        # can get here via automated add_test -f script.tcl
        # but: this doesn't engage the fiducial list widget and is misleadingly fast
        catch "sc Delete"
        set sc [vtkMRMLScene scn]
        set fidList [vtkMRMLFiducialListNode fidNode]
    }

    set r -50
    set a -50
    set s -50

    set selFlag 0
    set lockFlag 1
    # loop over the glyphs, adding a new list for each one and using that glyph type
    for {set n  [$fidList GetMinimumGlyphType]} {$n <= [$fidList GetMaximumGlyphType]} {incr n} {
        catch "fidNode${n} Delete"
        set fidList${n} [vtkMRMLFiducialListNode fidNode${n}]
        fidNode${n} SetGlyphType $n
        fidNode${n} SetColor 0.2 0.1 1.0
        fidNode${n} SetSelectedColor 1.0 0.1 0.2
        fidNode${n} SetLocked $lockFlag
        # flip the list locked state
        if {$lockFlag} {
            set lockFlag 0
        } else {
            set lockFlag 1
        }
        $sc AddNode fidNode${n}
        fidNode${n} SetVisibility 1

        fidNode${n} AddFiducialWithXYZ $r $a $s $selFlag
        # flip selected state
        if {$selFlag} {
            set selFlag 0
        } else {
            set selFlag 1
        }

        set fidText [fidNode${n} GetGlyphTypeAsString $n]
        fidNode${n} SetNthFiducialLabelText 0 $fidText

        set r [expr $r + 10]
        set a [expr $a + 10]
        set s [expr $s + 10]
    }

    # clean up
    for {set n  [$fidList GetMinimumGlyphType]} {$n <= [$fidList GetMaximumGlyphType]} {incr n} {
         $sc RemoveNode fidNode${n}
         fidNode${n} Delete
     }
    $fidList Delete
    return 0
}

proc runtest {} {
    set ret [ catch {
         $::slicer3::Application TraceScript TestFiducialsGlyphs
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update 
after idle runtest
