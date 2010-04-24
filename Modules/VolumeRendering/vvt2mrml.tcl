
# main entry point...
proc ImportVvt {file out} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $file
  $parser Parse

  set root [$parser GetRootElement]

  set sc [vtkMRMLScene New]
  
  set ::so(values) ""
  set ::so(inside) ""
  set ::so(count) 0
  set ::so(incrm) 1

  set ::go(values) ""
  set ::go(inside) ""
  set ::go(count) 0
  set ::go(incrm) 1

  set ::ct(values) ""
  set ::ct(inside) ""
  set ::ct(count) 0
  set ::ct(incrm) 3

  ImportElement $root

  puts "ScalarOpacity parsed"
  puts $::so(count)
  puts $::so(values)

  puts "GradientOpacity parsed"
  puts $::go(count)
  puts $::go(values)

  puts "ColorTransferFunction parsed"
  puts $::ct(count)
  puts $::ct(values)

  set so $::so(count)

  append so $::so(values)

  puts "ScalarOpacity"
  puts $so

  set vp [vtkMRMLVolumePropertyNode New]
  set pfs [vtkPiecewiseFunction New]
  $vp GetPiecewiseFunctionFromString $so $pfs
  [$vp GetVolumeProperty] SetScalarOpacity $pfs
  $pfs Delete

  $sc AddNode $vp
  $sc SetURL $out
  $sc Commit $out

  $sc Delete
  $parser Delete
}

#
# recursive routine to import all elements and their
# nested parts
#
proc ImportElement {element} {

  # import this element
  ImportNode $element

  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    ImportElement $nestElement
  }

}

#
# parse the attributes of a node into a tcl array
# and then invoke the type-specific handler
#
proc ImportNode {element} {

  set nAtts [$element GetNumberOfAttributes]
  for {set i 0} {$i < $nAtts} {incr i} {
    set attName [$element GetAttributeName $i]
    set node($attName) [$element GetAttributeValue $i]
  } 

  set nodeType [$element GetName]
  set handler ImportNode$nodeType
  if { [info command $handler] == "" } {
     puts "no handler for $nodeType"
  }

  # call the handler for this element
  $handler node
}

#####
#
# handler procs for each type of node
#

proc ImportNodeTransferFunctions {node} {
  
}

proc ImportNodeVolumeProperty {node} {

}


proc ImportNodeComponent {node} {

}

proc ImportNodeRGBTransferFunction {node} {

}

proc ImportNodePiecewiseFunction {node} {

}


proc ImportNodeColorTransferFunction {node} {
    set ::ct(inside) "1"
    if {$::ct(count) != 0} {
        set ::ct(inside) ""
    } else {
        puts "in ColorTransferFunction"
    }
    set ::so(inside) ""
    set ::go(inside) ""
}

proc ImportNodeScalarOpacity {node} {
    set ::ct(inside) ""
    set ::so(inside) "1"
    if {$::so(count) != 0} {
        set ::so(inside) ""
    } else {
        puts "in ScalarOpacity"
    }
    set ::go(inside) ""
}

proc ImportNodeGradientOpacity {node} {
    set ::ct(inside) ""
    set ::so(inside) ""
    set ::go(inside) "1"
    if {$::go(count) != 0} {
        set ::go(inside) ""
    } else {
        puts "in GradientOpacity"
    }
}


proc ImportNodePoint {node} {
    upvar $node n
    

    if {$::ct(inside) != ""} {
#        lappend ::ct(values) " "
        lappend ::ct(values) $n(X)
        lappend ::ct(values) $n(Value)
        set ::ct(count) [expr $::ct(count) + $::ct(incrm) + 1]
    }

    if {$::so(inside) != ""} {
#        lappend ::so(values) " "
        lappend ::so(values) $n(X)
        lappend ::so(values) $n(Value)
        set ::so(count) [expr $::so(count) + $::so(incrm) + 1]
    }
    if {$::go(inside) != ""} {
#        lappend ::go(values) " "
        lappend ::go(values) $n(X)
        lappend ::go(values) $n(Value)
        set ::go(count) [expr $::go(count) + $::go(incrm) + 1]
    }
}

