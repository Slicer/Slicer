
#
# test of importing data wrapped in xcede 2.0 CATALOG xml
# vtk xml data parser mechanism
#

#######


#------------------------------------------------------------------------------
# main entry point...
#------------------------------------------------------------------------------
proc XcedeCatalogImport { xcedeFile } {

    #--- create a parser and parse the file
    set parser [vtkXMLDataParser New]
    $parser SetFileName $xcedeFile
    set retval [ $parser Parse ]

    if { $retval == 0 } {
        $parser Delete
        return $retval
    } else {
        #--- display to progress guage and status bar.
        set ::XcedeCatalog_mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
        set ::XcedeCatalog_progressGauge [$::XcedeCatalog_mainWindow GetProgressGauge]
        $::XcedeCatalog_progressGauge SetValue 0
        $::XcedeCatalog_mainWindow SetStatusText "Parsing $xcedeFile"

        #--- get the XCEDE root
        set root [$parser GetRootElement]

        #--- get the directory of the normalized xcede file.
        set ::XcedeCatalog_Dir [file dirname [file normalize $xcedeFile]]
        puts "Reading file $xcedeFile from $::XcedeCatalog_Dir..."

        #--- initialize some globals
        set ::XcedeCatalog(transformIDStack) ""
        set ::XcedeCatalog_HParent_ID ""
        set ::XcedeCatalogMrmlID(LHmodel) ""
        set ::XcedeCatalogMrmlID(RHmodel) ""
        set ::XcedeCatalog_MrmlID(anat2exf) ""
        set ::XcedeCatalog_MrmlID(FSBrain) ""
        set ::XcedeCatalog_MrmlID(ExampleFunc) ""
        set ::XcedeCatalog_MrmlID(StatisticsToBrainXform) ""
        set ::XcedeCatalog_MrmlID(StatFileList) ""
        set ::XcedeCatalog_AnnotationFiles ""
        set ::XcedeCatalog_NumberOfElements 0
        set ::XcedeCatalog_WhichElement 0
        set ::XcedeCatalog_RAS2RASTransformCreated 0
        array unset ::XcedeCatalog_MrmlID ""
        set ::XcedeCatalog(transformIDStack) ""
        set ::XcedeCatalog_HParent_ID ""
        
        #--- recursively import cataloged datasets 
        XcedeCatalogImportGetNumberOfElements $root

        #--- recursively import cataloged datasets 
        set ::XcedeCatalog(transformIDStack) ""
        set ::XcedeCatalog_HParent_ID ""
        set root [$parser GetRootElement]
        XcedeCatalogImportGetElement $root

        #--- if the catalog includes a brain.mgz, example_func.nii and
        #--- anat2exf.dat, we assume this is a FreeSurfer/FIPS catalog
        #--- and convert FreeSurfer tkRegister2's registration matrix
        #--- to a Slicer RAS2RAS registration matrix. 
        XcedeCatalogImportComputeFIPS2SlicerTransformCorrection

        #--- if the Correction transform node is created,
        #--- place all statistics volumes inside that.
        XcedeCatalogImportApplyFIPS2SlicerTransformCorrection
        
        #--- reset the feedback things
        $::XcedeCatalog_progressGauge SetValue 0
        $::XcedeCatalog_mainWindow SetStatusText ""
        
        #--- update main viewer and slice viewers.
        $::slicer3::MRMLScene Modified
        [$::slicer3::ApplicationGUI GetViewerWidget ] RequestRender
        [ [$::slicer3::ApplicationGUI GetMainSliceGUI0 ] GetSliceViewer ]  RequestRender
        [ [$::slicer3::ApplicationGUI GetMainSliceGUI1 ] GetSliceViewer ]  RequestRender
        [ [$::slicer3::ApplicationGUI GetMainSliceGUI2 ] GetSliceViewer ]  RequestRender
        
        #--- clean up.
        $parser Delete
        $::slicer3::MRMLScene SetErrorCode 0
        puts "...done reading $xcedeFile."
        return $retval
    }
}


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportGetNumberOfElements { element } {

  #--- save current parent locally
  set parent $::XcedeCatalog_HParent_ID

  #--- increment count
  incr ::XcedeCatalog_NumberOfElements

  #---TODO: probably don't need this...
  # leave a place holder in case we are a group node
  lappend ::XcedeCatalog(transformIDStack) "NestingMarker"

  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XcedeCatalogImportGetNumberOfElements $nestElement
  }

  #---TODO: probably don't need this...
  # strip away any accumulated transform ids
  while { $::XcedeCatalog(transformIDStack) != "" && [lindex $::XcedeCatalog(transformIDStack) end] != "NestingMarker" } {
    set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]
  }
  # strip away the nesting marker
  set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]

  # restore parent locally
  set ::XcedeCatalog_HParent_ID $parent

}



