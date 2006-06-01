

set id [vtkImageData New]

$id SetDimensions 1000 1000 4000
$id AllocateScalars

$id Update
puts "[$id Print]"

puts "memory size is [$id GetActualMemorySize]"

$id Delete
