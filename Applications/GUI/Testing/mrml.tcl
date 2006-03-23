# Load the KWWidgets package

package require kwwidgets

# Process some command-line arguments
# The --test option here is used to run this example as a non-interactive test
# for software quality purposes. Ignore this feature in your own application.

set option_test [expr [lsearch -exact $argv "--test"] == -1 ? 0 : 1]

# Create the application
# If --test was provided, ignore all registry settings, and exit silently
# Restore the settings that have been saved to the registry, like
# the geometry of the user interface so far.

set app [vtkKWApplication New]
$app SetName "MRML Scene Example"
if {$option_test} {
  $app SetRegistryLevel 0
  $app PromptBeforeExitOff
}
$app RestoreApplicationSettingsFromRegistry

# Set a help link. Can be a remote link (URL), or a local file

$app SetHelpDialogStartingPage "http://www.kwwidgets.org"

# Add a window
# Set 'SupportHelp' to automatically add a menu entry for the help link

set win [vtkKWWindowBase New]
$win SupportHelpOn
$app AddWindow $win
$win Create


# create the mrml scene
set sc [vtkMRMLScene New]

set pb [vtkKWPushButton New]
$pb SetParent $win
$pb Create
$pb SetText "Undo"
$pb SetAnchorToWest
pack [$pb GetWidgetName] -side top -anchor nw -expand false -fill x -padx 2 -pady 2
$pb SetBalloonHelpString \
"Undo manipulations of mrml scene."
$pb SetCommand "" "$sc Undo"

# Add a button for each mrml node type

# set nodes [info command vtkMRML*Node]
set nodes "vtkMRMLVolumeNode vtkMRMLModelNode"

foreach n $nodes {

  set nn [$n New]
  $sc RegisterNodeClass $nn

  set pb [vtkKWPushButton New]
  $pb SetParent $win
  $pb Create
  $pb SetText "$n"
  $pb SetAnchorToWest
  pack [$pb GetWidgetName] -side top -anchor nw -expand false -fill x -padx 2 -pady 2
  $pb SetBalloonHelpString \
    "Add a $n to the current mrml scene."
  $pb SetCommand "" "mrmlAddNode $sc $n"

}

proc mrmlAddNode {sc nodetype} {
    puts "adding a $nodetype node to the scene $sc"

    set n [$nodetype New]
    $n SetName Node-[clock seconds]
    $sc SaveStateForUndo
    $sc AddNode $n
}



# -----------------------------------------------------------------------

# Create a multi-column list

set mcl1 [vtkKWMultiColumnList New]
$mcl1 SetParent $win
$mcl1 Create
$mcl1 SetBalloonHelpString \
"A simple multicolumn list. Columns can be resized moved and sorted.\
Double-click on some entries to edit them."
$mcl1 MovableColumnsOn
$mcl1 SetWidth 0
$mcl1 SetPotentialCellColorsChangedCommand \
    $mcl1 "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
$mcl1 SetColumnSortedCommand \
    $mcl1 "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

# Add the columns make some of them editable

set col_index [$mcl1 AddColumn "Node Type"] 

set col_index [$mcl1 AddColumn "Name"] 
$mcl1 SetColumnAlignmentToCenter $col_index
$mcl1 ColumnEditableOn $col_index


pack [$mcl1 GetWidgetName] -side top -anchor nw -expand n -fill y -padx 2 -pady 2


# 
# set an observer to refresh the list box when the scene changes
#

$sc AddObserver ModifiedEvent "mrmlFillMCL $sc $mcl1"

proc mrmlFillMCL {scene mcl} {
    puts "got scene $scene and mcl $mcl"
    $mcl DeleteAllRows
    set nitems [$scene GetNumberOfNodes]
    puts "$nitems items"
    for {set i 0} {$i < $nitems} {incr i} {
        set n [$scene GetNthNode $i]
        puts [$n Print]
        $mcl InsertCellText $i 0 [$n GetClassName]
        $mcl InsertCellText $i 1 [$n GetName]
    }
}




# Start the application
# If --test was provided, do not enter the event loop and run this example
# as a non-interactive test for software quality purposes.

set ret 0
$win Display
if {!$option_test} {
  $app Start
  set ret [$app GetExitStatus]
}
$win Close

# Deallocate and exit

$win Delete
$app Delete

