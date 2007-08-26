
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
#  ImportXcede2Data c:/cygwin/home/wjp/data/fBIRN-AHM2007.xcede
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

  #--- get the XCEDE root
  set root [$parser GetRootElement]

  #--- get the directory of the normalized xcede file.
  set ::XcedeCatalog_Dir [file dirname [file normalize $xcedeFile]]
  puts "reading file $xcedeFile from $::XcedeCatalog_Dir"

  set ::XcedeCatalog(transformIDStack) ""
  set ::XcedeCatalog_HParent_ID ""

  array unset ::XcedeCatalog_MrmlID ""

  #--- recursively import things until the data
  #--- in the catalog is converted into a scene.
  XcedeCatalogGetElement $root

  #--- clean up.
  $parser Delete
  $::slicer3::MRMLScene SetErrorCode 0
}





#------------------------------------------------------------------------------
# recursive routine to import all elements and their
# nested parts
#------------------------------------------------------------------------------
proc XcedeCatalogGetElement {element} {
  # save current parent locally
  set parent $::XcedeCatalog_HParent_ID

  # import this element if it contains an entry
  XcedeCatalogImportAnEntry $element
  
  #---TODO: probably don't need this...
  # leave a place holder in case we are a group (transform) node
  lappend ::XcedeCatalog(transformIDStack) "NestingMarker"

  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XcedeCatalogGetElement $nestElement
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
# if the element should be converted to a node,
# parse the attributes of a node into a tcl array
# and then invoke the type-specific handler
#------------------------------------------------------------------------------
proc XcedeCatalogImportAnEntry {element} {

    #--- is this a catalog entry that contains a file or reference?
    set elementType [$element GetName]
    puts "..."
    puts "Got element $elementType..."
    if { $elementType != "entry" && $elementType != "Entry" } {
        puts "$elementType is not a catalog entry. Getting next element..."
        return 
    }

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
        puts "description for entry contains no format information. Not trying to import."
        return
    }
         
         
    #--- make the file is a supported format, and what kind of node is it?
    puts "data format is $node($formatAttName)"
    set nodeType [ XcedeCatalogImportFormatCheck $node($formatAttName) ]
    if { $nodeType == "Unknown" } {
        puts "$node($formatAttName) is an unsupported format. Not trying to import anything."
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

    #--- get just filename for naming the volume
    set splituri [ file split $n(uri) ]
    set i [ llength $splituri ]
    set fname [ lindex $splituri [ expr $i-1 ] ]

    set centered 1
    set labelmap 0
    if { [info exists n(labelmap) ] } {
        set labelmap 1
    }
    
    set logic [$::slicer3::VolumesGUI GetLogic]
    set volumeNode [$logic AddArchetypeVolume $n(uri) $centered $labelmap $fname ]
    set volumeNodeID [$volumeNode GetID]
    set volumeNode [$::slicer3::MRMLScene GetNodeByID $volumeNodeID]

    # use the current top of stack (might be "" if empty, but that's okay)
    set transformID [lindex $::XcedeCatalog(transformIDStack) end]
    $volumeNode SetAndObserveTransformNodeID $transformID

    set volumeDisplayNode [$volumeNode GetDisplayNode]
    $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
    set logic [$::slicer3::VolumesGUI GetLogic]
    $logic SetActiveVolumeNode $volumeNode

    [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
    [$::slicer3::VolumesGUI GetApplicationLogic] PropagateVolumeSelection
}

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportEntryModel {node} {
  upvar $node n

    #--- ditch if there's no file in the uri
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryModel: no uri specified for node $n(uri)"
        return
    }

    #--- get just filename for naming the model
    set splituri [ file split $n(uri) ]
    set i [ llength $splituri ]
    set fname [ lindex $splituri [ expr $i-1 ] ]

    set logic [$::slicer3::ModelsGUI GetLogic]
    set mnode [$logic AddModel $n(uri)]
    set dnode [$mnode GetDisplayNode]

    #--- need this to associate scalar overlays with the actual model node
    set ::XcedeCatalog_MrmlID($n(ID)) [$mnode GetID]
    puts "model MRMLID is $::XcedeCatalog_MrmlID($n(ID))"
}



#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc XcedeCatalogImportEntryOverlay {node} {
  upvar $node n

    #--- not really a node, per se...
    #--- ditch if there's no file in the uri
    if { ! [info exists n(uri) ] } {
        puts "XcedeCatalogImportEntryOverlay: no uri specified for node $n(name)"
        return
    }

    #--- get just filename for naming the model
    set splituri [ file split $n(uri) ]
    set i [ llength $splituri ]
    set fname [ lindex $splituri [ expr $i-1 ] ]

    #--- what model node should these scalars be applied to?
    if { ![info exists n(modelID) ] } {
        puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri)"
        return
    }

    #--- get Slicer's ID for this model, and then get model node
    set mid $::XcedeCatalog_MrmlID($n(modelID))
    puts "Applying overlay to MRMLID=$mid"
    set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
    set logic [$::slicer3::ModelsGUI GetLogic]

    #--- add the scalar to the node
    $logic AddScalar $n(uri) $mnode 
}



#------------------------------------------------------------------------------
# checking to see if Slicer can read this file format
# and returns the nodeType associated with that format
#
#------------------------------------------------------------------------------
proc XcedeCatalogImportFormatCheck { format } {

    #--- check format against known formats
    #--- TODO: Once these values are formally defined for
    #--- all freesurfer data, we will have to change.
    #--- matrix-1 and overlay-1 are made up!
    #--- TODO: Add more as we know what their
    #--- XCEDE definitions are (analyze, etc.)
    
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
