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
$app SetName "Scripted Widget Example"
if {$option_test} {
  $app SetRegistryLevel 0
  $app PromptBeforeExitOff
}
$app RestoreApplicationSettingsFromRegistry

# Set a help link. Can be a remote link (URL), or a local file

$app SetHelpDialogStartingPage "http://www.kwwidgets.org"

# Add a window
# Set 'SupportHelp' to automatically add a menu entry for the help link

set win [vtkKWWindow New]
$win SupportHelpOn
$app AddWindow $win
$win Create


set sw [vtkKWScriptedWidget New]
$sw SetApplication $app
$sw SetParent $win
$sw SetInitializeCommand { source $::SLICER_BUILD/../Slicer3/Base/GUI/Tcl/ScriptedWidgetExample.tcl; ScriptedWidgetExampleInitialize }

$sw Create

pack [$sw GetWidgetName] -side top -anchor nw -expand true -fill x -padx 2 -pady 2



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

