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

    set handler [ $::slicer3::MRMLScene FindURIHandler "xnat://" ]
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
    #--- get the xnat handler
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
    
    if { $promptStat <= 0 } {
        #--- no reason for an error message here.
    } else { 
        #--- get info for publishing.
        #--- need to timestamp Scene name and all datasets.
        unset -nocomplain ::XnatPublish_Time
        set ::XnatPublish_Time [ XnatPublish_TimeStampString "" ]
        
        set ::XnatPublish_Tags [ $prompter GetResourceTag]
        if { $::XnatPublish_Tags == "" } {
            set ::XnatPublish_Tags "Slicer3_Upload"
        }
        set ::XnatPublish_CatalogID [ $prompter GetResourceSetName ]
        if { $::XnatPublish_CatalogID == "" } {
            set ::XnatPublish_CatalogID $::XnatPublish_Time
        }
        
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
            set scene $::slicer3::MRMLScene
            unset -nocomplain ::XnatPublish_NodeList
            unset -nocomplain ::XnatPublish_StorageNodeList
            XnatPublish_PopulateLists

            set nnodes [ llength $::XnatPublish_NodeList ]
            set nsnodes [ llength $::XnatPublish_StorageNodeList ]
            for { set n 0 } { $n < $nnodes } { incr n } {
                set node [ lindex $::XnatPublish_NodeList $n ]
                set snode [ lindex $::XnatPublish_StorageNodeList $n]
                if { $node != "" && $snode != "" } {
                    set saveURI [ $snode GetURI ]
                    set savename [ $snode GetFileName ]                
                    if { $savename != "" } {
                        set cachedname $savename
                        set cachedname [ file tail $cachedname ]
                        #--- put a timestamp on the name.

                        set cachedname $::XnatPublish_Time-$cachedname
                        set cachedname [ file join $cdir $cachedname ]

                        #--- temporarily swap in cache filename
                        $snode SetURI ""
                        $snode SetFileName $cachedname 
                        set res [ $snode WriteData $node]
                        
                        #--- update the CacheAndDataIOManagerGUI                        

                        #--- and then restore after writing.
                        $snode SetURI $saveURI
                        $snode SetFileName $savename
                    }
                }
            }

            unset -nocomplain ::XnatPublish_FileList
            XnatPublish_CreateUploadCatalog $cdir
            set z [ XnatPublish_CreateZippedArchive $cdir ]
            #--- create the xml for upload and zip data together.
            #--- upload to xnat database
            if { $z != "" } {
                set z [ file tail $z  ]
                set z [ file join $cdir $z ]
                puts "calling Upload of $z"
                XnatPublish_Upload $z
            }
        }
    } 
}

#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_CreateUploadCatalog { cdir } {

    if { $cdir != "" } {
        set fname [ file join $cdir "$::XnatPublish_CatalogID.xml" ]
        set fp [ open "$fname" w ]
        set headerStr "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        puts $fp $headerStr

        set catStr "<cat:Catalog ID=\"$::XnatPublish_CatalogID\" xmlns:arc=\"http://nrg.wustl.edu/arc\" xmlns:behavioral=\"http://nrg.wustl.edu/behavioral\" xmlns:cat=\"http://nrg.wustl.edu/catalog\" xmlns:cnda=\"http://nrg.wustl.edu/cnda\" xmlns:fs=\"http://nrg.wustl.edu/fs\" xmlns:ls2=\"http://nrg.wustl.edu/ls2\" xmlns:prov=\"http://www.nbirn.net/prov\" xmlns:wrk=\"http://nrg.wustl.edu/workflow\" xmlns:xdat=\"http://nrg.wustl.edu/security\" xmlns:xnat=\"http://nrg.wustl.edu/xnat\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
        puts $fp $catStr

        puts $fp "<cat:tags>"

        if { $::XnatPublish_Tags == "" } {
            puts $fp "<cat:tag>Slicer3</cat:tag>"
        } else {
            puts $fp "<cat:tag>$::XnatPublish_Tags</cat:tag>"
        }

        puts $fp "</cat:tags>"
        puts $fp "<cat:entries>"

        #--- get list of files in the cache directory with the timestamp.
        set ::XnatPublish_FileList [ glob -directory $cdir -type f *$::XnatPublish_Time*]
        set listlen [ llength $::XnatPublish_FileList ]

        for { set i 0 } { $i < $listlen } { incr i } {
            set f [ lindex $::XnatPublish_FileList $i ]
            set f [ file normalize $f ]
            set f [ file tail $f ]
            set f "./$f"
            set uriStr "<cat:entry URI=\"$f\" />"
            puts $fp $uriStr
        }

        puts $fp "</cat:entries>"        
        puts $fp "</cat:Catalog>"        
        close $fp
    }
    #--- update the CacheAndDataIOManagerGUI    
}



#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_CreateZippedArchive { cdir } {

    if { $cdir != "" } {
        #
        # look for zip on the system, if not found try the 
        # tcl fallback
        #
        set zip ""
        set candidates {
            "c:/cygwin/bin/zip.exe"
            /usr/bin/zip /bin/zip /usr/local/bin/zip
        }
        foreach c $candidates {
            if { [file exists $c] } {
                set zip $c
                break
            }
        }
        
        set listlen [ llength $::XnatPublish_FileList ]
        for { set i 0 } { $i < $listlen } { incr i } {
            set f [ lindex $::XnatPublish_FileList $i ]
            set f [ file normalize $f ]
            set f [ file tail $f ]
            lappend fl $f
        }

        if { [file exists $zip] } {
            set zip [ file normalize $zip ]
            set cwd [pwd]
            cd $cdir

            set listlen [ llength $fl ]
            for { set i 0 } { $i < $listlen } { incr i } {
                set f [ lindex $fl $i ]
                puts "zipping $f"
                exec $zip -b . $::XnatPublish_CatalogID $f
            }

            cd $cwd
            #--- update the CacheAndDataIOManagerGUI
            
        } else {
            set dialog [vtkKWMessageDialog New]
            $dialog SetParent $o(toplevel)
            $dialog SetStyleToMessage
            $dialog SetText "Cannot find zipfile reader, cannot process $path"
            $dialog Create
            $dialog Invoke
            $dialog Delete
            return ""
        }
        
        #--- did the zip archive get created?
        set chk [ file join $cdir $::XnatPublish_CatalogID.zip ]
        if { [file exists $chk] } {
            return $chk
        } else {
            set dialog [vtkKWMessageDialog New]
            $dialog SetParent $o(toplevel)
            $dialog SetStyleToMessage
            $dialog SetText "Error creating zipfile. nothing to upload."
            $dialog Create
            $dialog Invoke
            $dialog Delete
            return ""
        }
    }

}


#------------------------------------------------------------------------------
#---
#------------------------------------------------------------------------------
proc XnatPublish_Upload { zfile } {

    if { $zfile != "" } {

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

        set u [ $prompter GetUsername ]
        set p [ $prompter GetPassword ]
        set h [ $prompter GetHostName ]
        set id [ $prompter GetMRsessionID ]
        if { [file exists $zfile] } {
            #--- schedule the upload
            puts "calling with $zfile $u $p $h $id"
            $handler StageFileWrite $zfile $u $p $h $id
        } else {
            set dialog [ vtkKWMessageDialog New ]
            $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
            $dialog SetStyleToMessage
            $dialog SetText "XnatPublishScene: can't find Permission Prompter for XNAT. No data was published."
            $dialog Create
            $dialog Invoke
            $dialog Delete
            return -1
        }
        return 1
    }
    return -1;
}


