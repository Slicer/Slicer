# test closing the current mrml scene and exiting

proc TestSceneClear {  } {

    $::slicer3::MRMLScene Clear 0

    # success
    return 0
}

proc runtest { } {
    set ret [ catch {
         $::slicer3::Application TraceScript TestSceneClear
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0

}

update 
after idle runtest
