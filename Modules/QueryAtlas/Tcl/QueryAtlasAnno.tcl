
#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldModelAnnotations { } {

    #---
    #--- MODELS
    #---
    #--- get new list of current scene models
    set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode" ]
    for { set i 0 } { $i < $n } { incr i } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLModelNode" ]            
        lappend tmpNodeList [$node GetID ]
    }

    #--- COMPARE new list with last list.
    set cullList ""
    set len [ llength $::QA(sceneModelNodeIDs) ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $::QA(sceneModelNodeIDs) $i ]
        #--- if a node is no longer in the current scene
        #--- it must have been deleted;
        #--- mark it  for removal
        if { [ lsearch $tmpNodeList $id ] < 0 } {
            lappend cullList $id
        }
    }
    
    #--- REMOVE references to deleted data
    set len [ llength $cullList ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $cullList $i ]

        if { [ info exists ::QA(polyData_$id) ] } {
            $::QA(polyData_$id) Delete
            unset -nocomplain ::QA(polyData_$id)
        }
        if { [ info exists ::QA(mapper_$id) ]} {
            $::QA(mapper_$id) Delete
            unset -nocomplain ::QA(mapper_$id)
        }
        if { [ info exists ::QA(actor_$id) ] } {
            $::QA(actor_$id) Delete
            unset -nocomplain ::QA(actor_$id)
        }

        set indx [ lsearch $::QA(sceneModelNodeIDs) $id ]
        set ::QA(annoModelNodeIDs) [ lreplace ::QA(sceneModelNodeIDs) $indx $indx ]
        set indx [ lsearch $::QA(annoModelNodeIDs) $id ]
        set ::QA(annoModelNodeIDs) [ lreplace ::QA(annoModelNodeIDs) $indx $indx ]
        set indx [ lsearch $::QA(annoModelDisplayNodeIDs) $id ]
        set ::QA(annoModelDisplayNodeIDs) [ lreplace ::QA(annoModelDisplayNodeIDs) $indx $indx ]
    }
}





