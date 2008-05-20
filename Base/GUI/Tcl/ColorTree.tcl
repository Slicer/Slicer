package require Itcl

#########################################################
#
if {0} { ;# comment

  ColorTree is a wrapper around a set of kwwidgets and 
  the tktreectrl tcl widget
  to show and edit mrml color nodes

# TODO : 
  - part of this should be split out into logic helper methods
  - this could be reimplemented in C++ pretty easily

}
#
#########################################################


namespace eval ColorTree {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog { {path ""} } {

    set colorTrees [itcl::find objects -class ColorTree]
    if { $colorTrees != "" } {
      set colorTree [lindex $colorTrees 0]
      array set o [$colorTree objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set colorTree [ColorTree #auto $::slicer3::MRMLScene]
    }
  }

}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ColorTree] == "" } {

  itcl::class ColorTree {

    constructor  { {scene ""} } {
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable scene ""  ;# the mrml scene to use

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access

    variable _tree ""
    variable _nodesByItem ;# an array to get a node ID from a tree item
    variable _observerRecords ""

    # methods
    method clear {} {}
    method expand {item} {}
    method processEvent {{caller ""} {event ""}} {}
    method errorDialog {errorText} {}
    method status {message} {}

    method objects {} {return [array get o]}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this ColorTree
    method vtkDelete {} {
      foreach object $_vtkObjects {
        catch "$object Delete"
      }
      set _vtkObjects ""
    }

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body ColorTree::constructor { {scene ""} } {

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Color Tree"
  $o(toplevel) Create

  # delete this instance when the window is closed
  wm protocol [$o(toplevel) GetWidgetName] \
    WM_DELETE_WINDOW "itcl::delete object $this"

  set _tree [treectrl [$o(toplevel) GetWidgetName].tree -width 500 -height 600]
  $_tree configure -showrootbutton no -showlines no -selectmode browse
  pack $_tree -fill both -expand true

  #
  # create columns
  #
  $_tree column create -text "Node" -itembackground {#F7F7F7} -tag colNode
  $_tree column create -text "Color Name" -itembackground {} -tag colName
  $_tree column create -text "Color" -justify center -itembackground {} -tag colColor

  # Specify the column that will display the heirarchy buttons and lines
  $_tree configure -treecolumn colNode

  #
  # create elements
  #
  $_tree element create elemNode text -lines 1
  $_tree element create elemColorName text -lines 1
  $_tree element create elemColorBox rect -width 20 -height 20

  #
  # create styles
  #
  set style [$_tree style create styleNode]
  $_tree style elements $style elemNode
  $_tree style layout $style elemNode -expand ns

  set style [$_tree style create styleColorName]
  $_tree style elements $style elemColorName
  $_tree style layout $style elemColorName -expand ns

  set style [$_tree style create styleColorBox]
  $_tree style elements $style elemColorBox
  $_tree style layout $style elemColorBox -expand ewns

  #
  # create items based on scene
  #
  $this configure -scene $scene

  $o(toplevel) Display
}


itcl::body ColorTree::destructor {} {

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this vtkDelete

}

itcl::configbody ColorTree::scene {

  $_tree item delete all

  $_tree notify bind $_tree <Expand-before> "$this expand %I"

  set n [$scene GetNumberOfNodesByClass "vtkMRMLColorNode"]
  for {set i 0} {$i < $n} {incr i} {
    set colorNode [$scene GetNthNodeByClass $i "vtkMRMLColorNode"]

    set item [$_tree item create -open no]
    $_tree item style set $item colNode styleNode
    $_tree item configure $item -button yes
    $_tree item element configure $item colNode elemNode -text [$colorNode GetName]
    $_tree item lastchild root $item
    set _nodesByItem($item) $colorNode
  }

}

# remove entries from the list box
itcl::body ColorTree::clear { } {
  $_tree item delete all
}

# create the sub-tree for a node
itcl::body ColorTree::expand {item} {

  if { ![info exists _nodesByItem($item)] } {
    # this isn't a parent node
    return
  } else {
    set node $_nodesByItem($item)
  }

  if { [$_tree item firstchild $item] != "" } {
    # already opened
    return
  }


  set lut [$node GetLookupTable]
  if { $lut == "" } {
    return
  }
  set nColors [$lut GetNumberOfColors]

  for {set c 0} {$c < $nColors} {incr c} {
    set name [$node GetColorName $c]
    set child [$_tree item create -open no]

    $_tree item style set $child colName styleColorName
    $_tree item element configure $child colName elemColorName -text $name

    $_tree item style set $child colColor styleColorBox
    set color [::ColorTree::vtkColorToHex [$lut GetTableValue $c]]
    $_tree item element configure $child colColor elemColorBox -fill $color

    $_tree item configure $child -button no
    $_tree item lastchild $item $child
  }
}

#
# handle gui events
# -basically just map button events onto methods
#
itcl::body ColorTree::processEvent { {caller ""} {event ""} } {

  if { $caller == $o(addDir) } {
    # TODO: switch to kwwidgets directory browser
    $this add [$this chooseDirectory]
    return
  }

  if { $caller == $o(addFile) } {
    # TODO: switch to kwwidgets directory browser
    $this add [$this getOpenFile]
    return
  }

  if { $caller == $o(apply) } {
    $this apply
    after idle "itcl::delete object $this"
    return
  }

  if { $caller == $o(cancel) } {
    after idle "itcl::delete object $this"
    return
  }
  
  puts "unknown event from $caller"
}

proc ::ColorTree::vtkColorToHex  {color} {
  foreach {r g b a} $color {}
  foreach c {r g b a} {
    set $c [expr int (255 * [set $c])]
  }
  return [format #%02x%02x%02x $r $g $b]
}

proc cccccc {} {
  itcl::delete class ColorTree
  source $::env(Slicer3_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl/ColorTree.tcl
  ::ColorTree::ShowDialog
}
