
#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldPickModels { id } {

    #--- for each model id to cull, delete it's pick model trappings
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

    if { [ info exists ::QA(actor_$id,visibility)  ] } {
        unset -nocomplain ::QA(actor_$id,visibility)
    }
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldModelAnnotations { } {

    puts "inside cull"
    if {[info exists ::QA(annoModelNodeIDs) ] } {
        #---
        #--- MODELS
        #---
        #--- get new list of current scene models
        set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode" ]
        for { set i 0 } { $i < $n } { incr i } {
            set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLModelNode" ]            
            lappend tmpNodeList [$node GetID ]
        }
        puts "all: $tmpNodeList"

        #--- COMPARE new list with last list.

        set cullList ""
        set len [ llength $::QA(annoModelNodeIDs) ]
        for { set i 0 } { $i < $len } { incr i } {
            set id [ lindex $::QA(annoModelNodeIDs) $i ]
            #--- if a node is no longer in the current scene
            #--- it must have been deleted;
            #--- mark it  for removal
            if { [llength $tmpNodeList] == 0 } {
                lappend cullList $id
            } elseif { [ lsearch $tmpNodeList $id ] < 0 } {
                lappend cullList $id
            }
        }
        puts "cull: $cullList"
        
        #--- REMOVE references to deleted data
        set len [ llength $cullList ]
        for { set i 0 } { $i < $len } { incr i } {
            set id [ lindex $cullList $i ]

            #--- Cull old pick model
            QueryAtlasCullOldPickModels $id
            
            set indx [ lsearch $::QA(annoModelNodeIDs) $id ]
            #--- hopefully these two lists have corresponding indices.
            set ::QA(annoModelNodeIDs) [ lreplace $::QA(annoModelNodeIDs) $indx $indx ]
            set ::QA(annoModelDisplayNodeIDs) [ lreplace $::QA(annoModelDisplayNodeIDs) $indx $indx ]
        }

    }
}