#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldLabelMapAnnotations { } {

    #---
    #--- LABEL MAPS
    #---
    #--- get new list of current scene label maps
    set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScalarVolumeNode" ]
    for { set i 0 } { $i < $n } { incr i } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScalarVolumeNode" ]            
        if { [$node GetLabelMap] == 1 } {
            lappend tmpNodeList [$node GetID ]
        }
    }

    #--- COMPARE new list with last list.
    set cullList ""
    set len [ llength $::QA(sceneLabelMapIDs) ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $::QA(sceneLabelMapIDs) $i ]
        #--- if a node is no longer in the current scene
        #--- it must have been deleted;
        #--- mark it for removal
        if { [ lsearch $tmpNodeList $id ] < 0 } {
            puts "culling $id"
            lappend cullList $id
        }
    }
    
    #--- REMOVE references to deleted data
    set len [ llength $cullList ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $cullList $i ]

        if { [ info exists ::QA(polyData_$id) ] } {
            $::QA(polyData_$id) Delete
            unset -nocomplain ::QA(polyData_$id)
        }
        if { [ info exists ::QA(mapper_$id) ]} {
            $::QA(mapper_$id) Delete
            unset -nocomplain ::QA(mapper_$id)
        }
        if { [ info exists ::QA(actor_$id) ] } {
            $::QA(actor_$id) Delete
            unset -nocomplain ::QA(actor_$id)
        }

        set indx [ lsearch $::QA(sceneLabelMapIDs) $id ]
        set ::QA(sceneLabelMapIDs) [ lreplace ::QA(sceneLabelMapIDs) $indx $indx ]
        set indx [ lsearch $::QA(annoLabelMapIDs) $id ]
        set ::QA(annoLabelMapIDs) [ lreplace ::QA(annoLabelMapIDs) $indx $indx ]
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewModelAnnotations { modelAnnotationDir } {

    #---
    #--- MODELS
    #---
    #--- get new list of current scene models
    set tmpNodeList ""
    set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode" ]
    for { set i 0 } { $i < $n } { incr i } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLModelNode" ]            
        lappend tmpNodeList [$node GetID ]
    }

    #--- COMPARE new list with last list.
    set addList ""
    if { [ info exists ::QA(sceneModelNodeIDs) ] } {
        set len [ llength $tmpNodeList ];
        for { set i 0 } { $i < $len } { incr i } {
            set id [ lindex $tmpNodeList $i ]
            #--- if a node is not in our last list
            #--- it must have been added;
            #--- mark it for adding.
            if { [ lsearch $::QA(sceneModelNodeIDs) $id ] < 0 } {
                lappend addList $id
            }
        }
    }
    #--- update current scene model list
    set ::QA(sceneModelNodeIDs) ""
    set len [ llength $tmpNodeList ]
    for { set i 0 } { $i < $len } { incr i } {
        lappend ::QA(sceneModelNodeIDs) [ lindex $tmpNodeList $i ]
    }

    
    #---
    #--- get annotation for all new models we found
    #--- that don't already have them.
    #--- for MODELS, check to see if an lh.aparc.annot
    #--- or an rh.aparc.annot is loaded first -- if not,
    #--- look in the modelAnnotation dir passed in.
    #---
    set len [ llength $addList ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $addList $i ]
        set node [ $::slicer3::MRMLScene GetNodeByID $id ]
        set name [ $node GetName ]
        
        set annoFileName ""
        #--- append all models with either .lh or .rh
        if { ( [ string first "lh." $name ] >= 0 ) || ( [ string first "rh." $name ] >= 0 ) } {

            #--- first try looking in the same directory as the model (xcede catalog format)
            set fname [[ $node GetStorageNode ] GetFileName ]
            if { [string first "lh." $name] >= 0 } {
                set annoFileName [ file dirname $fname ]
                append annoFileName "/lh.aparc.annot"
            } elseif { [string first "rh." $name] >= 0 } {
                set annoFileName [ file dirname $fname ]
                append annoFileName "/rh.aparc.annot"
            }

            #--- if that doesn't work, try looking in the directory passed in (qdec format)
            if { ! [ file exists $annoFileName ] } {
                if { $modelAnnotationDir != "" } {
                    set dirName [ file join $modelAnnotationDir "fsaverage" ]
                    if { [ file isdirectory $dirName ] } {
                        set dirName [ file join $dirName "label" ]
                        if { [ file isdirectory $dirName ] } {

                            #--- left or right?
                            if { [string first "lh." $name] >= 0 } {
                                set annoFileName [ file join $dirName "lh.aparc.annot" ]
                            } elseif { [string first "rh." $name] >= 0 } {
                                set annoFileName [ file join $dirName "rh.aparc.annot" ]
                            }            
                        }
                    }
                }
            }

            #--- if that didn't work either, just look for the next new model.
            if { ! [ file exists $annoFileName ] } {
                break
            }

            #--- if we're still here, then add annotations
            set mlogic [ $::slicer3::ModelsGUI GetLogic ]
            if { $mlogic != "" } {
                #--- add the scalar onto the node
                $mlogic AddScalar $annoFileName $node
                lappend ::QA(annoModelNodeIDs) $id
                lappend ::QA(annoModelDisplayNodeIDs) [ $node GetDisplayNodeID ]

                set dnodeID [ $node GetDisplayNodeID ]
                set dnode [ $node GetDisplayNode ]
                set snode [ $node GetStorageNode ]

                set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
                unset -nocomplain  ::QA(labelMap_$id)

                $viewer UpdateFromMRML
                set actor [ $viewer GetActorByID $dnodeID ]
                if { $actor == "" } {
                    puts "QueryAtlasAddNewModelAnnotations: can't find model as actor in scene..."
                    return
                }
                set mapper [$actor GetMapper]

                set polydata [$node GetPolyData]
                set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
                [$node GetDisplayNode] SetActiveScalarName "labels"
                [$node GetDisplayNode] SetScalarVisibility 1

                 if { $scalaridx == "-1" } {
                    puts "couldn't find scalars -- adding"
                    set scalars [vtkIntArray New]
                    $scalars SetName "labels"
                    [$polydata GetPointData] AddArray $scalars
                    [$polydata GetPointData] SetActiveScalars "labels"
                    $scalars Delete
                } 
                set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
                set scalars [[$polydata GetPointData] GetArray $scalaridx]

                set lutNode [vtkMRMLColorTableNode New]
                $lutNode SetTypeToUser
                $::slicer3::MRMLScene AddNode $lutNode
                $lutNode SetName "QueryLUT_$id"
                [$node GetDisplayNode] SetAndObserveColorNodeID [$lutNode GetID]

                set fssar [vtkFSSurfaceAnnotationReader New]

                $fssar SetFileName $annoFileName
                $fssar SetOutput $scalars
                $fssar SetColorTableOutput [$lutNode GetLookupTable]
                # try reading an internal colour table first
                $fssar UseExternalColorTableFileOff

                set retval [$fssar ReadFSAnnotation]

                array unset _labels

                if {$retval == 6} {
                    error "ERROR: no internal colour table, using default"
                    # use the default colour node
                    set colorLogic [$::slicer3::ColorGUI GetLogic]                
                    [$node GetDisplayNode] SetAndObserveColorNodeID [$colorLogic GetDefaultFreeSurferSurfaceLabelsColorNodeID]
                    set lutNode [[$node GetDisplayNode] GetColorNode]
                    # get the names 
                    for {set i 0} {$i < [$lutNode GetNumberOfColors]} {incr i} {
                        set _labels($i) [$lutNode GetColorName $i]
                    }
                } else {
                    # get the colour names from the reader       
                    array set _labels [$fssar GetColorTableNames]
                }
                array unset ::vtkFreeSurferReadersLabels_$id
                array set ::vtkFreeSurferReadersLabels_$id [array get _labels]

                # print them out
                set ::QA(labelMap_$id) [array get _labels]
                puts "$::QA(labelMap_$id)"

                set entries [lsort -integer [array names _labels]]

                # set the look up table
                $mapper SetLookupTable [$lutNode GetLookupTable]

                # make the scalars visible
                $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
                $mapper SetScalarVisibility 1

                [$node GetDisplayNode] SetScalarRange [lindex $entries 0] [lindex $entries end]

                $lutNode Delete
                $fssar Delete
                [$viewer GetMainViewer] Reset
            }
        }
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewLabelMapAnnotations { } {

    #---
    #--- LABEL MAPS
    #---
    #--- get new list of current scene label maps
    set tmpNodeList ""
    set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScalarVolumeNode" ]
    for { set i 0 } { $i < $n } { incr i } {
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScalarVolumeNode" ]            
        if { [ $node GetLabelMap ] == 1 } {
            lappend tmpNodeList [$node GetID ]
        }
    }
    
    #--- COMPARE new list with last list.
    set addList ""
    set len [ llength $tmpNodeList ]
    if { [ info exists ::QA(sceneLabelMapIDs) ] } {
        for { set i 0 } { $i < $len } { incr i } {
            set id [ lindex $tmpNodeList $i ]
            #--- if a node is not in our last list
            #--- it must have been added;
            #--- mark it for adding.
            if { [ lsearch $::QA(sceneLabelMapIDs) $id ] < 0 } {
                lappend addList $id
            }
        }
    }

    #--- update current scene label map list
    set ::QA(sceneLabelMapIDs) ""
    set len [ llength $tmpNodeList ]
    for { set i 0 } { $i < $len } { incr i } {
        lappend ::QA(sceneLabelMapIDs) [ lindex $tmpNodeList $i ]
    }

    #---
    #--- get annotation for new label maps we found
    #--- that don't already have them.
    #--- for LABELMAPS, look at the label values themselves.
    #---
    set len [ llength $addList ]
    for { set i 0 } { $i < $len } { incr i } {
        set id [ lindex $addList $i ]
        set node [ $::slicer3::MRMLScene GetNodeByID $id ]
        set name [ $node GetName ]

        if { $name != "" } {
            #--- is this a freesurfer LUT?
            if {  [ string first "aseg" $name ] >= 0 } {
                set lutFile "$::SLICER_BUILD/../Slicer3/Libs/FreeSurfer/FreeSurferColorLUT.txt"
                if { [file exists $lutFile] } {
                    set fp [open $lutFile "r"]
                    while { ![eof $fp] } {
                        gets $fp line
                        if { [scan $line "%d %s %d %d %d" label name r g b] == 5 } {
                            set ::QAFS($label,name) $name
                            set ::QAFS($label,rgb) "$r $g $b"
                            puts "$name -- $r $g $b"
                        }
                    }
                    close $fp
                }
                lappend ::QA(annoLabelMapIDs) $id
            } else {
                #--- this uses some non-freesurfer lut
                #--- TODO: get its LUT file, read it, and go....
                set dnode [ $node GetScalarVolumeDisplayNode ]
                if { $dnode != "" } {
                    set cid [ $dnode GetColorNodeID ]
                    if { $cid != "" } {
                        set cnode [ $::slicer3::MRMLScene GetNodeByID $cid ]
                        if { $cnode != "" } {
                            set lut [ $cnode GetLookupTable ]
                            #...
                        }
                    }
                }
            }
        }
    }

}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasUpdateAnnotations { modelAnnotationDir } {


    #--- take stock of what data we have loaded already
    #--- models:
    if { ![ info exists ::QA(sceneModelNodeIDs) ] } {
        #--- if no record of the scene exists yet, make one.
        unset -nocomplain ::QA(annoModelNodeIDs)
        set ::QA(sceneModelNodeIDs) ""
    } 
    puts "culling old model annotations"
    QueryAtlasCullOldModelAnnotations
    puts "adding new model annotations"
    QueryAtlasAddNewModelAnnotations $modelAnnotationDir 

    #--- labelmaps:
    if { ![ info exists ::QA(sceneLabelMapIDs) ] } {
        unset -nocomplain ::QA(annoLabelMapIDs)
        set ::QA(sceneLabelMapIDs) ""
    }

    puts "culling old label map annos"
    QueryAtlasCullOldLabelMapAnnotations
    puts "adding new label map annos"
    QueryAtlasAddNewLabelMapAnnotations
}


