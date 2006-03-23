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
set scene [vtkMRMLScene New]

# Undo button
set pb [vtkKWPushButton New]
$pb SetParent $win
$pb Create
$pb SetText "Undo"
$pb SetAnchorToWest
pack [$pb GetWidgetName] -side top -anchor nw -expand false -fill x -padx 2 -pady 2
$pb SetBalloonHelpString \
"Undo manipulations of mrml scene."
$pb SetCommand "" "$scene Undo"

# Redo button
set pb [vtkKWPushButton New]
$pb SetParent $win
$pb Create
$pb SetText "Redo"
$pb SetAnchorToWest
pack [$pb GetWidgetName] -side top -anchor nw -expand false -fill x -padx 2 -pady 2
$pb SetBalloonHelpString \
"Redo manipulations of mrml scene."
$pb SetCommand "" "$scene Redo"

# Add a button for each mrml node type

# set nodes [info command vtkMRML*Node]
set nodes "vtkMRMLVolumeNode vtkMRMLModelNode"

foreach n $nodes {

  set nn [$n New]
  $scene RegisterNodeClass $nn

  set pb [vtkKWPushButton New]
  $pb SetParent $win
  $pb Create
  $pb SetText "$n"
  $pb SetAnchorToWest
  pack [$pb GetWidgetName] -side top -anchor nw -expand false -fill x -padx 2 -pady 2
  $pb SetBalloonHelpString \
    "Add a $n to the current mrml scene."
  $pb SetCommand "" "mrmlAddNode $scene $n"

}

proc mrmlAddNode {scene nodetype} {

    set n [$nodetype New]
    $n SetName Node-[clock seconds]
    $scene SaveStateForUndo
    $scene AddNode $n
}



# -----------------------------------------------------------------------

# Create a multi-column list

set mcl [vtkKWMultiColumnList New]
$mcl SetParent $win
$mcl Create
$mcl SetBalloonHelpString \
"A simple multicolumn list. Columns can be resized moved and sorted.\
Double-click on some entries to edit them."
$mcl MovableColumnsOn
$mcl SetWidth 0
$mcl SetPotentialCellColorsChangedCommand \
    $mcl "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
$mcl SetColumnSortedCommand \
    $mcl "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

# Add the columns make some of them editable

set col_index [$mcl AddColumn "Node Type"] 

set col_index [$mcl AddColumn "Name"] 
$mcl SetColumnAlignmentToCenter $col_index
$mcl ColumnEditableOn $col_index

set col_index [$mcl AddColumn "ID"] 
$mcl SetColumnAlignmentToCenter $col_index


pack [$mcl GetWidgetName] -side top -anchor nw -expand n -fill y -padx 2 -pady 2

#
# set the command to execute when a cell is edited
#

$mcl SetCellUpdatedCommand "" "mrmlUpdateName $scene $mcl"

proc mrmlUpdateName {scene mcl row col txt} {

    set id [$mcl GetCellText $row 2]
    set name [$mcl GetCellText $row 1]
    set coll [$scene GetNodesByID $id]
    set node [$coll GetItemAsObject 0]
    $scene SaveStateForUndo $node
    $node SetName $name
}

# 
# set an observer to refresh the list box when the scene changes
#

$scene AddObserver ModifiedEvent "mrmlFillMCL $scene $mcl"

proc mrmlFillMCL {scene mcl} {
    $mcl DeleteAllRows
    set nitems [$scene GetNumberOfNodes]
    for {set i 0} {$i < $nitems} {incr i} {
        set n [$scene GetNthNode $i]
        $mcl InsertCellText $i 0 [$n GetClassName]
        $mcl InsertCellText $i 1 [$n GetName]
        $mcl InsertCellText $i 2 [$n GetID]
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

