

proc EditorTestPaint { {greyFile ""} } {

    if {$greyFile == ""} {
        puts "No input file to paint upon!"
        return 1
    }

    # load the volume
    set logic [$::slicer3::VolumesGUI GetLogic]
    set labelMap 0
    set centered 1
    set loadingOptions [expr $labelMap * 1 + $centered * 2]
    set node [$logic AddArchetypeVolume $greyFile GreyVolume $loadingOptions]
    if {$node == ""} {
        puts "Could not load $greyFile"
        return 1
    }
    # make a lable volume
    set labelNode [$logic CreateLabelVolume $::slicer3::MRMLScene $node "LabelVolume"]

    # make them active
    set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
    if { $selNode == "" } {
        puts "Could not get selection node"
        return 1
    }
    $selNode SetReferenceActiveVolumeID [$node GetID]
    $selNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
    $::slicer3::ApplicationLogic PropagateVolumeSelection

    # set up the paint effect
    catch "itcl::delete object  paintEffect" err
    PaintEffect paintEffect [$::slicer3::SlicesGUI GetFirstSliceGUI]
    paintEffect buildOptions

    # trigger the paint effect
    paintEffect paintAddPoint 10 10
    catch "paintEffect paintApply" err
    if {$err != ""} {
        puts "Error in running paintEffect paintApply:\n$err"
        return 1
    }

    # clean up
    paintEffect tearDownOptions
    catch "itcl::delete object paintEffect" err

    return 0
}



proc runtest { greyFile } {
    set ret [ catch {
         $::slicer3::Application TraceScript "EditorTestPaint $greyFile"
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update 

if {$argc == 0} {
    # when use source ../Slicer3/Modules/Editor/Testing/TestEditorPaint.tcl
    set greyFile ../../../../../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
} else {
    # puts "argv = $argv"
    # for now assume that it's run just through the testing framework with --test-mode --script TestEditorPaint.tcl fixed.nrrd
    set greyFile [lindex $argv 3]
}
# puts "Using label file = $greyFile"
after idle runtest $greyFile
