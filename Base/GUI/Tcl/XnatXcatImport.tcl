
#------------------------------------------------------------------------------
# main entry point...
#------------------------------------------------------------------------------
proc XnatXcatImport { xnatxcatFile } {

    #--- create a parser and parse the file

    set parser [vtkXMLDataParser New]
    $parser SetFileName $xnatxcatFile
    set retval [ $parser Parse ]

    if { $retval == 0 } {
        $parser Delete
        return $retval
    } else {
        #--- display to progress guage and status bar.
        set ::XnatXcat_mainWindow [ $::slicer3::ApplicationGUI GetMainSlicerWindow]
        set ::XnatXcat_ProgressGauge [ $::XnatXcat_mainWindow GetProgressGauge ]
        $::XnatXcat_ProgressGauge SetValue 0
        $::XnatXcat_mainWindow SetStatusText "Parsing $xnatxcatFile"

        #--- initialize some globals
        array unset ::XnatXcat_MrmlID ""
        set ::XnatXcat_HParent_ID ""
        set ::XnatXcat_NumberOfElements 0
        set ::XnatXcat_WhichElement 0
        set ::XnatXcat_MrmlID(Volumes) ""

        set ::XnatXcat(transformIDStack) ""
        set ::XnatXcat_MrmlID(LHmodel) ""
        set ::XnatXcat_MrmlID(RHmodel) ""
        set ::XnatXcat_MrmlID(anat2exf) ""
        set ::XnatXcat_MrmlID(FSBrain) ""
        set ::XnatXcat_MrmlID(ExampleFunc) ""
        set ::XnatXcat_MrmlID(StatisticsToBrainXform) ""
        set ::XnatXcat_MrmlID(StatFileList) ""
        set ::XnatXcat_AnnotationFiles ""
        set ::XnatXcat_NumberOfElements 0
        set ::XnatXcat_WhichElement 0
        set ::XnatXcat_RAS2RASTransformCreated 0
        set ::XnatXcat(transformIDStack) ""

        #--- get the root
        set root [ $parser GetRootElement ]
        XnatXcatImportGetNumberOfElements $root

        #--- get the directory of the normalized catalog.

        if { $::XnatXcat_NumberOfElements > 0 } {

            #--- get the directory
            set ::XnatXcat_Dir [ file dirname [ file normalize $xnatxcatFile]]

            #--- recursively import datasets
            set root [ $parser GetRootElement ]
            XnatXcatImportGetElement $root

        #--- if the catalog includes a brain.mgz, example_func.nii and
        #--- anat2exf.dat, we assume this is a FreeSurfer/FIPS catalog
        #--- and convert FreeSurfer tkRegister2's registration matrix
        #--- to a Slicer RAS2RAS registration matrix. 
        XnatXcatImportComputeFIPS2SlicerTransformCorrection

        #--- if the Correction transform node is created,
        #--- place all statistics volumes inside that.
        XnatXcatImportApplyFIPS2SlicerTransformCorrection
        
            #--- reset feedback things
            $::XnatXcat_ProgressGauge SetValue 0
            $::XnatXcat_mainWindow SetStatusText ""

            #--- update main viewer and slice viewers
            $::slicer3::MRMLScene Modified
            [$::slicer3::ApplicationGUI GetViewerWidget ] RequestRender
            [ [$::slicer3::ApplicationGUI GetMainSliceGUI0 ] GetSliceViewer ]  RequestRender
            [ [$::slicer3::ApplicationGUI GetMainSliceGUI1 ] GetSliceViewer ]  RequestRender
            [ [$::slicer3::ApplicationGUI GetMainSliceGUI2 ] GetSliceViewer ]  RequestRender
        
            #--- clean up.
            $parser Delete
            $::slicer3::MRMLScene SetErrorCode 0
            return $retval            
        }
    }

}


#------------------------------------------------------------------------------
# main entry point...
#------------------------------------------------------------------------------
proc XnatXcatImportGetNumberOfElements { element } {

 #--- save current parent locally
  set parent $::XnatXcat_HParent_ID

  #--- increment count
  incr ::XnatXcat_NumberOfElements

  # process all the sub nodes
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XnatXcatImportGetNumberOfElements $nestElement
  }

  # restore parent locally
  set ::XnatXcat_HParent_ID $parent

}