#------------------------------------------------------------------------------
# recursive routine to import all elements and their
# nested parts
#------------------------------------------------------------------------------
proc XcedeCatalogImportGetElement { element } {
  # save current parent locally
  set parent $::XcedeCatalog_HParent_ID

  #--- update progress bar 
  #set elementType [$element GetName]
  #$::XcedeCatalog_mainWindow SetStatusText "Parsing $elementType..."

  incr ::XcedeCatalog_WhichElement
  $::XcedeCatalog_progressGauge SetValue [expr 100 * $::XcedeCatalog_WhichElement / (1. * $::XcedeCatalog_NumberOfElements)]

  # import this element if it contains an entry of the correct type
  XcedeCatalogImportGetEntry $element 
  
  #---TODO: probably don't need this...
  # leave a place holder in case we are a group node
  lappend ::XcedeCatalog(transformIDStack) "NestingMarker"

  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XcedeCatalogImportGetElement $nestElement
  }

  #---TODO: probably don't need this...
  # strip away any accumulated transform ids
  while { $::XcedeCatalog(transformIDStack) != "" && [lindex $::XcedeCatalog(transformIDStack) end] != "NestingMarker" } {
    set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]
  }
  # strip away the nesting marker
  set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]

  # restore parent locally
  set ::XcedeCatalog_HParent_ID $parent
}




