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
    set edit $::Editor($singleton,editBox)
    set helper $::Editor($singleton,editHelper)
    array set o [$helper objects]

    puts "Select volume..."
    $o(masterSelector) SetSelected $volumeNode
    update

    puts "Select color node..."
    # reresh object array to get newly created dialog
    array unset o
    array set o [$::Editor($singleton,editHelper) objects]
    $o(colorDialogApply) InvokeEvent 10000
    update

    #
    # create two structure volumes
    # - expand frame
    # - create label maps 5 and 7
    # 
    $o(structuresFrame) ExpandFrame
    update
    $o(addStructureButton) InvokeEvent 10000
    update
    foreach l {5 7} threshold { {0 100} {100 200} } {
      puts "Creating $l"
      array set v [$::Editor($singleton,editHelper) variables]
      array set co [::HelperBox::$v(_colorBox) objects]
      [$co(colors) GetWidget] SelectSingleRow $l
      update

      $edit selectEffect Threshold
      set thresh [lindex [::itcl::find objects -class ThresholdEffect] 0]
      array set to [$thresh getObjects]
      foreach {lo hi} $threshold {}
      $to(range) SetRange $lo $hi
      update
      $thresh apply
      $edit selectEffect DefaultTool
    }

    # merge and build, then delete structures, re-split, display two volumes
    $helper merge
    update
    $helper build
    update
    $helper deleteStructures "noconfirm"
    update


    # wait for the build to complete...

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

    set i 20
    while { [$::slicer3::ApplicationLogic GetReadDataQueueSize] && $i > 0 } {
        puts "$i Waiting for data to be read...queue size = [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
        incr i
        update
        after 1000
    }

    if { $i <= 0 } {
      puts "Error: timeout waiting for data to be read"
      return 1
    }

    
    puts "Model is finished..."

    puts "Test passed."
    
    after 1000

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
    set backgroundFile $::env(Slicer_HOME)/../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
    set ::thisDebug 1
} else {
    # puts "argv = $argv"
    # for now assume that it's run just through the testing framework with 
    # ./Slicer3 --test-mode --script ../Slicer3/Modules/Editor/Testing/TestEditorStructures.tcl ../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
    set backgroundFile [lindex $argv 4]
    if { $backgroundFile == "" } {
      # account for the difference between ctest invocation and command line invocation
      set backgroundFile [lindex $argv 3]
    }
}
puts $argv
puts "Using background file = $backgroundFile"
after idle runtest $backgroundFile