#------------------------------------------------------------------------------
# recursive routine to import all elements and their
# nested parts
#------------------------------------------------------------------------------
proc XnatXcatImportGetElement { element } {
    
 # save current parent locally
  set parent $::XnatXcat_HParent_ID

  #--- update progress bar 
  #set elementType [$element GetName]
  #$::XnatXcat_mainWindow SetStatusText "Parsing $elementType..."

  incr ::XnatXcat_WhichElement
  set val [expr 100 * $::XnatXcat_WhichElement / (1. * $::XnatXcat_NumberOfElements)]
  $::XnatXcat_ProgressGauge SetValue $val

  # import this element if it contains an entry of the correct type
  XnatXcatImportGetEntry $element 
  
  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XnatXcatImportGetElement $nestElement
  }

  # restore parent locally
  set ::XnatXcat_HParent_ID $parent
}


#------------------------------------------------------------------------------
# recursive routine to import all metaFields
#------------------------------------------------------------------------------
proc XnatXcatImportGetMetaFields { element node } {

    set parent $::XnatXcat_ParentEntry

    set nNested [$element GetNumberOfNestedElements ]
    for {set i 0} {$i < $nNested} {incr i} {
        set nestElement [$element GetNestedElement $i]
        set elementType [$element GetName]
        if { $elementType != "cat:metaField" && $elementType != "cat:METAFIELD" } {
            #--- get attributes
            set nAtts [$element GetNumberOfAttributes]
            for {set i 0} {$i < $nAtts} {incr i} {
                set attName [$element GetAttributeName $i]
                set node($attName) [$element GetAttributeValue $i]
                puts "got $node($attName)"
            }
        }
    }
    # restore parent locally
    set ::XnatXcat_ParentEntry $parent
}




