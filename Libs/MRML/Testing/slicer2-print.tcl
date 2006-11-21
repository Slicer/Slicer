
#
# test of printing a slicer2 mrml scene using the 
# vtk xml data parser mechanism
#


proc PrintSlicer2Scene {sceneFile} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $sceneFile
  $parser Parse

  set root [$parser GetRootElement]

  PrintElement 0 $root
}

proc PrintIndent {indent} {
  for {set i 0} {$i < $indent} {incr i} {
    puts -nonewline " "
  }
}

proc PrintElement {indent element} {
  
  PrintIndent $indent
  incr indent
  puts "$element [$element GetName]"

  set nAtts [$element GetNumberOfAttributes]
  for {set i 0} {$i < $nAtts} {incr i} {
    PrintIndent $indent
    set attName [$element GetAttributeName $i]
    puts "$attName [$element GetAttributeValue $i]"
  } 

  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    PrintElement $indent $nestElement
  }
}

PrintSlicer2Scene c:/data/tutorial/tutorial.xml