#------------------------------------------------------------------------------
# if the element is of a certain type of data
# (Transform, Volume, Model, etc.)
# parse the attributes of a node into a tcl array
# and then invoke the type-specific handler
#------------------------------------------------------------------------------
proc XcedeCatalogImportGetEntry {element } {

    #--- is this a catalog entry that contains a file or reference?
    set elementType [$element GetName]
    if { $elementType != "entry" && $elementType != "Entry" } {
        #--- only process catalog entry tags
        return 
    }
    
    #--- get attributes
    set nAtts [$element GetNumberOfAttributes]
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        set node($attName) [$element GetAttributeValue $i]
    } 

    
    #--- make sure the entry has a "uri" attribute by searching
    #--- all attributes to find one with a name that matches "uri"
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
        puts "can't find an attribute called URI in $element"
        return
    }
    
    $::XcedeCatalog_mainWindow SetStatusText "Loading $node(uri)..."
    
    #--- strip off the entry's relative path, and add the 
    #--- absolute path of the Xcede file to it.
    set fname [ file normalize $node($uriAttName) ]
    set plist [ file split $fname ]
    set len [ llength $plist ]
    set fname [ lindex $plist [ expr $len - 1 ] ]

    #set node($uriAttName) $::XcedeCatalog_Dir/$fname
    set node(localFileName)  $::XcedeCatalog_Dir/$fname

    #--- check to see if it's a remote file
    set cacheManager [$::slicer3::MRMLScene GetCacheManager]

    if {0} {
    #--- make sure the local file exists
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
  }

    #--- get the file format
    set gotformat 0
    set formatAttName ""
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        if { $attName == "format" || $attName == "Format" } {
            #--- mark as found and capture its case (upper or lower)
            set gotformat 1    
            set formatAttName $attName
        }
    }
    if { $gotformat == 0 } {
        puts "description for entry contains no format information. Cannot import entry."
        return
    }
    #--- what kind of node is it?
    set nodeType [ XcedeCatalogImportGetNodeType $node($formatAttName) ]
    if { $nodeType == "Unknown" } {
        puts "$node($formatAttName) is an unsupported format. Cannot import entry."
        return
    }
    #--- make sure the file is a supported format
    set fileformat [ XcedeCatalogImportFormatCheck $node($formatAttName) ]
    if { $fileformat == 0 } {
        puts "$node($formatAttName) is an unsupported format. Cannot import entry."
        return
    } elseif { $fileformat == 1 } {
#        puts "$node($formatAttName) can handle downloads automatically"
    } elseif { $fileformat == 2 } {
#        puts "$node($formatAttName) is something we have to download manually if it has a remote uri"
        if {$cacheManager != ""} {
            set isRemote [$cacheManager IsRemoteReference $node($uriAttName)]
            if {$isRemote == 1} {
                $::XcedeCatalog_mainWindow SetStatusText "Loading remote $node($uriAttName)..."
                # puts "Trying to find URI handler for $node($uriAttName)"
                set uriHandler [$::slicer3::MRMLScene FindURIHandler $node($uriAttName)]
                if {$uriHandler != ""} {
                    # for now, do a synchronous download
#                    puts "Found a file handler, doing a synchronous download from $node($uriAttName) to $node(localFileName)"
                    $uriHandler StageFileRead $node($uriAttName) $node(localFileName)
                } else {
                    puts "Unable to find a file handler for $node($uriAttName)"
                }
            }
        }
    }
    # puts "\tNow resetting uri to local file name so can read from disk"
    set node($uriAttName) $node(localFileName)

    #--- finally, create the node
    set handler XcedeCatalogImportEntry$nodeType
    
    if { [info command $handler] == "" } {
        set err [$::slicer3::MRMLScene GetErrorMessagePointer]
        $::slicer3::MRMLScene SetErrorMessage "$err\nno handler for $nodeType"
        $::slicer3::MRMLScene SetErrorCode 1
    }

    # call the handler for this element
    puts "importing $nodeType"
    $handler node
}


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportEntryVolume {node} {
  upvar $node n


    #--- ditch if there's no file in the uri
    if { ![info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryVolume: no uri specified for node $n(uri)"
        return
    }

    set centered 1
    set labelmap 0
    set singleFile 0
    set loadingOptions [expr $labelmap * 1 + $centered * 2 + $singleFile * 4]

    if { [info exists n(labelmap) ] } {
        set labelmap 1
    }
    
    set logic [$::slicer3::VolumesGUI GetLogic]
    if { $logic == "" } {
        puts "XcedeCatalogImportEntryVolume: Unable to access Volumes Logic. $n(uri) not imported."
        return
    }
    set volumeNode [$logic AddArchetypeVolume $n(uri) $n(name) $loadingOptions]
    if { $volumeNode == "" } {
        puts "XcedeCatalogImportEntryVolume: Unable to add Volume Node for $n(uri)."
        return
    }
    set volumeNodeID [$volumeNode GetID]

    if { [info exists n(description) ] } {
        $volumeNode SetDescription $n(description)
    }

    #--- try using xcede differently than the slicer2 xform descrption
    # use the current top of stack (might be "" if empty, but that's okay)
    #set transformID [lindex $::XcedeCatalog(transformIDStack) end]
    #$volumeNode SetAndObserveTransformNodeID $transformID

    set volumeDisplayNode [$volumeNode GetDisplayNode]
    if { $volumeDisplayNode == ""  } {
        puts "XcedeCatalogImportEntryVolume: Unable to access Volume Display Node for  $n(uri). Volume display not configured."
        return
    }

    #--- make some good guesses about what color node to set
    #--- and what window/level/threshold properties to set
    if { [ string first "stat" $n(uri) ] >= 0 } {
        #--- this is likely a statistical volume.
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodefMRIPA"
        #$volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeIron"
        $volumeDisplayNode SetAutoWindowLevel 0
        #$volumeDisplayNode SetThresholdType 1
    } elseif { [ string first "aseg" $n(uri) ] >= 0 } {
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

    #set logic [$::slicer3::VolumesGUI GetLogic]
    $logic SetActiveVolumeNode $volumeNode
                              
    #--- If volume freesurfer brain.mgz, set a global
    #--- This global is used as a reference volume for any
    #--- potential functional or statistical volumes
    #--- that may need to be registered to the brain
    #--- image via the anat2exf.register.dat xform.
    if { [ string first "brain.mgz" $n(uri) ] >= 0 } {
        set ::XcedeCatalog_MrmlID(FSBrain) $volumeNodeID
    }

    #--- If volume is an example_func image (used for
    #--- registration with the anatomical), set a global.
    if { [ string first "example_func" $n(uri) ] >= 0 } {
        set ::XcedeCatalog_MrmlID(ExampleFunc) $volumeNodeID
    }

    #--- If volume is a statistics volume, add to a
    #--- global list: these volumes will be put inside
    #--- a transform to register them to brain.mgz
    #--- if that transform is created.
    #--- this is weak; need a better test.
    if { [ string first "stat" $n(uri) ] >= 0 } {
        lappend ::XcedeCatalog_MrmlID(StatFileList) $volumeNodeID
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
proc XcedeCatalogImportEntryModel {node} {
  upvar $node n

    #--- ditch if there's no file in the uri
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryModel: no uri specified for node $n(uri). No model imported."
        return
    }

    set logic [$::slicer3::ModelsGUI GetLogic]
    if {$logic == "" } {
        puts "XcedeCatalogImportEntryModel: couldn't retrieve Models Logic. Model $n(name) not imported."
        return
    }
    set mnode [$logic AddModel $n(uri)]
    #--- maybe don't need this?
    #set snode [ $mnode GetModelStorageNode ]
    #set type [ $snode IsA ]
    #if { $type == "vtkMRMLFreeSurferModelStorageNode" } {
    #    $snode SetUseStripper 0
    #}

    if { $mnode == "" } {
        puts "XcedeCatalogImportEntryModel: couldn't created Model Node. Model $n(name) not imported."
        return
    }

    #--- set name and description
    if { [info exists n(description) ] } {
        $mnode SetDescription $n(description)
    }

    if { [info exists n(name) ] } {
        $mnode SetName $n(name)
    }

    #--- we assume catalogs will contain a single LH model
    #--- with which all LHoverlays will be associated.
    #--- and/or a single RH model with which RH overlays are associated.
    #--- left hemisphere models
    if { [ string first "lh." $n(uri) ] >= 0 } {
        if { $::XcedeCatalogMrmlID(LHmodel) == "" } {
            set ::XcedeCatalogMrmlID(LHmodel) [ $mnode GetID ]
        } else {
            puts "Warning: Xcede catalogs for slicer should contain at single LH model to which LH scalar overlays will be associated. This xcede file appears to contain multiple left hemisphere models: all scalar overlays will be associated with the first LH model."
        }
    }
    #--- right hemisphere models
    if { [ string first "rh." $n(uri) ] >= 0 } {
        if { $::XcedeCatalogMrmlID(RHmodel) == "" } {
            set ::XcedeCatalogMrmlID(RHmodel) [ $mnode GetID ]
        } else {
            puts "Warning: Xcede catalogs for slicer should contain at single RH model to which RH scalar overlays will be associated. This xcede file appears to contain multiple right hemisphere models: all scalar overlays will be associated with the first RH model."
        }

    }


}


#------------------------------------------------------------------------------
# helper function adds new transform node to mrml scene
# and returns the node id
#------------------------------------------------------------------------------
proc XcedeCatalogImportCreateIdentityTransformNode { name } {

    set tnode [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLLinearTransformNode ]
    $tnode SetScene $::slicer3::MRMLScene
    $tnode SetName $name
    $::slicer3::MRMLScene AddNode $tnode
    set tid [ $tnode GetID ]
    return $tid

}
    
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportSetMatrixFromURI { id filename }    {

    set tnode [ $::slicer3::MRMLScene GetNodeByID $id ]
    if { $tnode == "" } {
        puts "XcedeCatalogImportSetMatrixFromURI: transform ID=$id not found in scene. No elements set."
        return
    }

    set matrix [ $tnode GetMatrixTransformToParent ]
    if { $matrix == "" } {
        puts "XcedeCatalogImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
        return
    }
    
    #--- if filename contains ".register.dat" then we know what to do
    set check [ string first "register.dat" $filename ]
    if { $check < 0 } {
        puts "XcedeCatalogImportSetMatrixFromURI: $filename is unknown filetype, No elements set."
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
proc XcedeCatalogImportEntryTransform {node} {
  upvar $node n


    #--- ditch if there's no file in the uri
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryTransform: no uri specified for node $n(uri). No transform imported."
        return
    }

    #--- if filename contains ".register.dat" then we know what to do
    set check [ string first "register.dat" $n(uri) ]
    if { $check < 0 } {
        puts "XcedeCatalogImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
        tk_messageBox -message "XcedeCatalogImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
        return
    } 


    #--- add the node
    set tnode [ vtkMRMLLinearTransformNode New ]
    $tnode SetName $n(name)
    $::slicer3::MRMLScene AddNode $tnode
    set tid [ $tnode GetID ]

    if { $tid == "" } {
        puts "XcedeCatalogImportEntryTransform: unable to add Transform Node. No transform imported."
       tk_messageBox -message "XcedeCatalogImportEntryTransform: unable to add Transform Node. No transform imported."
        return
    }

    if { [info exists n(description) ] } {
        $tnode SetDescription $n(description)
    }

    #--- open register.dat file and read 
    set matrix [ vtkMatrix4x4 New ]
    set fid [ open $n(uri) r ]
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
        puts "XcedeCatalogImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
       tk_messageBox -message "XcedeCatalogImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
        return
    }

    $M DeepCopy $matrix
    $matrix Delete
    $tnode Delete
    
    #--- this is for help with FIPS registration correction
    if { $n(name) == "anat2exf" } {
        set ::XcedeCatalog_MrmlID(anat2exf) $tid
    }

}








#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportEntryOverlay {node} {
  upvar $node n

    #--- not really a node, per se...
    #--- ditch if there's no file in the uri
    
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryOverlay: no uri specified for node $n(name). No overlay imported."
        return
    }

    #--- what model node should these scalars be applied to?
    if { [ string first "lh." $n(uri) ] >= 0 } {
        if { ![info exists ::XcedeCatalogMrmlID(LHmodel) ] } {
            puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri). No overlay imported."
            return
        }
        set mid $::XcedeCatalogMrmlID(LHmodel)
        set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
    }



    if { [ string first "rh." $n(uri) ] >= 0 } {
        if { ![info exists ::XcedeCatalogMrmlID(RHmodel) ] } {
            puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri). No overlay imported."
            return
        }
        set mid $::XcedeCatalogMrmlID(RHmodel)
        set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
    }


    if { $mnode == "" } {
        puts "XcedeCatalogImportEntryOverlay: Model MRML Node corresponding to ID=$mid not found. No overlay imported."
        return
    }
    
    set logic [$::slicer3::ModelsGUI GetLogic]
    if { $logic == "" } {
        puts "XcedeCatalogImportEntryOverlay: cannot access Models Logic class. No overlay imported."
        return
    }
    
    #--- add the scalar to the node
    $logic AddScalar $n(uri) $mnode 

    #--- keep track of all annotation files loaded.
    if { [ string first "annot" $n(uri) ] >= 0 } {
        lappend ::XcedeCatalog_AnnotationFiles $n(uri)
    }
}



