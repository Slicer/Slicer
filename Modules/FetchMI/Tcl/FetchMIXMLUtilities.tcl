
#----------------------------------------------------------------------------------------------------
#--- NO LONGER USED. moved to FetchMILogic's Writer
#----------------------------------------------------------------------------------------------------
proc FetchMIWriteDocumentDeclaration_XND { filename } {

    #--- write the chars in declaration to file filename
    #--- using UTF-8 character encoding.
    set declaration "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    string trim $declaration
    set f [ open $filename w ]
    fconfigure $f -encoding utf-8 
    puts $f $declaration
    close $f
    
    puts "docdec = $declaration"
}


#----------------------------------------------------------------------------------------------------
#--- NO LONGER USED. moved to FetchMILogic's Writer
#----------------------------------------------------------------------------------------------------
proc FetchMIWriteXMLHeader_XND { headerFilename  dataFilename } {

    #--- write the chars in declaration to file filename
    #--- using UTF-8 character encoding.
    set f [ open $headerFilename w ]
    fconfigure $f -encoding utf-8 

    set header "Content-Type: application/x-xnat-metadata+xml"
    string trim $header
    puts $f $header
    set header "Content-Disposition: x-xnat-metadata; filename=\"$dataFilename\""
    string trim $header
    puts $f $header

    close $f

}



#----------------------------------------------------------------------------------------------------
#--- NO LONGER USED. moved to FetchMILogic's Writer
#----------------------------------------------------------------------------------------------------
proc FetchMIWriteMetadataForScene_XND {  metadataFilename docdecFilename } {

    set fgui  $::slicer3::FetchMIGUI
    set t [ $::slicer3::MRMLScene GetUserTagTable ]
    $t AddOrUpdateTag "SlicerDataType" "MRML"
    set num [$t GetNumberOfTags ]

    #--- hardcode for now.
    set namespace "xmlns=\"http://nrg.wustl.edu/xe\""
    
    #--- get declaration
    set f [ open $docdecFilename r ]
    gets $f declaration
    close $f

    set f [ open $metadataFilename w ]
    fconfigure $f -encoding utf-8

    string trim $declaration
    puts $f $declaration
    set line "<Metadata $namespace>" 
    string trim $line
    puts $f $line

    for { set i 0} {$i < $num} { incr i } {
        set att [ $t GetTagAttribute $i ]
        set val [ $t GetTagValue $i ]
        #if  $t IsTagSelected $att  > 0 
        # use all tags for now
        set line "<Tag Label=\"$att\">"
        string trim $line
        puts $f $line
        set line "<Value>$val</Value>"
        string trim $line
        puts $f $line
        set line "</Tag>"
        string trim $line
        puts $f $line
    }
    set line "</Metadata>"
    string trim $line
    puts $f $line

#    exec dos2unix.exe $metadataFilename
    
    close $f
}



#----------------------------------------------------------------------------------------------------
#--- NO LONGER USED. moved to FetchMILogic's Writer
#----------------------------------------------------------------------------------------------------
proc FetchMIWriteMetadataForNode_XND { metadataFilename docdecFilename nodeID } {

    set node [ $::slicer3::MRMLScene GetNodeByID $nodeID ]
    set t [ $node GetUserTagTable ]
    set num [ $t GetNumberOfTags ]
    set found 0
    #--- hardcode for now.
    set namespace "xmlns=\"http://nrg.wustl.edu/xe\""

    #--- check for SlicerDataType Attribute
    for { set i 0 } { $i < $num } { incr i } {
        set att [ $t GetTagAttribute $i ]
        if { [string equal $att "SlicerDataType"] } {
            set found 1
            break
        }
    }

    if {$found == 0 } {
        $t AddOrUpdateTag "SlicerDataType" "unknown"
    }

#---
    # i think this is a bug block -- copied and pasted in. but wrong.
    # test with it commented out and delete if it's truly junk.
#    set fgui  $::slicer3::FetchMIGUI
#    set t [ $::slicer3::MRMLScene GetUserTagTable ]
#    $t AddOrUpdateTag "SlicerDataType" "MRML"
#    set num [$t GetNumberOfTags ]
#---

    #--- get declaration
    set f [ open $docdecFilename r ]
    gets $f declaration
    close $f

    set f [ open $metadataFilename w ]
    fconfigure $f -encoding utf-8

    string trim $declaration
    puts $f $declaration
    set line "<Metadata $namespace>" 
    string trim $line
    puts $f $line

    for { set i 0} {$i < $num} { incr i } {
        set att [ $t GetTagAttribute $i ]
        set val [ $t GetTagValue $i ]
        #if  $t IsTagSelected $att  > 0 
        # use all tags for now
        set line "<Tag Label=\"$att\">"
        string trim $line
        puts $f $line
        set line "<Value>$val</Value>"
        string trim $line
        puts $f $line
        set line "</Tag>"
        string trim $line
        puts $f $line
    }
    set line "</Metadata>"
    string trim $line
    puts $f $line    

#    exec dos2unix.exe $metadataFilename
    
    close $f

}




#----------------------------------------------------------------------------------------------------
#--- NO LONGER USED. moved to FetchMILogic's Parser
#----------------------------------------------------------------------------------------------------
proc FetchMIParseMetadataPostResponse_XND {  responseFilename  } {

    set fgui  $::slicer3::FetchMIGUI
    set f [ open $responseFilename r ]
    set found 0

    #--- look for a second uri in response
    while { [gets $f line] >= 0 }  {
        if { [string first "http://" $line ] == 0 } {
            set found 1
            break
        }
    }
    if { $found == 1 } {
        set dirtyuri [encoding convertto $line ]
        set uri [ string trim $dirtyuri ]
        $fgui SetReservedURI $uri
        close $f
        puts "got :$uri"
    }
}

