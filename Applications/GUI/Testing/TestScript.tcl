puts "hoot!"

if { [info exists ::slicer3::Application] } {
  # the application exists, so exit with full shutdown
  $::slicer3::Application Exit
} else {
  # no application, so just exit the program directly
  exit 0
}
