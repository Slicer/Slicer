set missingLibraries 0

if { [catch "package require Itcl" err] } {
    puts "Must have the Itcl package for plotting:\n\t$err"
    incr missingLibraries
}

if { [catch "package require Iwidgets" err] } {
    puts "Must have the Iwidgets package for plotting:\n\t$err"
    incr missingLibraries
}

if { [catch "package require BLT" err] } {
    puts "Must have the BLT package for plotting:\n\t$err"
    incr missingLibraries
}

exit $missingLibraries