#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasInitialize { } {

    QueryAtlasInitializeGlobals
    puts "updating anotations"
    QueryAtlasUpdateAnnotations ""
    puts "creating picker and cursor"
    QueryAtlasCreatePicker
    puts "parsing ontology resources"
    QueryAtlasParseOntologyResources

}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCreatePicker { } {
    
    #--- set up progress guage
    
        set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        set prog [ $win GetProgressGauge ]
        $prog SetValue 0
        $win SetStatusText "Setting up query scene..."

        $win SetStatusText "Initializing picker..."
        $prog SetValue [ expr 100 * 1.0 / 5.0 ]
        QueryAtlasInitializePicker 

        $win SetStatusText "Rendering view..."
        $prog SetValue [ expr 100 * 2.0 / 5.0 ]
        QueryAtlasRenderView

        $win SetStatusText "Adding query cursor..."
        $prog SetValue [ expr 100 * 3.0 / 5.0 ]
        QueryAtlasUpdateCursor

        set ::QA(CurrentRASPoint) "0 0 0"
        $prog SetValue [ expr 100 * 4.0 / 5.0 ]
        
        $prog SetValue 100
        $win SetStatusText "Picker initialized."
        
        #--- clear progress
        $win SetStatusText ""
        $prog SetValue 0
}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseOntologyResources { } {

    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Parsing ontology resources..."
    $prog SetValue 0
    
    $win SetStatusText "Parsing controlled vocabulary..."
    $prog SetValue [ expr 100 * 1.0 / 4.0 ]
    QueryAtlasParseControlledVocabulary

    $win SetStatusText "Parsing NeuroNames Synonyms..."
    $prog SetValue [ expr 100 * 2.0 / 4.0 ]
    QueryAtlasParseNeuroNamesSynonyms

    $win SetStatusText "Parsing precompiled URIs..."
    $prog SetValue [ expr 100 * 3.0 / 4.0 ]
    QueryAtlasParseBrainInfoURIs

    $prog SetValue 100
    $win SetStatusText "Ontology resources loaded."

    $win SetStatusText ""
    $prog SetValue 0
    
}
