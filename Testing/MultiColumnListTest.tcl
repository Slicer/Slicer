

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
$::mcl InsertCellText 1 0 "Test2"

set ::mcl2 [vtkKWMultiColumnList New]
$::mcl2 SetParent [$::lframe GetFrame]
$::mcl2 Create
pack [$::mcl2 GetWidgetName] -side top -anchor nw -expand false -fill none -padx 2 -pady 2

update
$::mcl2 DeleteAllRows
$::mcl2 AddColumn "Test Col"
$::mcl2 InsertCellText 0 0 "Test"
$::mcl2 InsertCellText 1 0 "Test2"
