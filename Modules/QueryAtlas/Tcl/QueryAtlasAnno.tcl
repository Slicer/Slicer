
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

    #--- progress feedback
    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Culling any old model annotations..."
    $prog SetValue 0

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

        #--- COMPARE new list with last list.
        #--- progress feedback
        $prog SetValue 10
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
        
        #--- REMOVE references to deleted data

        set len [ llength $cullList ]
        if {$len == 0 } {
            #--- progress feedback
            $prog SetValue 100
            $win SetStatusText ""
            $prog SetValue 0
            return
        }
        set stprog 20.0
        for { set i 0 } { $i < $len } { incr i } {
            #--- progress feedback
            set progress [ expr $stprog + ( $i * ((100.0-$stprog)/$len)) ]
            $prog SetValue $progress

            set id [ lindex $cullList $i ]
            #--- Cull old pick model
            QueryAtlasCullOldPickModels $id
            
            set indx [ lsearch $::QA(annoModelNodeIDs) $id ]
            #--- hopefully these two lists have corresponding indices.
            set ::QA(annoModelNodeIDs) [ lreplace $::QA(annoModelNodeIDs) $indx $indx ]
            set ::QA(annoModelDisplayNodeIDs) [ lreplace $::QA(annoModelDisplayNodeIDs) $indx $indx ]
        }
        #--- progress feedback
        $win SetStatusText ""
        $prog SetValue 0
    }
}





#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCullOldLabelMapAnnotations { } {

    #--- progress feedback
    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Culling any old label map annotations..."
    $prog SetValue 0

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

        #--- progress feedback
        $prog SetValue 10
        
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
                lappend cullList $id
            }
        }

        
        #--- REMOVE references to deleted data

        set len [ llength $cullList ]
        if {$len == 0 } {
            #--- progress feedback
            $prog SetValue 100
            $win SetStatusText ""
            $prog SetValue 0
            return
        }
        set stprog 20.0
        for { set i 0 } { $i < $len } { incr i } {
            #--- progress feedback
            set progress [ expr $stprog + ( $i * ((100.0-$stprog)/$len)) ]
            $prog SetValue $progress
            
            set id [ lindex $cullList $i ]

            QueryAtlasCullOldPickModels $id
            
            set indx [ lsearch $::QA(annoLabelMapIDs) $id ]
            set ::QA(annoLabelMapIDs) [ lreplace $::QA(annoLabelMapIDs) $indx $indx ]
        }
        #--- progress feedback
        $win SetStatusText ""
        $prog SetValue 0
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewPickModels { mid } {

    #--- progress feedback
    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Creating new pick model..."
    $prog SetValue 0

    set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set renderer [$renderWidget GetRenderer]

    #--- add pick model and trappings for all new annotated models.
    #--- run the model's polydata through a triangle filter so picking will work right
    #--- update current scene model list
    set modelNode [$::slicer3::MRMLScene GetNodeByID $mid]

    set ::QA(polyData_$mid) [vtkPolyData New]
    $::QA(polyData_$mid) DeepCopy [$modelNode GetPolyData]
    set ::QA(actor_$mid) [vtkActor New]
    set ::QA(mapper_$mid) [vtkPolyDataMapper New]
    $::QA(mapper_$mid) SetInput $::QA(polyData_$mid)
    $::QA(actor_$mid) SetMapper $::QA(mapper_$mid)
    $::QA(polyData_$mid) Update


    #--- progress feedback
    $prog SetValue 10
    
    set cellData [$::QA(polyData_$mid) GetCellData]
    set cellNumberColors [$cellData GetArray "CellNumberColors"] 
    if { $cellNumberColors == "" } {
        set cellNumberColors [vtkUnsignedCharArray New]
        $cellNumberColors SetName "CellNumberColors"
        $cellData AddArray $cellNumberColors
    }

    #--- progress feedback
    $prog SetValue 40

    set cellNumberColors [$cellData GetArray "CellNumberColors"] 
    if { ![info exists ::QA(nextCellIndex)] } {
      set ::QA(nextCellIndex) 0
    }
    set numberOfCells [$::QA(polyData_$mid) GetNumberOfCells]
    $cellData SetScalars [ $::slicer3::QueryAtlasGUI AssignCellColorCode  $numberOfCells $::QA(nextCellIndex) $cellNumberColors]
    set ::QA(nextCellIndex) [expr $::QA(nextCellIndex) + $numberOfCells]
    set ::QA(numberOfCells,$mid) $numberOfCells

    $win SetStatusText "Adding new pick model..."
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

    #--- progress feedback
    $prog SetValue 80
    
    $::QA(mapper_$mid) SetScalarModeToUseCellData
    $::QA(mapper_$mid) SetScalarVisibility 1
    $::QA(mapper_$mid) SetScalarMaterialModeToAmbient
    $::QA(mapper_$mid) SetScalarRange 0 $::QA(numberOfCells_$mid)
    [$::QA(actor_$mid) GetProperty] SetAmbient 1.0
    [$::QA(actor_$mid) GetProperty] SetDiffuse 0.0

    #--- progress feedback
    $prog SetValue 100
    $renderer AddActor $::QA(actor_$mid)

    #--- set the actor's visibility to match its model
    set dnode [ $modelNode GetDisplayNode ]
    set ::QA(actor_$mid,visibility) [ $dnode GetVisibility ]
    $::QA(actor_$mid) SetVisibility  $::QA(actor_$mid,visibility)

    #--- progress feedback
    $prog SetValue 0
}







