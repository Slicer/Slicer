set ::dfplcLabelFile /spl/tmp/nicole/fsLabels/lh.laurel-DLPFC-cut.label
#set ::dfplcLabelFile /spl/tmp/nicole/rh-zora-DLPFC-cut.label

# read in the label file and make fiducials at each point
# .ARGS
# fileName string name of the file to load labels from, ie lh-name-cut.label
# linesVisibleFlag boolean set to 1 if you wish to see lines between the contours
proc ReadLabels { { fileName ""} { linesVisibleFlag 0 } } {

    if { $fileName == ""} {
        set labelfile $::dfplcLabelFile
    } else {
        set labelfile $fileName
    }
    set fid [open $labelfile]
    set data [read $fid]
    close $fid
    set lines [split $data \n]

    set ver [package require vtk]
    if {$ver < 5.2} {
        puts "ERROR: this script only works with VTK 5.2+, the contour widget is not defined in this version: $ver"
        return
    }

    # set up a contour widget
    if { [info commands contourWidget] != "" } {
        CleanUpContourWidget
    }
    catch "contourRepresentation Delete"
    if {[catch "vtkContourWidget contourWidget" err] != 0} {
        puts "ERROR: unable to define a contour widget:\n$err"
        return
    }
    vtkOrientedGlyphContourRepresentation contourRepresentation

    set mainviewer [$::slicer3::ViewerWidget GetMainViewer]
    set renw [$mainviewer GetRenderWindow]
    set interactor [$renw GetInteractor]
    contourWidget SetInteractor $interactor

    contourWidget SetRepresentation contourRepresentation
    [contourRepresentation GetLinesProperty] SetColor 1.0 0.0 0.0
    if {$linesVisibleFlag == 0} {
        # make the lines invisible
        [contourRepresentation GetLinesProperty] SetOpacity 0 
    }
    contourWidget On
   

    catch "pd Delete"
    catch "points Delete"
    catch "lines Delete"
    catch "lineIndices Delete"
    vtkPolyData pd
    vtkPoints points
    vtkCellArray lines
    vtkIdList lineIndices

    set numLabelsRead 0
    foreach l $lines {
        if {[string index $l 0] == "#"} { 
            puts "Comment:\n$l" 
        } elseif {[llength [split $l]] == 1} {
            set numLabels $l
            puts "Have $numLabels to read"
        } else {
            # assume have a regular line, vertex_num r a s val
            set vertexNum [lindex $l 0]
            set r [lindex $l 1]
            set a [lindex $l 2]
            set s [lindex $l 3]
            set val [lindex $l 3]
            if {$r != "" && $a != "" && $s != ""} {
                points InsertPoint $numLabelsRead $r $a $s
                lineIndices InsertNextId $numLabelsRead
                incr numLabelsRead
            } else {
                puts "Error parsing line '$l'"
            }
            if {$numLabelsRead > 200} {
               # break
            }
        }
    }
    lineIndices InsertNextId 0
    lines InsertNextCell lineIndices
    pd SetPoints points
    pd SetLines lines
    points Delete
    lines Delete
    
    # 1 is the state, ready to manipulate it now, 0 would be that we're still defining it
    contourWidget Initialize pd 1
    contourWidget Render
    $renw Render
    $interactor Initialize

    if {$numLabelsRead != $numLabels} {
        puts "Error: read $numLabelsRead labels, file $labelfile said there should be $numLabels"
        return 0
    } else {
        puts "Read  $numLabelsRead labels from file $labelfile"
        return 1
    }
}

proc CleanUpContourWidget {} {
    contourWidget RemoveObservers EnableEvent
    contourWidget RemoveObservers DeleteEvent
    contourWidget EnabledOff
    contourWidget Off
    
    catch "contourWidget Delete"
    catch "contourRepresentation Delete"
}