#------------------------------------------------------------------------------
# and returns the nodeType associated with that format
#------------------------------------------------------------------------------
proc XcedeCatalogImportGetNodeType { format } {
    if {$format == "FreeSurfer:mgz-1" } {
        return "Volume"
    } elseif {$format == "nifti:nii-1" } {
        return "Volume"
    } elseif { $format == "FreeSurfer:w-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:thickness-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:curv-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:avg_curv-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:sulc-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:area-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:annot-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:mgh-1" } {
        return "Volume"
    } elseif { $format == "FreeSufer:mat-1" } {
        return "Transform"
    } elseif { $format == "FreeSurfer:surface-1" } {
        return "Model"
    } elseif { $format == "FreeSurfer:overlay-1" } {
        return "Overlay"
    } elseif { $format == "FreeSurfer:matrix-1" } {
        return "Transform"
    }  else {
        return "Unknown"
    }
}


#------------------------------------------------------------------------------
# checking to see if Slicer can read this file format
#------------------------------------------------------------------------------
proc XcedeCatalogImportFormatCheck { format } {

    #--- check format against known formats
    #--- TODO: Once these values are formally defined for
    #--- all freesurfer data, we will have to change.
    #--- matrix-1 and overlay-1 are made up!
    #--- TODO: Add more as we know what their
    #--- XCEDE definitions are (analyze, etc.)
    
# return 1 if have a valid storage node that can deal with remote uri's, return 2 if need to synch download
    if {$format == "FreeSurfer:mgz-1" } {
        return 2
    } elseif {$format == "nifti:nii-1" } {
        return 2
    } elseif { $format == "FreeSurfer:w-1" } {
        return 1
    } elseif { $format == "FreeSurfer:thickness-1" } {
        return 1
    } elseif { $format == "FreeSurfer:curv-1" } {
        return 1
    } elseif { $format == "FreeSurfer:avg_curv-1" } {
        return 1
    } elseif { $format == "FreeSurfer:sulc-1" } {
        return 1
    } elseif { $format == "FreeSurfer:area-1" } {
        return 1
    } elseif { $format == "FreeSurfer:annot-1" } {
        return 1
    } elseif { $format == "FreeSurfer:mgh-1" } {
        return 2
    } elseif { $format == "FreeSurfer:surface-1" } {
        return 1
    } elseif { $format == "FreeSurfer:overlay-1" } {
        return 1
    } elseif { $format == "FreeSurfer:matrix-1" } {
        return 2
    }  else {
        return 0
    }

}




