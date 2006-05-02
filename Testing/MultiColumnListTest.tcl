

set ::app [vtkKWApplication ListInstances] ;# application should be a singleton

set ::top [vtkKWTopLevel New]
$::top SetApplication $::app
$::top Create
$::top SetTitle "Matrices"
$::top Display

$::top SetDeleteWindowProtocolCommand "" "$::top Delete"

set ::lframe [vtkKWFrameWithLabel New]
$::lframe SetParent $::top
$::lframe Create
$::lframe SetLabelText "Frame"
pack [$::lframe GetWidgetName] -side top -anchor nw -expand false -fill none -padx 2 -pady 2

set ::mcl [vtkKWMultiColumnList New]
$::mcl SetParent [$::lframe GetFrame]
$::mcl Create
pack [$::mcl GetWidgetName] -side top -anchor nw -expand false -fill none -padx 2 -pady 2

update
$::mcl DeleteAllRows
$::mcl AddColumn "Test Col"
$::mcl InsertCellText 0 0 "Test"