#------------------------------------------------------------------------------
# if the element is of a certain type of data
# (Transform, Volume, Model, etc.)
# parse the attributes of a node into a tcl array
# and then invoke the type-specific handler
#------------------------------------------------------------------------------
proc XnatXcatImportGetEntry { element } {


    #---
    #--- Check to see if this is an item of interest.
    #---
    set elementType [$element GetName]
    if { $elementType != "cat:entry" && $elementType != "cat:Entry" && $elementType != "cat:catalog" && $elementType != "cat:Catalog" } {
        return 
    }

    #--- Set the MRsessionID in the XNAT PermissionPrompter
    if { $elementType == "cat:Catalog" || $elementType == "cat:catalog" } {
        set nAtts [$element GetNumberOfAttributes]
        for {set i 0} {$i < $nAtts} {incr i} {
            set attName [$element GetAttributeName $i]
            if { $attName == "ID" } {
                set handler [ $::slicer3::MRMLScene FindURIHandler "xnat://" ]
                if { $handler != "" } {
                    set prompter [ $handler GetPermissionPrompter ]
                    if { $prompter != "" } {
                        $prompter SetMRsessionID [ $element GetAttributeValue $i]
                        puts "Set SessionID to [$element GetAttributeValue $i]"
                    }
                }
            }
        }
        return
    }

    #--- Parse entries.
    #--- get attributes and values of the entry
    #--- so far, we will use only a few in xnatxcats:
    #--- uri, and name. We store values for attributes 
    #--- in node($attName), notably node(URI)
    #--- and node(name) 
    set nAtts [$element GetNumberOfAttributes]
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        set node($attName) [$element GetAttributeValue $i]
    }

    #--- Don't continue if we don't have both a
    #--- node(uri) and node(name).
    #--- The uri should contain a link to the file, and hopefully
    #--- ticket information which encapsulates permissions to
    #--- access the data.
    set hasuri 0
    set uriAttName ""
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        if { $attName == "uri" || $attName == "URI" } {
            #--- mark as found and capture its case (upper or lower)
            set hasuri 1
            set uriAttName $attName
        }
    }
    if { $hasuri == 0 } {
        XnatXcatImportMessage "can't find an attribute called URI in $element"
        return
    }    

    #--- node(name) should define the filename
    set hasname 0
    set nameAtt ""
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        if { $attName == "name" || $attName == "Name" } {
            #--- mark as found and capture its case (upper or lower)
            set hasname 1
            set nameAtt $attName
        }
    }
    if { $hasname == 0 } {
        XnatXcatImportMessage "can't find an attribute called 'name' in $element"
        return
    }    
    set ext [ file extension $node(name) ]
    if { $ext == ".xml" } {
        puts "skipping entry $node(name)"
        return
    }


    #---
    #--- TODO: grab any metaField data present for this entry.
    #---
    $::XnatXcat_mainWindow SetStatusText "Working on $node($uriAttName)..."
    #--- strip off the entry's relative path, and add the 
    #--- absolute path of the Xcede file to it.
    set fname [ file normalize $node(name) ]
    set plist [ file split $fname ]
    set len [ llength $plist ]
    set fname [ lindex $plist [ expr $len - 1 ] ]

    set node(localFileName)  $::XnatXcat_Dir/$fname

    #--- try to pull the XNAT host out of the uri.
    set i_end [ string first  ".org" $node($uriAttName) ]
    if { $i_end < 0 } {
        set i_end [ string first  ".com" $node($uriAttName) ]
        if { $i_end < 0 } {
            set i_end [ string first  ".net" $node($uriAttName)  ]
            if { $i_end < 0 } {
                set i_end [ string first  ".gov" $node($uriAttName)  ]
            }
        }
    }
    if { $i_end >= 0 } {
        set host [ string range $node($uriAttName) 0 [expr $i_end + 4 ]]
        set handler [ $::slicer3::MRMLScene FindURIHandler "xnat://" ]
        if { $handler != "" } {
            set prompter [ $handler GetPermissionPrompter ]
            if { $prompter != "" } {
                $prompter SetHostName $host
                puts "Set HostName to $host"
            }
        }
    }


    #--- get cache manager in case the file is remote resource
    set cacheManager [$::slicer3::MRMLScene GetCacheManager]
    $cacheManager MapFileToURI $node($uriAttName) $node(localFileName)

    #--- test:
    set testy [$cacheManager GetFilenameFromURI $node($uriAttName)]
    
    #--- what kind of node is it?
    set nodeType [ XnatXcatImportGetNodeType $node(name) ]

    if { $nodeType == "Unknown" } {
        puts "$node($uriAttName) is an unsupported format. Cannot import entry."
        return
    }

    #--- make sure the file is a supported format
    set gotformat 0
    set fileformat [ XnatXcatImportFormatCheck $node(name) ]
    if { $fileformat == 0 } {
        puts "$node($uriAttName) is an unsupported format. Cannot import entry."
        return
    } elseif { $fileformat == 1 } {
        if {$cacheManager != ""} {
            set isRemote [$cacheManager IsRemoteReference $node($uriAttName)]

            #--- If the uri points to a local file, just read off of disk.
            if {$isRemote == 0} {
                set node(localFileName) [ file normalize $node(localFileName) ]
                if {![ file exists $node(localFileName) ] } {
                    puts "can't find file $node(localFileName)."
                    return
                }
                #--- make sure the local file is a file (and not a directory)
                if { ![file isfile $node(localFileName) ] } {
                    puts "$node(localFileName) doesn't appear to be a file. Not trying to import."
                    return
                }
                # it's a local file, so reset the uri
                set node($uriAttName) $node(localFileName)
            }
        }
    } elseif { $fileformat == 2 } {
        if {$cacheManager != ""} {
            set isRemote [$cacheManager IsRemoteReference $node($uriAttName)]
            if {$isRemote == 1} {
                $::XnatXcat_mainWindow SetStatusText "Downloading remote $node($uriAttName)..."
                # puts "Trying to find URI handler for $node($uriAttName)"
                set uriHandler [$::slicer3::MRMLScene FindURIHandler $node($uriAttName)]
                if {$uriHandler != ""} {
                    # for now, do a synchronous download
                    $uriHandler StageFileRead $node($uriAttName) $node(localFileName)
                } else {
                    puts "Unable to find a file handler for $node($uriAttName)"
                }
            }
        }
        set node($uriAttName) $node(localFileName)
    }

    #--- finally, create the node
    set handler XnatXcatImportEntry$nodeType
    
    if { [info command $handler] == "" } {
        set err [$::slicer3::MRMLScene GetErrorMessagePointer]
        $::slicer3::MRMLScene SetErrorMessage "$err\nno handler for $nodeType"
        $::slicer3::MRMLScene SetErrorCode 1
    }

    # call the handler for this element
    $handler node

    
}