#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportComputeFIPS2SlicerTransformCorrection { } {

    if { $::XcedeCatalog_MrmlID(anat2exf) == "" } {
        return
    }
    if { $::XcedeCatalog_MrmlID(FSBrain) == "" } {
        return
    }
    if { $::XcedeCatalog_MrmlID(ExampleFunc) == "" } {
        return
    }

    #--- find a brain.mgz, an example_func.nii, and an anat2exf.register.dat.
    $::XcedeCatalog_mainWindow SetStatusText "Computing corrected registration matrix."
    #--- get required nodes from scene
    set v1 [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(FSBrain) ]
    set v2 [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(ExampleFunc) ]

    set anat2exfT [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(anat2exf) ]

    #--- get FSregistration matrix from node
    set anat2exf [ $anat2exfT GetMatrixTransformToParent ]

    #--- create a new node to hold the transform
    set ras2rasT [ vtkMRMLLinearTransformNode New ]
    $ras2rasT SetName StatisticsToBrainXform
    $::slicer3::MRMLScene AddNode $ras2rasT

    set ::XcedeCatalog_MrmlID(StatisticsToBrainXform) [ $ras2rasT GetID ]

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
    set ::XcedeCatalog_RAS2RASTransformCreated 1

}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportApplyFIPS2SlicerTransformCorrection { } {
    
    if { $::XcedeCatalog_RAS2RASTransformCreated == 1 } {
        $::XcedeCatalog_mainWindow SetStatusText "Applying registration matrix to statistics volumes"
        #--- move all the detected stats files under the new registration xform
        foreach id  $::XcedeCatalog_MrmlID(StatFileList) {
            set vnode [ $::slicer3::MRMLScene GetNodeByID $id ]
            $vnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(StatisticsToBrainXform) 
            $vnode Modified
        }
        #--- move the example func also into the new registration xform
        set vnode [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(ExampleFunc) ]
        $vnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(StatisticsToBrainXform) 
        $vnode Modified        
    }
}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCataLogImportTest  { } {
    #no op.
}
