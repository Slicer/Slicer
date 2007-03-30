
# package require ETC

::ETC::SetProject ExampleSegmentation

::ETC::SetWorkingDirectory c:/Projects/Data/MRSRegistrationData/1055/2
set SubjectList [lrange [::ETC::FindData *.dcm] 0 4]

# ::ETC::Schedule {EMSegment -foo 10.0 $Subject} "Subject $SubjectList"

foreach Subject $SubjectList {
  set Step1Out [file root $Subject].out1
  set Step2Out [file root $Subject].out2
  ::ETC::Schedule -name EM$Subject -depends ""  "EMSegment -foo 10.0 $Subject $Step1Out"
  ::ETC::Schedule -name EM2$Subject -depends EM$Subject  "EMSegment2 -foo 10.0 $Step1Out $Step2Out"
}

::ETC::Schedule -name Summary -depends EM2* ""
::ETC::RootTask Summary

::ETC::Generate Makefile

if { 0 } {
::ETC::PForeach Subject $Subjects {
  set Step1Out [file root $Subject].out1
  set Step2Out [file root $Subject].out2
  ::ETC::Schedule -name EM3$Subject "EMSegment -foo 10.0 $Subject $Subject.out1"
  ::ETC::Schedule -name EM4$Subject -depends EM3$Subject "EMSegment2 -foo 10.0 $Subject.out1 $Subject.out2"
}

::ETC::LeaveOneOut TestSubject TrainSubjects $Subjects {
  ::ETC::Schedule -name Train$Subject Train -foo 10.0 $TrainSubjects $TestSubject
} {
  ::ETC::Schedule -name Test$Subject -depends Train$Subject "Test $TestSubject"
}


::ETC::LeaveNOut 10 TestSubjects TrainSubjects $Subjects {
  ::ETC::Schedule -name TrainN$TestSubjects "Train -foo 10.0 $TrainSubjects $TestSubjects"
} {
  ::ETC::Schedule -name TestN$TestSubjects -depends TrainN$TestSubjects "Test $TestSubjects"
}


foreach Subject $Subjects {
  ::ETC::Schedule "EMSegment -foo 10.0 $Subject $Subject.out; EMSegment2 -foo 10.0 $Subject.out $Subject.out2"
}
  


::ETC::Schedule -name ProcessA -depends EM2* [eval "ProcessA -foo 10.0 $SubjectList"]


foreach Subject $Subjects {
  for { set i -10 } { $i < 10 } { incr i } {
    for { set j -100 } { $j < 100 } { incr j 10 } {
      Schedule -priority low "$Module(EMSegment) -bah [list $i $j] $Subject [OutputFilename $FooStep($Subject)]"
    }
  }
  Barrier -depends FooStep BarStep
}

Go BarStep

}
