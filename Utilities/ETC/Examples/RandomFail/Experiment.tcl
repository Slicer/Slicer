
source ../../ETC.tcl

::ETC::SetProject ExampleSegmentation

::ETC::SetWorkingDirectory .
for {set i 0} {$i < 50} {incr i} {
  lappend SubjectList ex$i
}

# ::ETC::Schedule {EMSegment -foo 10.0 $Subject} "Subject $SubjectList"

foreach Subject $SubjectList {
  ::ETC::Schedule -name EM$Subject "./randomFail -d 1000 -p 0.6 -o data/$Subject"
}

::ETC::Schedule -name Summary -depends EM* ""
::ETC::RootTask Summary

::ETC::Generate Makefile.ETC