#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportEntryVolume {node} {
  upvar $node n


    #--- ditch if there's no file in the uri
    if { ![info exists n(URI) ] } {
        puts "XnatXcatImportEntryVolume: no uri specified for node $n(URI)"
        return
    }

    set centered 1
    set labelmap 0
    set singleFile 0
    set autoLevel 1

    if { [info exists n(labelmap) ] } {
        set labelmap 1
    }
    if { [ string first "stat" $n(name) ] >= 0 } {
        # set autoLevel 0
    }
    set loadingOptions [expr $labelmap * 1 + $centered * 2 + $singleFile * 4 + $autoLevel * 8]
 
    set logic [$::slicer3::VolumesGUI GetLogic]
    if { $logic == "" } {
        puts "XnatXcatImportEntryVolume: Unable to access Volumes Logic. $n(URI) not imported."
        return
    }
#    puts "Calling volumes logic add archetype scalar volume with uri = $n(URI) and name = $n(name)"
#    set volumeNode [$logic AddArchetypeVolume $n(URI) $n(name) $loadingOptions]
    set volumeNode [$logic AddArchetypeScalarVolume $n(URI) $n(name) $loadingOptions]
    if { $volumeNode == "" } {
        puts "XnatXcatImportEntryVolume: Unable to add Volume Node for $n(URI)."
        return
    }

    set volumeNodeID [$volumeNode GetID]

    if { [info exists n(description) ] } {
        $volumeNode SetDescription $n(description)
    }

    #--- try using xcede differently than the slicer2 xform descrption
    # use the current top of stack (might be "" if empty, but that's okay)
    #set transformID [lindex $::XnatXcat(transformIDStack) end]
    #$volumeNode SetAndObserveTransformNodeID $transformID

    set volumeDisplayNode [$volumeNode GetDisplayNode]
    if { $volumeDisplayNode == ""  } {
        puts "XnatXcatImportEntryVolume: Unable to access Volume Display Node for  $n(URI). Volume display not configured."
        return
    }

    #--- make some good guesses about what color node to set
    #--- and what window/level/threshold properties to set
    if { [ string first "stat" $n(name) ] >= 0 } {
        #--- this is likely a statistical volume.
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodefMRIPA"
        #$volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeIron"
        $volumeDisplayNode SetAutoWindowLevel 0
        #$volumeDisplayNode SetThresholdType 1
    } elseif { [ string first "aseg" $n(name) ] >= 0 } {
        #--- this is likely a freesurfer label map volume
        set colorLogic [ $::slicer3::ColorGUI GetLogic ]
        if { $colorLogic != "" } {
            $volumeDisplayNode SetAndObserveColorNodeID [$colorLogic GetDefaultFreeSurferLabelMapColorNodeID ]
        } else {
            $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
        }
        #$volumeDisplayNode SetAutoWindowLevel 0
        #$volumeDisplayNode SetAutoThreshold 0
        
    } else {
        #--- assume this is a greyscale volume
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
        if { $labelmap == 0 } {
            $volumeDisplayNode SetAutoWindowLevel 1
            $volumeDisplayNode SetAutoThreshold 1
        }
    }
    # puts "\tFor volume [$volumeNode GetName], on volume display node [$volumeDisplayNode GetID], observing colour node [$volumeDisplayNode GetColorNodeID]. Display node window = [$volumeDisplayNode GetWindow], level = [$volumeDisplayNode GetLevel]"
    $logic SetActiveVolumeNode $volumeNode
                              
    #--- If volume freesurfer brain.mgz, set a global
    #--- This global is used as a reference volume for any
    #--- potential functional or statistical volumes
    #--- that may need to be registered to the brain
    #--- image via the anat2exf.register.dat xform.
    if { [ string first "brain.mgz" $n(name) ] >= 0 } {
        set ::XnatXcat_MrmlID(FSBrain) $volumeNodeID
    }

    #--- If volume is an example_func image (used for
    #--- registration with the anatomical), set a global.
    if { [ string first "example_func" $n(name) ] >= 0 } {
        set ::XnatXcat_MrmlID(ExampleFunc) $volumeNodeID
    }

    #--- If volume is a statistics volume, add to a
    #--- global list: these volumes will be put inside
    #--- a transform to register them to brain.mgz
    #--- if that transform is created.
    #--- this is weak; need a better test.
    if { [ string first "stat" $n(name) ] >= 0 } {
        lappend ::XnatXcat_MrmlID(StatFileList) $volumeNodeID
    }

      if { $labelmap } {
      [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetReferenceActiveLabelVolumeID [$volumeNode GetID]
    } else {
      [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
    }
    [$::slicer3::VolumesGUI GetApplicationLogic] PropagateVolumeSelection

}
    

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportEntryModel {node} {
  upvar $node n

    #--- ditch if there's no file in the uri
    if { ! [info exists n(URI) ] } {
        puts "XnatXcatImportEntryModel: no uri specified for node $n(URI). No model imported."
        return
    }

    set logic [$::slicer3::ModelsGUI GetLogic]
    if {$logic == "" } {
        puts "XnatXcatImportEntryModel: couldn't retrieve Models Logic. Model $n(name) not imported."
        return
    }

    #--- is the reference remote?
    set mnode [$logic AddModel $n(URI) ]

    #--- maybe don't need this?
    #set snode [ $mnode GetModelStorageNode ]
    #set type [ $snode IsA ]
    #if { $type == "vtkMRMLFreeSurferModelStorageNode" } {
    #    $snode SetUseStripper 0
    #}

    if { $mnode == "" } {
        puts "XnatXcatImportEntryModel: couldn't created Model Node. Model $n(name) not imported."
        return
    }

    #--- set name and description
    if { [info exists n(description) ] } {
        $mnode SetDescription $n(description)
    }

    #--- we assume catalogs will contain a single LH model
    #--- with which all LHoverlays will be associated.
    #--- and/or a single RH model with which RH overlays are associated.
    #--- left hemisphere models
    if { [ string first "lh." $n(name) ] >= 0 } {
        if { $::XnatXcat_MrmlID(LHmodel) == "" } {
            set id [ $mnode GetID]
            set ::XnatXcat_MrmlID(LHmodel) $id
        } else {
            puts "Warning: Xcede catalogs for slicer should contain at single LH model to which LH scalar overlays will be associated. This xcede file appears to contain multiple left hemisphere models: all scalar overlays will be associated with the first LH model."
        }
    }
    #--- right hemisphere models
    if { [ string first "rh." $n(name) ] >= 0 } {
        if { $::XnatXcat_MrmlID(RHmodel) == "" } {
            set id [ $mnode GetID]
            set ::XnatXcat_MrmlID(RHmodel) $id
        } else {
            puts "Warning: Xcede catalogs for slicer should contain at single RH model to which RH scalar overlays will be associated. This xcede file appears to contain multiple right hemisphere models: all scalar overlays will be associated with the first RH model."
        }
    }
}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportEntryOverlay {node} {
  upvar $node n

    #--- not really a node, per se...
    #--- ditch if there's no file in the uri
    
    if { ! [info exists n(URI) ] } {
        puts "XnatXcatImportEntryOverlay: no uri specified for node $n(name). No overlay imported."
        return
    }

    set mid ""
    set mnode ""

    #--- what model node should these scalars be applied to?
    if { [ string first "lh." $n(name) ] >= 0 } {
        if { ![info exists ::XnatXcat_MrmlID(LHmodel) ] } {
            puts "XnatXcatImportEntryOverlay: no model ID specified for overlay $n(URI). No overlay imported."
            return
        }
        set mid $::XnatXcat_MrmlID(LHmodel)
        set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
    }



    if { [ string first "rh." $n(name) ] >= 0 } {
        if { ![info exists ::XnatXcat_MrmlID(RHmodel) ] } {
            puts "XnatXcatImportEntryOverlay: no model ID specified for overlay $n(URI). No overlay imported."
            return
        }
        set mid $::XnatXcat_MrmlID(RHmodel)
        set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
    }


    if { $mnode == "" } {
        puts "XnatXcatImportEntryOverlay: Model MRML Node corresponding to ID=$mid not found. No overlay imported."
        return
    }
    
    set logic [$::slicer3::ModelsGUI GetLogic]
    if { $logic == "" } {
        puts "XnatXcatImportEntryOverlay: cannot access Models Logic class. No overlay imported."
        return
    }
    
    #--- add the scalar to the node
    $logic AddScalar $n(URI) $mnode 

    #--- keep track of all annotation files loaded.
    if { [ string first "annot" $n(name) ] >= 0 } {
        lappend ::XnatXcat_AnnotationFiles $n(name)
    }
}




#------------------------------------------------------------------------------
# helper function adds new transform node to mrml scene
# and returns the node id
#------------------------------------------------------------------------------
proc XnatXcatImportCreateIdentityTransformNode { name } {

    set tnode [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLLinearTransformNode ]
    $tnode SetScene $::slicer3::MRMLScene
    $tnode SetName $name
    $::slicer3::MRMLScene AddNode $tnode
    set tid [ $tnode GetID ]
    return $tid

}
    
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportSetMatrixFromURI { id filename }    {

    set tnode [ $::slicer3::MRMLScene GetNodeByID $id ]
    if { $tnode == "" } {
        puts "XnatXcatImportSetMatrixFromURI: transform ID=$id not found in scene. No elements set."
        return
    }

    set matrix [ $tnode GetMatrixTransformToParent ]
    if { $matrix == "" } {
        puts "XnatXcatImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
        return
    }
    
    #--- if filename contains ".register.dat" then we know what to do
    set check [ string first "register.dat" $filename ]
    if { $check < 0 } {
        puts "XnatXcatImportSetMatrixFromURI: $filename is unknown filetype, No elements set."
        return
    }

    #--- open register.dat file and read 
    set fid [ open $filename r ]
    set row 0
    set col 0
    while { ! [ eof $fid ] } {
        gets $fid line
        set llen [ llength $line ]
        #--- grab only lines that have matrix elements
        if { $llen == 4 } {
            set element [ expr [ lindex $line 0 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 1 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 2 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 3 ] ]
            $matrix SetElement $row $col $element
            incr row
            set col 0
        }
    }


}
#------------------------------------------------------------------------------
#-- TODO: something in this proc is causing debug leaks. WHAT?
#------------------------------------------------------------------------------
proc XnatXcatImportEntryTransform {node} {
  upvar $node n


    #--- ditch if there's no file in the uri
    if { ! [info exists n(URI) ] } {
        puts "XnatXcatImportEntryTransform: no uri specified for node $n(URI). No transform imported."
        return
    }

    #--- if filename contains ".register.dat" then we know what to do
    set check [ string first "register.dat" $n(name) ]
    if { $check < 0 } {
        puts "XnatXcatImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
        tk_messageBox -message "XnatXcatImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
        return
    } 


    #--- add the node
    set tnode [ vtkMRMLLinearTransformNode New ]
    $tnode SetName $n(name)
    $::slicer3::MRMLScene AddNode $tnode
    set tid [ $tnode GetID ]

    if { $tid == "" } {
        puts "XnatXcatImportEntryTransform: unable to add Transform Node. No transform imported."
       tk_messageBox -message "XnatXcatImportEntryTransform: unable to add Transform Node. No transform imported."
        return
    }

    if { [info exists n(description) ] } {
        $tnode SetDescription $n(description)
    }

    #--- open register.dat file and read 
    set matrix [ vtkMatrix4x4 New ]
    set fid [ open $n(URI) r ]
    set row 0
    set col 0
    while { ! [ eof $fid ] } {
        gets $fid line
        set llen [ llength $line ]
        #--- grab only lines that have matrix elements
        if { $llen == 4 } {
            set element [ expr [ lindex $line 0 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 1 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 2 ] ]
            $matrix SetElement $row $col $element
            incr col
            set element [ expr [ lindex $line 3 ] ]
            $matrix SetElement $row $col $element
            incr row
            set col 0
        }
    }
    close $fid
    
    #--- read the uri and translate matrix element values into place.
    set M [ $tnode GetMatrixTransformToParent ]
    if { $M == "" } {
        puts "XnatXcatImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
       tk_messageBox -message "XnatXcatImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
        return
    }

    $M DeepCopy $matrix
    $matrix Delete
    $tnode Delete
    
    #--- this is for help with FIPS registration correction
    set check [ string first "anat2exf" $n(name) ]
    if { $check >= 0 } {
        set ::XnatXcat_MrmlID(anat2exf) $tid
    }

}









#------------------------------------------------------------------------------
# and returns the nodeType associated with that format
#------------------------------------------------------------------------------
proc XnatXcatImportGetNodeType { fname } {

    set ext [ file extension $fname ]

    #--- brawny, not brainy, approach.
    
    if {$fname == "aseg.mgz" } {
        return "Volume"
    } elseif { $fname == "aparc+aseg.mgz" } {
        return "Volume"
    } elseif {$ext == ".mgz" } {
        return "Volume"
    } elseif {$ext == ".nii" } {
        return "Volume"
    } elseif { $ext == ".annot" } {
        return "Overlay"
    } elseif { $ext == ".dat" } {
        return "Transform"
    } elseif { $ext == ".pial" } {
        return "Model"
    } elseif { $ext == ".vtk" } {
        return "Model"
    }  else {
        return "Unknown"
    }
}


#------------------------------------------------------------------------------
# checking to see if Slicer can read this file format
#------------------------------------------------------------------------------
proc XnatXcatImportFormatCheck { fname } {

    #--- check format against known formats
    
    set ext [ file extension $fname ]
    
# return 1 if have a valid storage node that can deal with remote uri's, return 2 if need to synch download
    if {$fname == "aseg.mgz" } {
        return 1
    } elseif { $fname == "aparc+aseg.mgz" } {
        return 1
    } elseif {$ext == ".mgz" } {
        return 1
    } elseif {$ext == ".nii" } {
        return 1
    } elseif { $ext == ".annot" } {
        return 1
    } elseif { $ext == ".dat" } {
        return 2
    } elseif { $ext == ".pial" } {
        return 1
    } elseif { $ext == ".vtk" } {
        return 1
    }  else {
        return 0
    }
}




#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportComputeFIPS2SlicerTransformCorrection { } {

    if { $::XnatXcat_MrmlID(anat2exf) == "" } {
        puts "cant find anat2exf"
        return
    }
    if { $::XnatXcat_MrmlID(FSBrain) == "" } {
        puts "can't find FSBrain"
        return
    }
    if { $::XnatXcat_MrmlID(ExampleFunc) == "" } {
        puts "can't find exampelFun"
        return
    }


    #--- find a brain.mgz, an example_func.nii, and an anat2exf.register.dat.
    $::XnatXcat_mainWindow SetStatusText "Computing corrected registration matrix."
    #--- get required nodes from scene
    set v1 [ $::slicer3::MRMLScene GetNodeByID $::XnatXcat_MrmlID(FSBrain) ]
    set v2 [ $::slicer3::MRMLScene GetNodeByID $::XnatXcat_MrmlID(ExampleFunc) ]

    set anat2exfT [ $::slicer3::MRMLScene GetNodeByID $::XnatXcat_MrmlID(anat2exf) ]

    #--- get FSregistration matrix from node
    set anat2exf [ $anat2exfT GetMatrixTransformToParent ]

    #--- create a new node to hold the transform
    set ras2rasT [ vtkMRMLLinearTransformNode New ]
    $ras2rasT SetName StatisticsToBrainXform
    $::slicer3::MRMLScene AddNode $ras2rasT

    set ::XnatXcat_MrmlID(StatisticsToBrainXform) [ $ras2rasT GetID ]

    #--- get access to methods we need thru logic
    set volumesLogic [ $::slicer3::VolumesGUI GetLogic ]

    #--- compute some matrices.
    set mat [ vtkMatrix4x4 New]
    $volumesLogic ComputeTkRegVox2RASMatrix $v1 $mat
    $volumesLogic TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix $v1 $v2 $anat2exf $mat

    #--- this inverse will register statistics to the brain.mgz
    $mat Invert

    #--- now have matrix. put it in transform.
    [ $ras2rasT GetMatrixTransformToParent ] DeepCopy $mat

    #--- ok -- now manually put your volume in the ras2rasT transform node.
    $mat Delete
    $ras2rasT Delete

    #--- mark the transform as created 
    set ::XnatXcat_RAS2RASTransformCreated 1

}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportApplyFIPS2SlicerTransformCorrection { } {
    
    if { $::XnatXcat_RAS2RASTransformCreated == 1 } {
        $::XnatXcat_mainWindow SetStatusText "Applying registration matrix to statistics volumes"
        #--- move all the detected stats files under the new registration xform
        foreach id  $::XnatXcat_MrmlID(StatFileList) {
            set vnode [ $::slicer3::MRMLScene GetNodeByID $id ]
            $vnode SetAndObserveTransformNodeID $::XnatXcat_MrmlID(StatisticsToBrainXform) 
            $vnode Modified
        }
        #--- move the example func also into the new registration xform
        set vnode [ $::slicer3::MRMLScene GetNodeByID $::XnatXcat_MrmlID(ExampleFunc) ]
        $vnode SetAndObserveTransformNodeID $::XnatXcat_MrmlID(StatisticsToBrainXform) 
        $vnode Modified        
    }
}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportMessage { msg } {

    set dialog [ vtkKWMessageDialog New ]
    $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    $dialog SetStyleToMessage
    $dialog SetText $msg
    $dialog Create
    $dialog Invoke
    $dialog Delete
}


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XnatXcatImportTest  { } {
    #no op.
}
