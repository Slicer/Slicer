
#
# test of importing data wrapped in xcede 2.0 xml
# vtk xml data parser mechanism
#

#######
#
# for debugging - run the command when the script is read...
#
#after idle {
#  puts -nonewline "importing..."
#  XcedeCatalogImport c:/cygwin/home/wjp/data/fBIRN-AHM2007/fBIRN-AHM2007.xcede
#  set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
#  [$viewer GetMainViewer] Reset
#  puts "done"
#}


#------------------------------------------------------------------------------
# main entry point...
#------------------------------------------------------------------------------
proc XcedeCatalogImport {xcedeFile} {

  #--- create a parser and parse the file
  set parser [vtkXMLDataParser New]
  $parser SetFileName $xcedeFile
  $parser Parse

  #--- display to progress guage and status bar.
  set ::XcedeCatalog_mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  set ::XcedeCatalog_progressGauge [$::XcedeCatalog_mainWindow GetProgressGauge]
  $::XcedeCatalog_progressGauge SetValue 0
  $::XcedeCatalog_mainWindow SetStatusText "Parsing $xcedeFile"
  

  #--- get the XCEDE root
  set root [$parser GetRootElement]

  #--- get the directory of the normalized xcede file.
  set ::XcedeCatalog_Dir [file dirname [file normalize $xcedeFile]]
  puts "reading file $xcedeFile from $::XcedeCatalog_Dir"

  set ::XcedeCatalog(transformIDStack) ""
  set ::XcedeCatalog_HParent_ID ""
  set ::XcedeCatalog_ModelMrmlID ""
  set ::XcedeCatalogFSVolumeNodeIDs ""
  set ::XcedeCatalogFSModelNodeIDs ""
  set ::XcedeCatalog_MrmlID(anat2exf) ""
  set ::XcedeCatalog_NumberOfElements 0
  set ::XcedeCatalog_WhichElement 0
  set ::XcedeCatalog_AnnotationFiles ""

  unset -nocomplain ::XcedeCatalog_MrmlID(flipYTransform) 
  unset -nocomplain ::XcedeCatalog_MrmlID(rotX90Transform) 
  array unset ::XcedeCatalog_MrmlID ""

  #--- recursively import cataloged datasets 
  set ::XcedeCatalog(transformIDStack) ""
  set ::XcedeCatalog_HParent_ID ""
  
  XcedeCatalogImportGetNumberOfElements $root
  puts "File has $::XcedeCatalog_NumberOfElements elements"

  #--- recursively import cataloged datasets 
  set ::XcedeCatalog(transformIDStack) ""
  set ::XcedeCatalog_HParent_ID ""
  set root [$parser GetRootElement]
  XcedeCatalogImportGetElement $root

  #--- assume a that fips analysis data is included
  #--- and we need to register freesurfer datasets
  #  if { $::XcedeCatalog_MrmlID(anat2exf) != "" } {
  #      if { $::XcedeCatalogFSVolumeNodeIDs != "" || $::XcedeCatalogFSModelNodeIDs != "" } {
  #         XcedeCatalogApplyFIPStoSlicerCorrection
  #      }

  }
  $::XcedeCatalog_progressGauge SetValue 0
  
  #-- update main viewer and slice viewers.
  [$::slicer3::ApplicationGUI GetViewerWidget ] RequestRender
  [ [$::slicer3::ApplicationGUI GetMainSliceGUI0 ] GetSliceViewer ]  RequestRender
  [ [$::slicer3::ApplicationGUI GetMainSliceGUI1 ] GetSliceViewer ]  RequestRender
  [ [$::slicer3::ApplicationGUI GetMainSliceGUI2 ] GetSliceViewer ]  RequestRender
  $::slicer3::MRMLScene Modified
  
  #--- clean up.
  $parser Delete
  $::slicer3::MRMLScene SetErrorCode 0
  $::XcedeCatalog_mainWindow SetStatusText ""
}


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportGetNumberOfElements { element } {

  #--- save current parent locally
  set parent $::XcedeCatalog_HParent_ID

  #--- increment count
  incr ::XcedeCatalog_NumberOfElements

  #---TODO: probably don't need this...
  # leave a place holder in case we are a group (transform) node
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
  # leave a place holder in case we are a group (transform) node
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
    puts ""
    puts "Got element $elementType..."
    if { $elementType != "entry" && $elementType != "Entry" } {
        #--- only process catalog entry tags
        return 
    }
    
    #--- get attributes
    set nAtts [$element GetNumberOfAttributes]
    for {set i 0} {$i < $nAtts} {incr i} {
        set attName [$element GetAttributeName $i]
        set node($attName) [$element GetAttributeValue $i]
        puts "node($attName) = $node($attName)"
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
    set node($uriAttName) $::XcedeCatalog_Dir/$fname
    
    #--- make sure the uri exists
    set node($uriAttName) [ file normalize $node($uriAttName) ]
    if {![ file exists $node($uriAttName) ] } {
        puts "can't find file $node($uriAttName)."
        return
    }

    #--- make sure the uri is a file (and not a directory)
    if { ![file isfile $node($uriAttName) ] } {
        puts "$node($uriAttName) doesn't appear to be a file. Not trying to import."
        return
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
    puts "data format is $node($formatAttName)"
    set nodeType [ XcedeCatalogImportGetNodeType $node($formatAttName) ]
    if { $nodeType == "Unknown" } {
        puts "$node($formatAttName) is an unsupported format. Cannot import entry."
        return
    }
    #--- make sure the file is a supported format
    puts "data format is $node($formatAttName)"
    set fileformat [ XcedeCatalogImportFormatCheck $node($formatAttName) ]
    if { $fileformat == 0 } {
        puts "$node($formatAttName) is an unsupported format. Cannot import entry."
        return
    }
    
    #--- finally, create the node
    set handler XcedeCatalogImportEntry$nodeType
    
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
proc XcedeCatalogImportEntryVolume {node} {
  upvar $node n


    #--- ditch if there's no file in the uri
    if { ![info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryVolume: no uri specified for node $n(uri)"
        return
    }

    set centered 1
    set labelmap 0
    if { [info exists n(labelmap) ] } {
        set labelmap 1
    }
    
    set logic [$::slicer3::VolumesGUI GetLogic]
    if { $logic == "" } {
        puts "XcedeCatalogImportEntryVolume: Unable to access Volumes Logic. $n(uri) not imported."
        return
    }
    set volumeNode [$logic AddArchetypeVolume $n(uri) $centered $labelmap $n(name) ]
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
        #--- this is likely a label map volume
        set colorLogic [ $::slicer3::ColorGUI GetLogic ]
        if { $colorLogic != "" } {
            $volumeDisplayNode SetAndObserveColorNodeID [$colorLogic GetDefaultFreeSurferLabelMapColorNodeID ]
        } else {
            $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
        }
        $volumeDisplayNode SetAutoWindowLevel 0
        $volumeDisplayNode SetAutoThreshold 0
    } else {
        #--- assume this is a greyscale volume
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
        $volumeDisplayNode SetAutoWindowLevel 1
        $volumeDisplayNode SetAutoThreshold 1      
    }

    #set logic [$::slicer3::VolumesGUI GetLogic]
    $logic SetActiveVolumeNode $volumeNode

    #--- keep track of all volumes loaded.
    if { [ string first "FreeSurfer" $n(format) ] >= 0 } {
        lappend ::XcedeCatalogFSVolumeNodeIDs $volumeNodeID
    }
                              
    [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
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

    if { [info exists n(description) ] } {
        $mnode SetDescription $n(description)
    }

    #--- we assume catalogs will contain a single model
    #--- with which all overlays will be associated.
    #--- If more models are included in the catalog, then
    #--- the FIRST model loaded is the one scalars are
    #--- associated with.
    if { $::XcedeCatalog_ModelMrmlID == "" } {
        set ::XcedeCatalog_ModelMrmlID [ $mnode GetID ]
    } else {
        puts "Warning: Xcede catalogs for slicer should contain a single model to which scalar overlays will be associated. This xcede file contains multiple models: all scalar overlays will be associated with the first model."
    }

    #--- keep track of all freesurfer models loaded
    if { [ string first "FreeSurfer" $n(format) ] >= 0 } {
        lappend ::XcedeCatalogFSModelNodeIDs [$mnode GetID ]
    }
    puts "model MRMLID is $::XcedeCatalog_ModelMrmlID"
    
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
        puts "$line"
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
#------------------------------------------------------------------------------
proc XcedeCatalogImportEntryTransform {node} {
  upvar $node n

    #--- ditch if there's no file in the uri
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryTransform: no uri specified for node $n(uri). No transform imported."
        return
    }

    #--- add the node
    set tid [ XcedeCatalogImportCreateIdentityTransformNode $n(name) ]
    if { $tid == "" } {
        puts "XcedeCatalogImportEntryTransform: unable to add Transform Node. No transform imported."
        return
    }

    if { [info exists n(description) ] } {
        set tnode [$::slicer3::MRMLScene GetNodeByID $tid ]
        $tnode SetDescription $n(description)
    }
    
    #--- read the uri and translate matrix element values into place.
    XcedeCatalogImportSetMatrixFromURI $tid $n(uri)    

    #--- need this to associate transformable datasets to this xform
    puts "Setting ::XcedeCatalog_MrmlID($n(ID)) to be $tid."
    set ::XcedeCatalog_MrmlID($n(ID)) $tid
    puts "transform MRMLID is $::XcedeCatalog_MrmlID($n(ID))"

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
    if { ![info exists ::XcedeCatalog_ModelMrmlID ] } {
        puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri). No overlay imported."
        return
    }

    set mid $::XcedeCatalog_ModelMrmlID
    puts "Applying overlay to MRMLID=$mid"
    set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
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
    
    if {$format == "FreeSurfer:mgz-1" } {
        return 1
    } elseif {$format == "nifti:nii-1" } {
        return 1
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
        return 1
    } elseif { $format == "FreeSurfer:surface-1" } {
        return 1
    } elseif { $format == "FreeSurfer:overlay-1" } {
        return 1
    } elseif { $format == "FreeSurfer:matrix-1" } {
        return 1
    }  else {
        return 0
    }

}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc  XcedeCatalogApplyFIPStoSlicerCorrection { } {

    #--- find all the freesurfer nodes
    puts "Applying FIPS-To-Slicer Registration correction..."
    set vlen [ llength $::XcedeCatalogFSVolumeNodeIDs]
    set mlen [ llength $::XcedeCatalogFSModelNodeIDs]

    #--- add the flipY and rotateX matrices
    XcedeCatalogImportAddFIPS2SlicerHelperMatrices 

    #--- if helper matrices were added, nest the volume and model data
    #--- into the transform hierarchy.
    if { [ info exists ::XcedeCatalog_MrmlID(rotX90Transform) ] } {
        for { set i 0 } { $i < $vlen } { incr i } {
            set id [ lindex $::XcedeCatalogFSVolumeNodeIDs $i ]
            set node [$::slicer3::MRMLScene GetNodeByID $id ]
            $node SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(rotX90Transform)
            $node Modified
        }
        for { set i 0 } { $i < $mlen } { incr i } {
            set id [ lindex $::XcedeCatalogFSModelNodeIDs $i ]
            set node [$::slicer3::MRMLScene GetNodeByID $id ]
            $node SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(rotX90Transform)
            $node Modified
        }
    }
    if { [ info exists ::XcedeCatalog_MrmlID(flipYTransform) ] } {
        set rotnode [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(rotX90Transform) ]
        set flipnode [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(flipYTransform) ]
        $rotnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(flipYTransform)
        $rotnode Modified
        $flipnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(anat2exf)
        $flipnode Modified
    }
    puts "...done."
}

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportAddFIPS2SlicerHelperMatrices { } {


    set flipTid [ XcedeCatalogImportCreateIdentityTransformNode flipY ]
    set flipnode [ $::slicer3::MRMLScene GetNodeByID $flipTid ]
    set ::XcedeCatalog_MrmlID(flipYTransform) $flipTid
    set matrix [ $flipnode GetMatrixTransformToParent ]
    #--- matrix that flips in Y
    $matrix SetElement 0 0 1.0
    $matrix SetElement 0 1 0.0
    $matrix SetElement 0 2 0.0
    $matrix SetElement 0 3 0.0

    $matrix SetElement 1 0 0.0
    $matrix SetElement 1 1 -1.0
    $matrix SetElement 1 2 0.0
    $matrix SetElement 1 3 0.0

    $matrix SetElement 2 0 0.0
    $matrix SetElement 2 1 0.0
    $matrix SetElement 2 2 1.0
    $matrix SetElement 2 3 0.0

    $matrix SetElement 3 0 0.0
    $matrix SetElement 3 1 0.0
    $matrix SetElement 3 2 0.0
    $matrix SetElement 3 3 1.0

    set flip2Tid [ XcedeCatalogImportCreateIdentityTransformNode flipY2 ]
    set flipnode [ $::slicer3::MRMLScene GetNodeByID $flip2Tid ]
    set ::XcedeCatalog_MrmlID(flipY2Transform) $flip2Tid
    set matrix [ $flipnode GetMatrixTransformToParent ]
    #--- matrix that flips in Y
    $matrix SetElement 0 0 1.0
    $matrix SetElement 0 1 0.0
    $matrix SetElement 0 2 0.0
    $matrix SetElement 0 3 0.0

    $matrix SetElement 1 0 0.0
    $matrix SetElement 1 1 -1.0
    $matrix SetElement 1 2 0.0
    $matrix SetElement 1 3 0.0

    $matrix SetElement 2 0 0.0
    $matrix SetElement 2 1 0.0
    $matrix SetElement 2 2 1.0
    $matrix SetElement 2 3 0.0

    $matrix SetElement 3 0 0.0
    $matrix SetElement 3 1 0.0
    $matrix SetElement 3 2 0.0
    $matrix SetElement 3 3 1.0

    
    set rotX90Tid  [ XcedeCatalogImportCreateIdentityTransformNode rotX90 ]
    set rotnode [ $::slicer3::MRMLScene GetNodeByID $rotX90Tid ]
    set ::XcedeCatalog_MrmlID(rotX90Transform) $rotX90Tid
    set matrix [ $rotnode GetMatrixTransformToParent ]
    #--- matrix that rots around X by 90 degrees
    $matrix SetElement 0 0 1.0
    $matrix SetElement 0 1 0.0
    $matrix SetElement 0 2 0.0
    $matrix SetElement 0 3 0.0

    $matrix SetElement 1 0 0.0
    $matrix SetElement 1 1 0.0
    $matrix SetElement 1 2 1.0
    $matrix SetElement 1 3 0.0

    $matrix SetElement 2 0 0.0
    $matrix SetElement 2 1 -1.0
    $matrix SetElement 2 2 0.0
    $matrix SetElement 2 3 0.0

    $matrix SetElement 3 0 0.0
    $matrix SetElement 3 1 0.0
    $matrix SetElement 3 2 0.0
    $matrix SetElement 3 3 1.0

    set rotX902Tid  [ XcedeCatalogImportCreateIdentityTransformNode rotX902 ]
    set rotnode [ $::slicer3::MRMLScene GetNodeByID $rotX902Tid ]
    set ::XcedeCatalog_MrmlID(rotX902Transform) $rotX902Tid
    set matrix [ $rotnode GetMatrixTransformToParent ]
    #--- matrix that rots around X by 90 degrees
    $matrix SetElement 0 0 1.0
    $matrix SetElement 0 1 0.0
    $matrix SetElement 0 2 0.0
    $matrix SetElement 0 3 0.0

    $matrix SetElement 1 0 0.0
    $matrix SetElement 1 1 0.0
    $matrix SetElement 1 2 11.0
    $matrix SetElement 1 3 0.0

    $matrix SetElement 2 0 0.0
    $matrix SetElement 2 1 -1.0
    $matrix SetElement 2 2 0.0
    $matrix SetElement 2 3 0.0

    $matrix SetElement 3 0 0.0
    $matrix SetElement 3 1 0.0
    $matrix SetElement 3 2 0.0
    $matrix SetElement 3 3 1.0

}


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCataLogImportTest  { } {
    #no op.
}
