# test making as many lists as there are glyph types, each list with a different glyph and colour

# set exit on completion flag to 1 if using it in an automated test with -e source testFiducialGlyph.tcl, as it will have to exit once it's done so as not to hang up testing. 

# if source it into slicer, this probably won't be set yet
if {[info exists ::exitOnCompletion] == 0} {
    set ::exitOnCompletion 0
}

# visibilityFlag int set to 0 if wish to see how fast it works w/o the 3d display widget in the way
proc TestFiducialGlyphs { {visibilityFlag 1} } {
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
    set t1 0
    set t2 0
    

    puts  "List\tTime to add fid\tDelta between adds"
    puts "i\tt\tdt"

    # loop over the glyphs, adding a new list for each one and using that glyph type
    for {set n  [$fidList GetMinimumGlyphType]} {$n <= [$fidList GetMaximumGlyphType]} {incr n} {
        catch "fidNode${n} Delete"
        set fidList${n} [vtkMRMLFiducialListNode fidNode${n}]
        fidNode${n} SetGlyphType $n
        fidNode${n} SetColor 0.2 0.1 1.0
        $sc AddNode fidNode${n}
        if {$visibilityFlag} {
            # Note: May 16/08 w/o setting the visibility to 1 explictly, the 3d widgets don't show up, and this test runs much faster
            fidNode${n} SetVisibility 1
        }
        set str [time "fidNode${n} AddFiducialWithXYZ $r $a $s 0"]        
        set t2 [lindex $str 0]        
        set dt [expr $t2 - $t1]
        set fidText [fidNode${n} GetGlyphTypeAsString $n]
        fidNode${n} SetNthFiducialLabelText 0 $fidText
        puts "$n\t[lindex $str 0]\t$dt"

        set r [expr $r + 10]
        set a [expr $a + 10]
        set s [expr $s + 10]
        set t1 $t2
    }
    #$fidList DisableModifiedEventOff
    #$fidList Modified
    # changes have no one listening for a modified event, trigger a node added, but tcl doesn't expose the two arg invoke event
    #$fidList InvokeEvent 66000 $fidList
}
time "TestFiducialGlyphs"

if {$::exitOnCompletion} {
    exit
}
