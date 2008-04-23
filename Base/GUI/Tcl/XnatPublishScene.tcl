#------------------------------------------------------------------------------
# Generates time stamp for MRML scene and all data files.
# This stamp is applied to all files being published
# to xnat so no original data is overwritten.
#------------------------------------------------------------------------------
proc XnatPublish_GetMRMLTimeStamp { } {

    set stamp [clock format [clock seconds] -format {%Y-%m-%d-%Hh-%Mm-%Ss}]
    return $stamp
}


#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_TimeStampString { str } {
    set stamp [ XnatPublish_GetMRMLTimeStamp ]
    if { $str == "" } {
        return $stamp
    } else {
        set newstr "$stamp-$str"
        return $newstr
    }
}


#------------------------------------------------------------------------------
#--- prompt user for scene
#------------------------------------------------------------------------------
proc XnatPublish_PromptUser { } {

    set handler [$::slicer3::MRMLScene FindURIHandler "xnat://"]
    if { $handler == "" } {
        #--- pop up error message.
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText "XnatPublishScene: can't find URI Handler for XNAT. No data was published."
        $dialog Create
        $dialog Invoke
        $dialog Delete
        return -1
    }
    set prompter [ $handler GetPermissionPrompter ]
    if { $prompter == "" } {
        #--- pop up error message.
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText "XnatPublishScene: can't find Permission Prompter for XNAT. No data was published."
        $dialog Create
        $dialog Invoke
        $dialog Delete
        return -1
    }
    set retval [ $prompter Prompt "Please provide the requested credentials." $::slicer3::MRMLScene ]
    return $retval
}



#------------------------------------------------------------------------------
#--- Creates lists of volume nodes, model nodes grid nodes that
#--- should be saved in the scene.
#------------------------------------------------------------------------------
proc XnatPublish_PopulateLists { } {

    #--- volume nodes
    set nnodes [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLVolumeNode" ]
    for { set n 0 } {$n < $nnodes } { incr n } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLVolumeNode" ]
        set snode [ $node GetStorageNode ]
        if { ![$node GetHideFromEditors] } {
            if { $snode != ""  } {
                lappend ::XnatPublish_NodeList $node
                lappend ::XnatPublish_StorageNodeList $snode
            }
        }
    }
    #--- model nodes
    set nnodes [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode" ]
    for { set n 0 } {$n < $nnodes } { incr n } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLModelNode" ]
        set snode [ $node GetStorageNode ]
        if { ![$node GetHideFromEditors] } {
            if { $snode != ""  } {
                lappend ::XnatPublish_NodeList $node
                lappend ::XnatPublish_StorageNodeList $snode
            }
        }
    }

    #--- transform nodes
    if { 0 } {
    set nnodes [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLTransformNode" ]
    for { set n 0 } {$n < $nnodes } { incr n } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLTransformNode" ]
        set snode [ $node GetStorageNode ]
        if { ![$node GetHideFromEditors] } {
            if { $snode != ""  } {
                lappend ::XnatPublish_NodeList $node
                lappend ::XnatPublish_StorageNodeList $snode
            }
        }
    }
    }
    #--- unstructured grid nodes
    set nnodes [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLUnstructuredGridNode" ]
    for { set n 0 } {$n < $nnodes } { incr n } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLUnstructuredGridNode" ]
        set snode [ $node GetStorageNode ]
        if { ![$node GetHideFromEditors] } {
            if { $snode != ""  } {
                lappend ::XnatPublish_NodeList $node
                lappend ::XnatPublish_StorageNodeList $snode
            }
        }
    }
}




#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_PublishScene {} {
    
    #--- make sure we have the proper handler and permission prompter.
    set handler [ $::slicer3::MRMLScene FindURIHandler "xnat://" ]
    if { $handler == "" } {
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText "XnatPublishScene: can't find URI Handler for XNAT. No data was published."
        $dialog Create
        $dialog Invoke
        $dialog Delete
        return -1
    }
    set prompter [ $handler GetPermissionPrompter ]
    if { $prompter == "" } {
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText "XnatPublishScene: can't find Permission Prompter for XNAT. No data was published."
        $dialog Create
        $dialog Invoke
        $dialog Delete
        return -1
    }

    #--- get info
    set promptStat [ XnatPublish_PromptUser ]
    puts "Prompt returned $promptStat"
    
    if { $promptStat <= 0 } {
        #--- no reason for an error message here.
    } else { 
        #--- get info for publishing.
        #--- need to timestamp Scene name and all datasets.
        unset -nocomplain ::XnatPublish_Time
        set ::XnatPublish_Time [ XnatPublish_TimeStampString "" ]
        
        set sceneName [ $prompter GetSceneName ]
        set sceneName $::XnatPublish_Time$sceneName
        
        if { $sceneName != "" &&  $::slicer3::MRMLScene != "" } { 
            #--- get cache directory from cache manager.
            set cm [ $::slicer3::MRMLScene GetCacheManager ]
            if { $cm == "" } {
                set dialog [ vtkKWMessageDialog New ]
                $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
                $dialog SetStyleToMessage
                $dialog SetText "XnatPublishScene: Can't find Cache Manager. No data was published.."
                $dialog Create
                $dialog Invoke
                $dialog Delete
                return -1
            }
            set cdir [ $cm GetRemoteCacheDirectory ] 
            if { $cdir != "" } {
                set len [ string length $cdir ]
                set len [ expr $len - 1 ]
                set tst [ string range $cdir $len $len ]
                if { ! [ string match $tst "/"] } {
                    set cdir "$cdir/"
                }
            } else {
                set dialog [ vtkKWMessageDialog New ]
                $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
                $dialog SetStyleToMessage
                $dialog SetText "XnatPublishScene: Remote cache directory must be set before data can be published to XNAT."
                $dialog Create
                $dialog Invoke
                $dialog Delete
                return -1
            }

            #--- convert all dataset paths to the cache directory
            #--- temporarily,  timestamp the filenames
            #--- save to cache, and restore original filenames
            puts "SAVING DATA."
            set scene $::slicer3::MRMLScene
            unset -nocomplain ::XnatPublish_NodeList
            unset -nocomplain ::XnatPublish_StorageNodeList
            XnatPublish_PopulateLists

            puts "storagenodes: $::XnatPublish_StorageNodeList"
            
            set nnodes [ llength $::XnatPublish_NodeList ]
            set nsnodes [ llength $::XnatPublish_StorageNodeList ]
            for { set n 0 } { $n < $nnodes } { incr n } {
                set node [ lindex $::XnatPublish_NodeList $n ]
                set snode [ lindex $::XnatPublish_StorageNodeList $n]
                if { $node != "" && $snode != "" } {
                    set savename [ $snode GetFileName ]                
                    if { $savename != "" } {
                        set cachename $savename
                        set cachename [ file tail $cachename ]
                        #--- put a timestamp on the name.

                        set cachename $::XnatPublish_Time-$cachename
                        set cachename [ file join $cdir $cachename ]

                        puts "Temporarily renaming $savename to $cachename"

                        #--- temporarily swap in cache filename
                        $snode SetFileName $cachename 
                        set res [ $snode WriteData $node]
                        
                        XnatPublish_CreateUploadCatalog
                        XnatPublish_ZipData
                        XnatPublish_Upload

                        #--- and then restore after writing.
                        $snode SetFileName $savename
                    }
                }
            }
        }
    } 
}

#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_CreateUploadCatalog { } {
}

#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_ZipData { } {
}


#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_Upload { } {
}

