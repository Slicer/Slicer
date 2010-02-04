# called after loading a volume, setting up a label map

proc EditorTestModelMaker { {labelFile ""} } {

    if {$labelFile == ""} {
        puts "No input label file to make a model from!"
        return 1
    }

    # load the volume
    set logic [$::slicer3::VolumesGUI GetLogic]
    set labelMap 1
    set centered 1
    set loadingOptions [expr $labelMap * 1 + $centered * 2]
    set node [$logic AddArchetypeVolume $labelFile LabelVolume $loadingOptions]
    if {$node == ""} {
        puts "Could not load $labelFile"
        return 1
    }
    # make it active
    set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
    if { $selNode == "" } {
        puts "Could not get selection node"
        return 1
    }
    $selNode SetReferenceActiveLabelVolumeID [$node GetID]
    $::slicer3::ApplicationLogic PropagateVolumeSelection

    # set up the model maker effect
    catch "itcl::delete object  modelEffect" err
    MakeModelEffect modelEffect [$::slicer3::SlicesGUI GetFirstSliceGUI]
    modelEffect buildOptions

    # trigger the model make effect
    catch "modelEffect apply" err
    if {$err != ""} {
        puts "Error in running modelEffect apply:\n$err"
        return 1
    }

    set waiting 1
    set needToWait { "Idle" "Scheduled" "Running" }
    while {$waiting == 1} {
        puts "Waiting for task..."
        set waiting 0
        set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLCommandLineModuleNode"]
        puts "Found $nNodes command line module nodes"
        for {set i 0} {$i < $nNodes} {incr i} {
            set clmNode [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLCommandLineModuleNode"]
            set moduleTitle [$clmNode GetModuleTitle]
            # puts "\tchecking node $i: $moduleTitle..."
            if {$moduleTitle == "Model Maker"} {
                set status [$clmNode GetStatusString]
                puts "Checking node $i: $moduleTitle = $status"
                if { [lsearch $needToWait $status] != -1 } {
                    set waiting 1
                }
            }
        }
        after 250
    }

    # clean up
    modelEffect tearDownOptions
    catch "itcl::delete object  modelEffect" err

    return 0
}



proc runtest { labelFile } {
    set ret [ catch {
         $::slicer3::Application TraceScript "EditorTestModelMaker $labelFile"
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update 

if {$argc == 0} {
    # when use source ../Slicer3/Modules/Editor/Testing/TestEditorModelMaker.tcl
    set labelFile ../../../../../Slicer3/Libs/MRML/Testing/TestData/helixMask.nrrd
} else {
    # puts "argv = $argv"
    # for now assume that it's run just through the testing framework with --test-mode --script TestEditorModelMaker.tcl helixMask.nrrd
    set labelFile [lindex $argv 3]
}
# puts "Using label file = $labelFile"
after idle runtest $labelFile