#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewModelAnnotations { modelAnnotationDir } {

    #--- progress feedback
    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Adding any new model annotations..."
    $prog SetValue 0


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

        #--- progress feedback
        $prog SetValue 10
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
        if {$len == 0 } {
            #--- progress feedback
            $prog SetValue 100
            $win SetStatusText ""
            $prog SetValue 0
            return
        }

        #--- progress feedback
        set stprog 20.0
        set pinc [ expr ( 100.0-$stprog)/$len  ]

        for { set i 0 } { $i < $len } { incr i } {
            
            set id [ lindex $addList $i ]
            set node [ $::slicer3::MRMLScene GetNodeByID $id ]
            set name [ $node GetName ]

            #--- progress feedback
            set progress [ expr $stprog + ($i * $pinc) ]
            $prog SetValue $progress
            
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

                #--- if that didn't work eit.0her, just look for the next new model.
                if { ! [ file exists $annoFileName ] } {
                    break
                }
                
                #--- if we're still here, then search for annotations on the
                #--- model and add them if they're not already there.
                $win SetStatusText "Adding annotation scalar for $name..."
            set mlogic [ $::slicer3::ModelsGUI GetLogic ]
                if { $mlogic != "" } {
                    #--- progress feedback
                    set p [expr $progress + ( $pinc/9.0 ) ]
                    $prog SetValue $p
                    
                    #--- check to see if scalar aparc.annot is there already.
                    #--- if not, add it
                    set dnodeID [ $node GetDisplayNodeID ]
                    set dnode [ $node GetDisplayNode ]
                    set snode [ $node GetStorageNode ]
                    # there are now more than one storage nodes on the model node, each overlay has it's own storage node. The annotations have been already added 
                    if {0} {
                    if { $snode != "" } {
                        set numOverlays [expr [ $node GetNumberOfStorageNodes ] - 1]
                        if { $numOverlays == 0 } {
                                #--- add the scalar onto the node
                                $mlogic AddScalar $annoFileName $node                                                        
                            } else {
                                for { set j 0 } { $j < $numOverlays } { incr j } {
                                    set oname   [ $snode GetOverlayFileName $j ]
                                    if { ( [ string first "lh.aparc.annot" $oname ] < 0 ) && ( [ string first "rh.aparc.annot" $oname] < 0 ) } {
                                        #--- add the scalar onto the node
                                        $mlogic AddScalar $annoFileName $node                            
                                    }
                                }
                            }
                    }
                    }
                    #--- progress feedback
                    set p [expr $progress + ( $pinc/9.0 ) ]
                    $prog SetValue $p
                    
                    lappend ::QA(annoModelNodeIDs) $id
                    lappend ::QA(annoModelDisplayNodeIDs) [ $node GetDisplayNodeID ]

                    #--- progress feedback
                    set p [expr $progress + ( $pinc/8.0 ) ]
                    $prog SetValue $p
                    
                    set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
                    unset -nocomplain  ::QA(labelMap_$id)

                    $viewer UpdateFromMRML
                    set actor [ $viewer GetActorByID $dnodeID ]
                    if { $actor == "" } {
                        puts "QueryAtlasAddNewModelAnnotations: can't find model as actor in scene..."
                        return
                    }
                    set mapper [$actor GetMapper]

                    #--- progress feedback
                    set p [expr $progress + ( $pinc/7.0 ) ]
                    $prog SetValue $p

                    #--- set the annotations to be the active scalars
                    set polydata [$node GetPolyData]
                    set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
                    [$node GetDisplayNode] SetActiveScalarName "labels"
                    [$node GetDisplayNode] SetScalarVisibility 1

                    if { $scalaridx == "-1" } {
                        set scalars [vtkIntArray New]
                        $scalars SetName "labels"
                        [$polydata GetPointData] AddArray $scalars
                        [$polydata GetPointData] SetActiveScalars "labels"
                        $scalars Delete
                    } 
                    #--- progress feedback
                    set p [expr $progress + ( $pinc/5.0 ) ]
                    $prog SetValue $p
                    
                    set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
                    set scalars [[$polydata GetPointData] GetArray $scalaridx]


                    set lutNode [vtkMRMLColorTableNode New]
                    $lutNode SetTypeToUser
                    $::slicer3::MRMLScene AddNode $lutNode
                    $lutNode SetName "QueryLUT_$id"
                    [$node GetDisplayNode] SetAndObserveColorNodeID [$lutNode GetID]

                    #--- progress feedback
                    set p [expr $progress + ( $pinc/3.0 ) ]
                    $prog SetValue $p

                    set fssar [vtkFSSurfaceAnnotationReader New]

                    $fssar SetFileName $annoFileName
                    $fssar SetOutput $scalars
                    $fssar SetColorTableOutput [$lutNode GetLookupTable]
                    # try reading an internal colour table first
                    $fssar UseExternalColorTableFileOff

                    #--- progress feedback
                    set p [expr $progress + ( $pinc/1.5 ) ]
                    $win SetStatusText "Reading colors and labels..."
                    $prog SetValue $p

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

                    #--- progress feedback
                    set p [expr $progress + ( $pinc/1.75 ) ]
                    $prog SetValue $p

                    # print them out
                    set ::QA(labelMap_$id) [array get _labels]

                    set entries [lsort -integer [array names _labels]]

                    # set the look up table
                    $mapper SetLookupTable [$lutNode GetLookupTable]

                    # make the scalars visible
                    $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
                    $mapper SetScalarVisibility 1

                    [$node GetDisplayNode] SetScalarRange [lindex $entries 0] [lindex $entries end]

                    $lutNode Delete
                    $fssar Delete

                    QueryAtlasAddNewPickModels $id

                    #--- progress feedback
                    $prog SetValue 0
                    $win SetStatusText ""
                }
            }
        }
    }
}






