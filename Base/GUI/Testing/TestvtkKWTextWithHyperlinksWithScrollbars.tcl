
  
  
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
$app SetName "KWHelloWorldExample"
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


 set ::Test(text1) [vtkKWTextWithHyperlinksWithScrollbars New]
  $::Test(text1) SetParent [$win GetViewFrame]
  $::Test(text1) Create
  pack [$::Test(text1) GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2
 $::Test(text1) SetText "0123456<a>http://www.kwwidgets.org</a>38"


 # $::Test(text1) SetText "0123456<a>http://www.kwwidgets.org</a> <a>http://www.slicer.org</a>fadsfasdfasdfasdfasdfad 38<a>http://www.google.com</a>"    
 
  set ::Test(text2) [vtkKWTextWithHyperlinksWithScrollbars New]
  $::Test(text2) SetParent [$win GetViewFrame]
  $::Test(text2) Create
  $::Test(text2) SetText "01234567"
  pack [$::Test(text2) GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2
  
  
    set ::Test(text3) [vtkKWTextWithHyperlinksWithScrollbars New]
  $::Test(text3) SetParent [$win GetViewFrame]
  $::Test(text3) Create
    $::Test(text3) SetText "01234567"
  $::Test(text3) SetText ""
  pack [$::Test(text3) GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2
  
  
     set ::Test(text4) [vtkKWTextWithHyperlinksWithScrollbars New]
  $::Test(text4) SetParent [$win GetViewFrame]
  $::Test(text4) Create
  $::Test(text4) SetText "0123456<a>http://www.kwwidgets.org</a> <a>http://www.slicer.org</a>fadsfasdfasdfasdfasdfad 38<a>http://www.google.com</a>"    
  pack [$::Test(text4) GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2
# Start the application
# If --test was provided, do not enter the event loop and run this example
# as a non-interactive test for software quality purposes.

set ret 0
$win Display