#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldLabelMapAnnotations { } {

    if { [info exists ::QA(annoLabelMapIDs) ] } {
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
        set len [ llength $::QA(annoLabelMapIDs) ]
        for { set i 0 } { $i < $len } { incr i } {
            set id [ lindex $::QA(annoLabelMapIDs) $i ]
            #--- if a node is no longer in the current scene
            #--- it must have been deleted;
            #--- mark it for removal
            if { [llength $tmpNodeList] == 0 } {
                lappend cullList $id
            } elseif { [ lsearch $tmpNodeList $id ] < 0 } {
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

            if { [ info exists ::QA(actor_$id,visibility)  ] } {
                unset -nocomplain ::QA(actor_$id,visibility)
            }
            
            set indx [ lsearch $::QA(annoLabelMapIDs) $id ]
            set ::QA(annoLabelMapIDs) [ lreplace $::QA(annoLabelMapIDs) $indx $indx ]
        }
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewPickModels { mid } {

    set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set renderer [$renderWidget GetRenderer]

    #--- add pick model and trappings for all new annotated models.
    #--- update current scene model list
    set modelNode [$::slicer3::MRMLScene GetNodeByID $mid]
    set ::QA(polyData_$mid) [vtkPolyData New]
    $::QA(polyData_$mid) DeepCopy [$modelNode GetPolyData]
    set ::QA(actor_$mid) [vtkActor New]
    set ::QA(mapper_$mid) [vtkPolyDataMapper New]
    $::QA(mapper_$mid) SetInput $::QA(polyData_$mid)
    $::QA(actor_$mid) SetMapper $::QA(mapper_$mid)
    $::QA(polyData_$mid) Update
    
    set cellData [$::QA(polyData_$mid) GetCellData]
    set cellNumberColors [$cellData GetArray "CellNumberColors"] 
    if { $cellNumberColors == "" } {
        set cellNumberColors [vtkUnsignedCharArray New]
        $cellNumberColors SetName "CellNumberColors"
        $cellData AddArray $cellNumberColors
        $cellData SetScalars $cellNumberColors
    }

    $cellData SetScalars $cellNumberColors
    set cellNumberColors [$cellData GetArray "CellNumberColors"] 
    $cellNumberColors Initialize
    $cellNumberColors SetNumberOfComponents 4

    set numberOfCells [$::QA(polyData_$mid) GetNumberOfCells]
    for {set i 0} {$i < $numberOfCells} {incr i} {
        eval $cellNumberColors InsertNextTuple4 [QueryAtlasNumberToRGBA $i]
    }

    set ::QA(cellData_$mid) $cellData
    set ::QA(numberOfCells_$mid) $numberOfCells
    $cellNumberColors Delete

    set scalarNames {"CellNumberColors" "Opaque Colors"}
    foreach scalarName $scalarNames {
        if { [$::QA(cellData_$mid) GetScalars $scalarName] != "" } {
            $::QA(cellData_$mid) SetActiveScalars $scalarName
            break
        }
    }
    
    $::QA(mapper_$mid) SetScalarModeToUseCellData
    $::QA(mapper_$mid) SetScalarVisibility 1
    $::QA(mapper_$mid) SetScalarMaterialModeToAmbient
    $::QA(mapper_$mid) SetScalarRange 0 $::QA(numberOfCells_$mid)
    [$::QA(actor_$mid) GetProperty] SetAmbient 1.0
    [$::QA(actor_$mid) GetProperty] SetDiffuse 0.0

    $renderer AddActor $::QA(actor_$mid)

    #--- set the actor's visibility to match its model
    set dnode [ $modelNode GetDisplayNode ]
    set ::QA(actor_$mid,visibility) [ $dnode GetVisibility ]
    $::QA(actor_$mid) SetVisibility  $::QA(actor_$mid,visibility)
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewModelAnnotations { modelAnnotationDir } {

    if { [info exists ::QA(annoModelNodeIDs) ] } {
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
        set len [ llength $tmpNodeList ]
        if { $len > 0 } {
            if { [ info exists ::QA(annoModelNodeIDs) ] } {
                for { set i 0 } { $i < $len } { incr i } {
                    set id [ lindex $tmpNodeList $i ]
                    #--- if a node is not in our last list
                    #--- it must have been added;
                    #--- mark it for adding.
                    if { [ lsearch $::QA(annoModelNodeIDs) $id ] < 0 } {
                        lappend addList $id
                    }
                }
            }
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

                    #WJP TEST -- add a new pick model
                    QueryAtlasAddNewPickModels $id
                    
                    [$viewer GetMainViewer] Reset
                }
            }
        }
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewLabelMapAnnotations { } {

    if {[ info exists ::QA(annoLabelMapIDs) ] } { 
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
        if { $len > 0 } {
            if { [ info exists ::QA(annoLabelMapIDs) ] } {
                for { set i 0 } { $i < $len } { incr i } {
                    set id [ lindex $tmpNodeList $i ]
                    #--- if a node is not in our last list
                    #--- it must have been added;
                    #--- mark it for adding.
                    if { [ lsearch $::QA(annoLabelMapIDs) $id ] < 0 } {
                        lappend addList $id
                    }
                }
            }
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
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasUpdateAnnotations { modelAnnotationDir } {

    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Updating annotations..."
    $prog SetValue 0

    #--- take stock of what data we have loaded already
    #--- models:
    if { ![ info exists ::QA(annoModelNodeIDs) ] } {
        set ::QA(annoModelNodeIDs) ""
    } 
    $win SetStatusText "Culling old model annotations..."
    $prog SetValue 20
    QueryAtlasCullOldModelAnnotations

    $win SetStatusText "Adding new model annotations and pick models..."
    $prog SetValue 40
    QueryAtlasAddNewModelAnnotations $modelAnnotationDir 

    #--- labelmaps:
    if { ![ info exists ::QA(annoLabelMapIDs) ] } {
        set ::QA(annoLabelMapIDs) ""
    }


    $win SetStatusText "Culling old label map annotations..."
    $prog SetValue 70
    QueryAtlasCullOldLabelMapAnnotations

    $win SetStatusText "Adding new label map annotations..."
    $prog SetValue 90
    QueryAtlasAddNewLabelMapAnnotations

    $prog SetValue 0
    $win SetStatusText ""

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

    $win SetStatusText "Initializing picker..."
    $prog SetValue 0

    #--- WJP TRY this piece of InitializePicker...
    set ::QA(propPicker) [vtkPropPicker New]
    set ::QA(cellPicker) [vtkCellPicker New]
    set ::QA(cellPickerSliceActor) [vtkActor New]
    set ::QA(cellPickerSliceMapper) [vtkPolyDataMapper New]
    set ::QA(cellPickerUserMatrix) [vtkMatrix4x4 New]
    $::QA(cellPickerSliceActor) SetMapper $::QA(cellPickerSliceMapper)
    if { ![info exists ::QA(windowToImage)] } {
        set ::QA(windowToImage) [vtkWindowToImageFilter New]
    }
    QueryAtlasAddInteractorObservers
#        QueryAtlasInitializePicker 


    $win SetStatusText "Rendering view..."
    $prog SetValue 60
    QueryAtlasRenderView

    $win SetStatusText "Adding query cursor..."
    $prog SetValue 80
    QueryAtlasUpdateCursor

    set ::QA(CurrentRASPoint) "0 0 0"
        
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
    $prog SetValue 30
    QueryAtlasParseControlledVocabulary

    $win SetStatusText "Parsing NeuroNames Synonyms..."
    $prog SetValue 60
    QueryAtlasParseNeuroNamesSynonyms

    $win SetStatusText "Parsing precompiled URIs..."
    $prog SetValue 90
    QueryAtlasParseBrainInfoURIs

    $win SetStatusText ""
    $prog SetValue 0
    
}