#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddNewLabelMapAnnotations { } {

    #--- progress feedback
    set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    set prog [ $win GetProgressGauge ]
    $win SetStatusText "Adding any new label map annotations..."
    $prog SetValue 0

    #--- WJP test: try just starting fresh each time...
    set ::QA(annoLabelMapIDs) ""
    #--- end WJP test.
    
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
        
        #--- progress feedback
        $prog SetValue 10

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
        if {$len == 0 } {
            #--- progress feedback
            $prog SetValue 100
            $win SetStatusText "Found nothing new to add."
            return
        }

        #--- progress feedback
        set stprog  20.0
        
        for { set i 0 } { $i < $len } { incr i } {

            set id [ lindex $addList $i ]
            set node [ $::slicer3::MRMLScene GetNodeByID $id ]
            set name [ $node GetName ]

            #--- progress feedback
            set progress [ expr $stprog + ( $i * ((100.0-$stprog)/$len)) ]
            $prog SetValue $progress

            if { $name != "" } {
                #--- is this a freesurfer LUT?
                if {  [ string first "aseg" $name ] >= 0 } {
                    set lutFile "$::Slicer3_HOME/share/FreeSurfer/FreeSurferColorLUT.txt"
                    if { [file exists $lutFile] } {
                        set fp [open $lutFile "r"]
                        while { ![eof $fp] } {
                            gets $fp line
                            if { [scan $line "%d %s %d %d %d" label name r g b] == 5 } {
                                set ::QAFS($label,name) $name
                                set ::QAFS($label,rgb) "$r $g $b"
                            }
                        }
                        close $fp
                    }
                    lappend ::QA(annoLabelMapIDs) $id
                } else {
                    #--- this uses some non-freesurfer lut
                    #--- TODO: get its LUT file, read it, and go....
                    set dnode [ $node GetDisplayNode ]
                    if { $dnode != "" } {
                        set cnode [ $dnode GetColorNode ]
                        if { $cnode != "" } {
                            set lut [ $cnode GetLookupTable ]
                            if { $lut != "" } {
                                set numColors [ $lut GetNumberOfColors ]
                                for {set zz 0 } {$zz<$numColors } {incr zz} {
                                    set ::QAFS($zz,name) [ $cnode GetColorName $zz ]
                                    set colorList [ $lut GetTableValue $zz ]
                                    set r [ lindex $colorList 0 ]
                                    set g [ lindex $colorList 1 ]                                    
                                    set b [ lindex $colorList 2 ]
                                    set ::QAFS($zz,rgb) "$r $g $b"
                                }
                            }
                        }
                    }
                    lappend ::QA(annoLabelMapIDs) $id
                }
            }
        }
    }
    #--- progress feedback
    $win SetStatusText ""
    $prog SetValue 0
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasUpdateAnnotations { modelAnnotationDir } {

    #--- take stock of what data we have loaded already
    #--- models:
    if { ![ info exists ::QA(annoModelNodeIDs) ] } {
        set ::QA(annoModelNodeIDs) ""
    } 
    if { ![ info exists ::QA(annoModelDisplayNodeIDs) ] } {
        set ::QA(annoModelDisplayNodeIDs) ""
    } 
    QueryAtlasCullOldModelAnnotations
    QueryAtlasAddNewModelAnnotations $modelAnnotationDir 

    #--- labelmaps:
    if { ![ info exists ::QA(annoLabelMapIDs) ] } {
        set ::QA(annoLabelMapIDs) ""
    }
    QueryAtlasCullOldLabelMapAnnotations
    QueryAtlasAddNewLabelMapAnnotations
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasInitialize { dataset annoPath } {

    set annotest 0
    # handle known datasets
    switch  $dataset {
        "Qdec" {
            QueryAtlasLoadQdecAnnotations $annoPath
            set annotest [ QueryAtlasConfirmFreeSurferAnnotations ]            
            if { $annotest } {
                QueryAtlasInitializeGlobals
                QueryAtlasUpdateAnnotations ""
                QueryAtlasCreatePicker
                QueryAtlasParseOntologyResources
                $::slicer3::QueryAtlasGUI UpdateScalarOverlayMenu
                QueryAtlasRenderView
            } else {
                QueryAtlasDialog "No annotations were found. Querying not enabled."
            }
        }
        "FIPSFreeSurfer" {
            set annotest [ QueryAtlasConfirmFreeSurferAnnotations ]
            if { $annotest } {
                QueryAtlasInitializeGlobals
                QueryAtlasUpdateAnnotations ""
                QueryAtlasCreatePicker
                QueryAtlasParseOntologyResources
                QueryAtlasRenderView
            } else {
                QueryAtlasDialog "No annotations were found. Querying not enabled."
            }
        }
        "NULL" {
            QueryAtlasInitializeGlobals
            QueryAtlasUpdateAnnotations ""
            QueryAtlasCreatePicker
            QueryAtlasParseOntologyResources
            QueryAtlasRenderView
        }
    }
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasLoadQdecAnnotations { annoPath } {

    #--- check scene for annotations; if none exist, load them.
    if { $annoPath == "" } {
        return 0
    }

    set qLogic [ $::slicer3::QdecModuleGUI GetLogic ]
    set mLogic [ $::slicer3::ModelsGUI GetLogic ] 
    set annoFile $annoPath
    if { $qLogic != "" && $mLogic != "" } {
        set mnode [ $qLogic GetModelNode ]
        if { $mnode != "" } {
            set mname [ $mnode GetName ]
            if { $mname == "lh.inflated" } {
                append annoFile "/lh.aparc.annot"
            } elseif { $mname == "rh.inflated" } {
                append annoFile "/rh.aparc.annot"
            }
            if { [ file exists $annoFile ] } {
                $mLogic AddScalar $annoFile $mnode
            }
        } 
    } 
}





#----------------------------------------------------------------------------------------------------
#--- So far, looks for freesurfer annotation files on models only
#----------------------------------------------------------------------------------------------------
proc QueryAtlasConfirmFreeSurferAnnotations { } {

    set retval 0
    #--- are the annotations already part of some model in the scene?
    #--- get all models in the scene.
    set n [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode" ]
    for { set i 0 } { $i < $n } { incr i } {
        #--- for each model, check its overlays for an annotation overlay
        set node [ $::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLModelNode" ]  
        # node changed, it now has GetNumberOfStorageNodes on it, the first one will contain the polydata, search the others (or all) for the GetFileName call that returns a string matching the annots.          
        set numOverlays [ $node GetNumberOfStorageNodes ]
        for { set j 0 } { $j < $numOverlays } { incr j } {
            set oname   [ [ $node GetNthStorageNode $j ] GetFileName]
            if { ( [ string first "lh.aparc.annot" $oname ] >= 0 ) || ( [ string first "rh.aparc.annot" $oname] >= 0 ) } {
              set retval 1
            }
        }
    }
    return $retval
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCreatePicker { } {
    
    #--- set up progress guage    
    set len1 0
    set len2 0
    if { [ info exists ::QA(annoModelNodeIDs) ] } {
        set len1 [ llength $::QA(annoModelNodeIDs) ]
    }
    if { [ info exists ::QA(annoLabelMapIDs) ] } {
        set len2 [ llength $::QA(annoLabelMapIDs) ]
    }

    if { $len1 > 0 || $len2 > 0 } {

        set win [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        set prog [ $win GetProgressGauge ]
        $win SetStatusText "Initializing picker..."
        $prog SetValue 0

        if { ! [info exists ::QA(propPicker) ] } {
            set ::QA(propPicker) [vtkPropPicker New]
        }
        if { ! [info exists ::QA(propPickerCollection) ] } {
            set ::QA(propPickerCollection) [vtkPropCollection New]
        }
        if { ![ info exists ::QA(cellPicker) ] } { 
            set ::QA(cellPicker) [vtkCellPicker New]
        }
        if { ![ info exists ::QA(cellPickerSliceActor) ] } {
            set ::QA(cellPickerSliceActor) [vtkActor New]
        }
        if { ![ info exists ::QA(cellPickerSliceMapper) ] } {
            set ::QA(cellPickerSliceMapper) [vtkPolyDataMapper New]
        }
        if { ![ info exists ::QA(cellPickerUserMatrix) ] } {
            set ::QA(cellPickerUserMatrix) [vtkMatrix4x4 New]
        }
        $::QA(cellPickerSliceActor) SetMapper $::QA(cellPickerSliceMapper)

        if { ![info exists ::QA(windowToImage)] } {
            set ::QA(windowToImage) [vtkWindowToImageFilter New]
        }

        #--- clean up 
        QueryAtlasRemoveInteractorObservers
        #--- and start fresh.
        QueryAtlasAddInteractorObservers

        #--- progress feedback
        $win SetStatusText "Rendering view..."
        $prog SetValue 60
        
        QueryAtlasRenderView

        #--- progress feedback
        $win SetStatusText "Adding query cursor..."
        $prog SetValue 80
        QueryAtlasUpdateCursor

        set ::QA(CurrentRASPoint) "0 0 0"
        
        #--- clear progress
        $win SetStatusText ""
        $prog SetValue 0
    }
}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseOntologyResources { } {

    #--- progress feedback
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
