

# a test for the progress dialog

update

set pd [vtkKWProgressDialog New]
$pd SetApplication $::slicer3::Application
$pd SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
$pd SetDisplayPositionToScreenCenter
$pd Create
$pd SetMessageText "This is a sample progress bar for testing..."
$pd Display

for {set p .1} {$p <= 1} {set p [expr $p + .1]} {
  $pd UpdateProgress $p
  after 100
}

$pd Withdraw
$pd Delete

update

exit 0
