#------------------------------------------------------------------------------
# main entry point...
#------------------------------------------------------------------------------
proc XcatalogImport { xcatFile } {

    #--- Figure out what xcat format this file is, and
    #--- import it with either the XcedeCatalogImport
    #--- or the XnatXcatalogImport resources.

    set parser [ vtkXMLDataParser New ]
    $parser SetFileName $xcatFile
    set retval [ $parser Parse ]

    if { $retval == 0 } {
        $parser Delete
        return $retval
    } 

    set ::XcatalogImport_ParentID ""
    set ::Xcat_ElementCount 0
    set  root [ $parser GetRootElement ]
    XcatalogImportGetNumberOfElements $root

    if { $::Xcat_ElementCount > 0  } {
        set fileType [ XcatalogImportCheckRoot $root ]
    }

    $parser Delete
    if { $fileType == "XCEDE" } {
        puts "Importing XCEDE2.0 catalog"
        XcedeCatalogImport $xcatFile
    } elseif { $fileType == "XNAT" } {
        puts "Importing XNAT XCEDE catalog"
        XnatXcatImport $xcatFile
    } else {
        #--- pop up error message.
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText "XcatalogImporter: unrecognized XML format. No data loaded."
        $dialog Create
        $dialog Invoke
        $dialog Delete
    }
}

#------------------------------------------------------------------------------
#--- snuffle through the xml structure
#------------------------------------------------------------------------------
proc XcatalogImportGetNumberOfElements { element } {

 #--- save current parent locally
  set parent $::XcatalogImport_ParentID

  #--- increment count
  incr ::Xcat_ElementCount

  # process all the sub nodes
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    XcatalogImportGetNumberOfElements $nestElement
  }

  # restore parent locally
  set ::XcatalogImport_ParentID $parent
}


#------------------------------------------------------------------------------
#--- snuffle through the xml structure
#------------------------------------------------------------------------------
proc XcatalogImportCheckRoot { root } {

    set elementType  [$root GetName ]
    if { $elementType == "XCEDE" || $elementType == "Xcede" } {
        #--- this is an Xcede catalog
        return "XCEDE"
    } 

    #--- is it an XNAT Xcede catalog?
    set nAtts [ $root GetNumberOfAttributes ]
    for { set i 0 } {$i < $nAtts} {incr i } {
        set attName [ $root GetAttributeName $i ]
        if { $attName == "xmlns:xnat" } {
            set value($attName) [ $root GetAttributeValue $i]
            if { $value($attName) == "http://nrg.wustl.edu/xnat" } {
                return "XNAT"
            }
        }
    }
    #--- unknown
    return "UNKNOWN"

}
