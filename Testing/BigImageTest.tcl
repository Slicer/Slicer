

set id [vtkImageData New]

$id SetDimensions 1000 1000 3000
$id SetScalarTypeToUnsignedChar 
$id AllocateScalars

puts "[$id Print]"

puts "memory size is [$id GetActualMemorySize]"

$id Delete
