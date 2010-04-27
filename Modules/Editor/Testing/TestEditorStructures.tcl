# called after loading a volume, setting up a background


proc EditorTestStructures { backgroundFile } {

    puts "Loading $backgroundFile..."
    # load the volume
    set logic [$::slicer3::VolumesGUI GetLogic]
    set volumeNode [$logic AddArchetypeVolume $backgroundFile "background"]
    if {$volumeNode == ""} {
        puts "Could not load $backgroundFile"
        return 1
    }

    # make it active
    puts "Activating $backgroundFile..."
    set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
    if { $selNode == "" } {
        puts "Could not get selection volumeNode"
        return 1
    }
    $selNode SetReferenceActiveLabelVolumeID [$volumeNode GetID]
    $::slicer3::ApplicationLogic PropagateVolumeSelection


    puts "Entering Editor..."
    $::slicer3::ApplicationGUI SelectModule "Editor"
    update

    #
    # note: to implement GUI testing this code takes advantage
    # of the internal structure of the Editor module, and will need
    # to be updated to track any future changes to that structure
    #
    set singleton $::Editor(singleton)
    array set o [$::Editor($singleton,editHelper) objects]

    puts "Select volume..."
    $o(masterSelector) SetSelected $volumeNode
    update

    puts "Select color node..."
    # reresh object array to get newly created dialog
    array unset o
    array set o [$::Editor($singleton,editHelper) objects]
    #$o(colorDialogCancel) InvokeEvent 10000
    # $o(colorDialogApply) InvokeEvent 10000
    

    return 0
}


set ::thisDebug 0

proc runtest { backgroundFile } {
    set ret [ catch {
         $::slicer3::Application TraceScript "EditorTestStructures $backgroundFile"
    } res]

 if { $ret } {
     puts stderr $res
     if {$::thisDebug} {
       return 1
     } else {
       exit 1
     }
 }
    if {$::thisDebug} {
       return 1
    } else {
       exit 0
    }
}

update 

if {$argc == 0} {
    # when use source ../Slicer3/Modules/Editor/Testing/TestEditorStructures.tcl
    set backgroundFile $::env(Slicer3_HOME)/../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
    set ::thisDebug 1
} else {
    # puts "argv = $argv"
    # for now assume that it's run just through the testing framework with 
    # ./Slicer3 --test-mode --script ../Slicer3/Modules/Editor/Testing/TestEditorStructures.tcl ../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
    set backgroundFile [lindex $argv 4]
}
# puts "Using background file = $backgroundFile"
after idle runtest $backgroundFile
