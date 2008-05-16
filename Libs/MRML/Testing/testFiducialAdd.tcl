# test adding 200 fiducials to a new list

# set exit on completion flag to 1 if using it in an automated test with -e source testFiducialAdd.tcl, as it will have to exit once it's done so as not to hang up testing. 

# if source it into slicer, this probably won't be set yet
if {[info exists ::exitOnCompletion] == 0} {
    set ::exitOnCompletion 0
}

# renameFlag int set to 1 if want to test time renaming
# visibilityFlag int set to 0 if wish to see how fast it works w/o the 3d display widget in the way
# numToAdd int number of fiducials to add
proc TestFiducialAdd { {renameFlag 1} {visibilityFlag 1} {numToAdd 20} } {
    # check to see if we were sourced into slicer
    if {[info exists ::slicer3::MRMLScene] == 1} {
        set fidLogic [ $::slicer3::FiducialsGUI GetLogic ]
        set fidList [$fidLogic AddFiducialList]
    } else {
        # can get here via automated add_test -f script.tcl
        # but: this doesn't engage the fiducial list widget and is misleadingly fast
        catch "sc Delete"
        vtkMRMLScene sc
        set fidList [vtkMRMLFiducialListNode fidNode]
        sc AddNode $fidList
    }

    if {$visibilityFlag} {
        # Note: May 16/08 w/o setting the visibility to 1 explictly, the 3d widgets don't show up, and this test runs much faster
        $fidList SetVisibility 1
    }
    $fidList DisableModifiedEventOn
    set r 0
    set a 0
    set s 0
    set t1 0
    set t2 0
    set t3 0
    set t4 0

    if {$renameFlag} {
        puts "Index\tTime to add fid\tDelta between adds\tTime to rename fid\tDelta between renames"
        puts "i\tt\tdt\tt\tdt"
    } else {
        puts  "Index\tTime to add fid\tDelta between adds"
        puts "i\tt\tdt"
    }
    while { $r < $numToAdd} {
        set str [time "$fidList AddFiducialWithXYZ $r $a $s 0"]        
        set t2 [lindex $str 0]        
        set dt [expr $t2 - $t1]
        if {$renameFlag} {
            set fidIndex [expr [$fidList GetNumberOfFiducials] - 1]
            set str2 [time "$fidList SetNthFiducialLabelText $fidIndex $r"]
            set t4 [lindex $str2 0]
            set dt2 [expr $t4 - $t3]
            set t3 $t4
            puts "$r\t[lindex $str 0]\t$dt\t[lindex $str2 0]\t$dt2"
        } else {
            puts "$r\t[lindex $str 0]\t$dt"
        }
        incr r
        incr a
        incr s
        set t1 $t2
    }
    $fidList DisableModifiedEventOff
    $fidList Modified
}
time "TestFiducialAdd"

if {$::exitOnCompletion} {
    exit
}
