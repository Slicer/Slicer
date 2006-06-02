
set uca [vtkUnsignedCharArray New]

$uca SetNumberOfComponents 1000
$uca SetNumberOfTuples [expr 1000 * 3000]

puts "[$uca Print]"

$uca Delete
