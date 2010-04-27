
# main entry point...
proc ImportVvt {file out} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $file
  $parser Parse

  set root [$parser GetRootElement]

  set sc [vtkMRMLScene New]
  set vp [vtkMRMLVolumePropertyNode New]
  $vp SetName [file rootname [file tail $file]]

  set ::vp $vp

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

  set so $::so(count)
  append so " "
  append so $::so(values)
  puts "ScalarOpacity"
  puts $so

  set go $::go(count)
  append go " "
  append go $::go(values)
  puts "GradientOpacity"
  puts $go

  set ct $::ct(count)
  append ct " "
  append ct $::ct(values)
  puts "ColorTransferFunction"
  puts $ct

  set pfs [vtkPiecewiseFunction New]
  set pfg [vtkPiecewiseFunction New]

  $vp GetPiecewiseFunctionFromString $so $pfs
  [$vp GetVolumeProperty] SetScalarOpacity $pfs

  $vp GetPiecewiseFunctionFromString $go $pfg
  [$vp GetVolumeProperty] SetGradientOpacity $pfg

  set ctf [vtkColorTransferFunction New]
  $vp GetColorTransferFunctionFromString $ct $ctf
  [$vp GetVolumeProperty] SetColor $ctf

  $ctf Delete
  $pfs Delete

  $sc AddNode $vp
  $sc SetURL $out
  $sc Commit $out
  
  $vp Delete
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
    upvar $node n
    if {$n(Index) == 0} {
        puts "Component 0"
        [$::vp GetVolumeProperty] SetShade $n(Shade)
        [$::vp GetVolumeProperty] SetAmbient $n(Ambient)
        [$::vp GetVolumeProperty] SetDiffuse $n(Diffuse)
        [$::vp GetVolumeProperty] SetSpecular $n(Specular)
        [$::vp GetVolumeProperty] SetSpecularPower $n(SpecularPower)
        [$::vp GetVolumeProperty] SetDisableGradientOpacity $n(DisableGradientOpacity)
    }
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
        append ::ct(values) " "
        append ::ct(values) $n(X)
        append ::ct(values) " "
        append ::ct(values) $n(Value)
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

