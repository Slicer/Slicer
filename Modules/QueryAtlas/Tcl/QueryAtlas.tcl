
#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTearDownPicker { } {

  set objects {
    propPicker cellPicker cellPickerSliceActor 
    cellPickerSliceMapper cellPickerUserMatrix
  }

  foreach o $objects {
    if { [ info exists ::QA($o)  ] } {
      $::QA($o) Delete
      unset -nocomplain ::QA($o)
    }
  }


    #--- clean out query models
    if { [ info exists ::QA(annoModelNodeIDs) ] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs) ]

        for { set m 0 } { $m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            if { [ info exists ::QA(polyData_$mid)  ] } {
                $::QA(polyData_$mid) Delete
                unset -nocomplain ::QA(polyData_$mid)
            }
            
            if { [ info exists ::QA(mapper_$mid)  ] } {
                $::QA(mapper_$mid) Delete
                unset -nocomplain ::QA(mapper_$mid)
            }
            
            if { [ info exists ::QA(actor_$mid)  ] } {
                $::QA(actor_$mid) Delete
                unset -nocomplain ::QA(actor_$mid)
            }

            if { [ info exists ::QA(actor_$mid,visibility)  ] } {
                unset -nocomplain ::QA(actor_$mid,visibility)
            }

        }
    }

    #--- clean out query label maps
    if { [ info exists ::QA(annoLabelMapIDs) ] } {
        set numMaps [ llength $::QA(annoLabelMapIDs) ]

        for { set m 0 } { $m < $numMaps } { incr m } {
            set mid [ lindex $::QA(annoLabelMapIDs) ]
            if { [ info exists ::QA(polyData_$mid)  ] } {
                $::QA(polyData_$mid) Delete
                unset -nocomplain ::QA(polyData_$mid)
            }
            
            if { [ info exists ::QA(mapper_$mid)  ] } {
                $::QA(mapper_$mid) Delete
                unset -nocomplain ::QA(mapper_$mid)
            }
            
            if { [ info exists ::QA(actor_$mid)  ] } {
                $::QA(actor_$mid) Delete
                unset -nocomplain ::QA(actor_$mid)
            }

            if { [ info exists ::QA(actor_$mid,visibility)  ] } {
                unset -nocomplain ::QA(actor_$mid,visibility)
            }
        }
    }
    
    if { [ info exists ::QA(windowToImage)  ] } {
        $::QA(windowToImage) Delete
        unset -nocomplain ::QA(windowToImage)
    }   
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasDialog { msg } {
    set dialog [ vtkKWMessageDialog New ]
    $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
    $dialog SetStyleToMessage
    $dialog SetText $msg
    $dialog Create
    $dialog Invoke
    $dialog Delete
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTearDownAnnoCursor { } {

    if { [info exists ::QA(cursor,mapper)] } {
        $::QA(cursor,mapper) Delete
        unset -nocomplain ::QA(cursor,mapper)
    }
    
    if { [info exists ::QA(cursor,actor)] } {
        $::QA(cursor,actor) Delete
        unset -nocomplain ::QA(cursor,actor)
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTearDown { } {

    #--- Delete things.
    QueryAtlasCloseOntologyBrowser
    QueryAtlasTearDownAnnoCursor
    QueryAtlasTearDownPicker

    if { [ info exists ::QA(globalsInitialized)  ] } {
        unset -nocomplain ::QA(globalsInitialized)
    }
    
    if { [ info exists ::QA(SceneSetUp) ] } {
        unset -nocomplain ::QA(SceneSetUp) 
    }
    
    #--- record of all annotated models and label maps in scene
    if {[info exists ::QA(annoModelNodeIDs) ] } {
        unset -nocomplain ::QA(annoModelNodeIDs)
    }
    if {[info exists ::QA(annoModelDisplayNodeIDs) ] } {
        unset -nocomplain ::QA(annoModelDisplayNodeIDs)
    }
    if {[info exists ::QA(annoLabelMapIDs) ] } {
        unset -nocomplain ::QA(annoLabelMapIDs)
    }

    set ::QA(brain,volumeNodeID) ""
    set ::QA(statvol,volumeNodeID) ""
    set ::QA(label,volumeNodeID) ""

    #--- set the model and label selectors to be NULL
    set as [$::slicer3::QueryAtlasGUI GetFSasegSelector]
    $as SetSelected ""
}




#----------------------------------------------------------------------------------------------------
# 
# 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasInitializeGlobals { } {


    if { ![info exists ::QA(CurrentRASPoint) ] } {
        set ::QA(CurrentRASPoint) ""
    }
    if { ![ info exists ::QA(ontologyHost) ] } {
        set ::QA(ontologyHost) "localhost"
    }
    if { ![ info exists ::QA(ontologyPort) ] } {
        set ::QA(ontologyPort) 3334
    }
    if { ![ info exists ::QA(ontologyViewerPID) ] }  {
        set ::QA(ontologyViewerPID) ""
    }
    if { ![ info exists ::QA(ontologBrowserRunning) ] } {
        set ::QA(ontologyBrowserRunning) 0
    }
    if { ![ info exists ::QA(annotationTermSet) ] } {
        set ::QA(annotationTermSet) "local"
    }
    if { ![ info exists ::QA(annotationVisibility) ] } {
        set ::QA(annotationVisibility) 1
    }
    if  { ![ info exists ::QA(localLabel) ] } {
        set ::QA(localLabel) ""
    }
    if { ![ info exists ::QA(lastLabels) ] } {
        set ::QA(lastLabels) ""
    }
    if { ![info exists ::QA(globalsInitialized) ] } {
        set ::QA(globalsInitialized) 1
    }

}





#----------------------------------------------------------------------------------------------------
# If a MRML node has been deleted,
# check to see if it's the model or label map.
# and do the right thing, whatever that turns
# out to be.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasNodeRemovedUpdate { } {
    
    #--- update overlay menu in case model with overlays
    #--- was deleted
    
    #--- cull deleted datasets
    puts "in remove node callback"
    QueryAtlasCullOldModelAnnotations
    QueryAtlasCullOldLabelMapAnnotations
}

#----------------------------------------------------------------------------------------------------
# If a MRML node has been added,
# check to see if it's a model or label map.
# we might need...
#----------------------------------------------------------------------------------------------------
proc QueryAtlasNodeAddedUpdate { } {

    #--- update overlay menu in case model
    #--- with overlays was deleted.
    
}




#----------------------------------------------------------------------------------------------------
# pops up a message when there's a problem
# str is a text string containing the message
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMessageDialog { str } {

    #--- convenience method for displaying popup message dialog.
        set dialog [ vtkKWMessageDialog New ]
        $dialog SetParent [ $::slicer3::ApplicationGUI GetMainSlicerWindow ]
        $dialog SetStyleToMessage
        $dialog SetText $str
        $dialog Create
        $dialog Invoke
        $dialog Delete
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetLabelLayer { } {

    set vs [$::slicer3::QueryAtlasGUI GetFSasegSelector]
    set ::QA(label,volumeNodeID) ""
    set node [ $vs GetSelected ]

    if { $node == "" } {
        return
    }

    set gotlabels 0
    set name ""
    set name [ $node GetName ]
    set t [ string first "aseg" $name ]
    if { $t >= 0 } {
        set gotlabels 1
        set ::QA(label,volumeNodeID) [ $node GetID ]
        #--- put in label layer
        set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]        
        for { set j 0 } { $j < $numCnodes } { incr j } {
            set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
            $cnode SetReferenceLabelVolumeID $::QA(label,volumeNodeID)
            $cnode SetLabelOpacity 0.4
        }
    }

    if { ! $gotlabels } {
        #QueryAtlasMessageDialog "Selected volume should be a FreeSurfer aparc+aseg file."
        set ::QA(label,volumeNodeID) ""
    }
}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetAnnotatedLabelMap { } {

    set vs [$::slicer3::QueryAtlasGUI GetFSasegSelector]
    set ::QA(label,volumeNodeID) ""
    set node [ $vs GetSelected ]

    if { $node == "" } {
        return
    }

    set gotlabels 0
    set name ""
    set name [ $node GetName ]
    set t [ string first "aseg" $name ]
    if { $t >= 0 } {
        set gotlabels 1
        set ::QA(label,volumeNodeID) [ $node GetID ]
        #--- put in label layer
        set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]        
        for { set j 0 } { $j < $numCnodes } { incr j } {
            set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
            $cnode SetReferenceLabelVolumeID $::QA(label,volumeNodeID)
            $cnode SetLabelOpacity 0.4
        }
    }

    if { ! $gotlabels } {
        #QueryAtlasMessageDialog "Selected volume should be a FreeSurfer aparc+aseg file."
        set ::QA(label,volumeNodeID) ""
    }
}

#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetAnatomical { } {

    set vs [$::slicer3::QueryAtlasGUI GetFSbrainSelector]
    set ::QA(brain,volumeNodeID) ""
    set node [ $vs GetSelected ]
    
    if { $node == "" } {
        return
    }

    set gotbrain 0
    set name ""
    set name [ $node GetName ]
    #--- now check on name
    set t [ string first "brain" $name ]
    if { $t >= 0 } {
        set gotbrain 1
        set ::QA(brain,volumeNodeID) [ $node GetID ]
        #--- put in background
        set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]        
        for { set j 0 } { $j < $numCnodes } { incr j } {
            set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
            $cnode SetReferenceBackgroundVolumeID $::QA(brain,volumeNodeID)
        }

    }
    
    if { ! $gotbrain } {
        #QueryAtlasMessageDialog "Selected volume should be a FreeSurfer brain.mgz file."
        set ::QA(brain,volumeNodeID) ""
    }
}


#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetStatistics { } {



    #--- Put the selected statistics in the foreground layer.
    set vs [$::slicer3::QueryAtlasGUI GetFSstatsSelector]
    set ::QA(statvol,volumeNodeID) ""
    set node [ $vs GetSelected ]

    if { $node == "" } {
        return
    }

    set gotstats 0
    set name ""
    set name [ $node GetName ]
    #--- now check on name
    set t [ string first "stat" $name ]
    if { $t >= 0 } {
        set gotstats 1
        set ::QA(statvol,volumeNodeID) [ $node GetID ]
        #--- put in foreground
        set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]        
        for { set j 0 } { $j < $numCnodes } { incr j } {
            set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
            $cnode SetReferenceForegroundVolumeID $::QA(statvol,volumeNodeID)
            $cnode SetForegroundOpacity 1
        }
    }
    
    if { ! $gotstats } {
        set ::QA(statvol,volumeNodeID) ""
    } 
}









#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddBIRNLogo {} {

  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set interactor [$renderWidget GetRenderWindowInteractor] 

  #
  # add the BIRN logo if possible (logo widget was added
  # after VTK 5, so not included in some builds)
  #
  if { [info command vtkLogoWidget] != "" } {

    # Logo
    set logoFile $::SLICER_BUILD/../Slicer3/Libs/FreeSurfer/Testing/birn-new-big.png
    set logoRep [vtkLogoRepresentation New]
    set reader [vtkPNGReader New]
    $reader SetFileName $logoFile
    $logoRep SetImage [$reader GetOutput]
    [$logoRep GetImageProperty] SetOpacity .75

    set logoWidget [vtkLogoWidget New]
    $logoWidget SetInteractor $interactor
    $logoWidget SetRepresentation $logoRep

    $logoWidget On
  }

  return

  #
  # generic logo setup
  # - BIRN specific config below
  #
  set reader [vtkPNGReader New]
  set texture [vtkTexture New]
  set polyData [vtkPolyData New]
  set points [vtkPoints New]
  set polys [vtkCellArray New]
  set tc [vtkFloatArray New]
  set textureMapper [vtkPolyDataMapper2D New]
  set textureActor [vtkActor2D New]
  set imageProperty [vtkProperty2D New]

  #$points SetNumberOfPoints 4
  set corners { {0.0 0.0 0.0} {1.0 0.0 0.0} {1.0 1.0 0.0} {0.0 1.0 0.0} }
  foreach corner $corners {
    eval $points InsertNextPoint $corner
  }

  $polyData SetPoints $points
  $polys InsertNextCell 4
  foreach p "0 1 2 3" {
    $polys InsertCellPoint 0
  }
  $polyData SetPolys $polys

  $tc SetNumberOfComponents 2
  $tc SetNumberOfTuples 4
  set tcs { {0.0 0.0} {1.0 0.0} {1.0 1.0} {0.0 1.0} }
  foreach tcoords $tcs point {0 1 2 3} {
    foreach i "0 1" tcoord $tcoords {
      $tc InsertComponent $point $i $tcoord
    }
  }
  [$polyData GetPointData] SetTCoords $tc

  $textureMapper SetInput $polyData
  $textureActor SetMapper $textureMapper

  $imageProperty SetOpacity 0.25


  #
  # birn logo specific
  #
  set logoFile $::SLICER_BUILD/../Slicer3/Libs/FreeSurfer/Testing/birn-new-big.png
  set ::QA(logo,actor) $textureActor

  set viewer [$::slicer3::ApplicationGUI GetViewerWidget]
  set renderWidget [$viewer GetMainViewer]
  set renderer [$renderWidget GetRenderer]
  $renderer AddActor2D $::QA(logo,actor)

  $reader Delete
  $texture Delete
  $polyData Delete
  $points Delete
  $polys Delete
  $tc Delete
  $textureMapper Delete
  $imageProperty Delete

  if { 0 } {
  #78 :  // Set up parameters from thw superclass
  #79 :double size[2];
  #80 :this->GetSize(size);
  #81 :this->Position2Coordinate->SetValue(0.04*size[0], 0.04*size[1]);
  #82 :this->ProportionalResize = 1;
  #83 :this->Moving = 1;
  #84 :this->ShowBorder = vtkBorderRepresentation::BORDER_ACTIVE;
  #85 :this->PositionCoordinate->SetValue(0.9, 0.025);
  #86 :this->Position2Coordinate->SetValue(0.075, 0.075); 
  }
}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasLoadQdecResults { qdecDir } {

    #--- check to see if the directory selected ends in qdec
    puts "got $qdecDir"
    #--- trim the trailing /
    set qdecDir [ string trimright $qdecDir "/" ]

    #--- test directory name: should be 'qdec'
    #--- get just the last directory name (sans path)
    set i [ string last "/" $qdecDir ]
    if { $i >= 0 } {
        set tststr [ string range $qdecDir [expr $i+1] end ]
        puts "is $tststr = qdec?"
        if { $tststr != "qdec" } {
            QueryAtlasDialog "QueryAtlasLoadQdecResults: please select a directory called 'qdec'. No results loaded."
        }
        return  0
    }

    #--- derive FreeSurfer subjects dir from qdecDir
    #--- and set it in the Qdec module logic.
    set FSdir "$qdecDir/../"
    puts "subejcts directory = $FSdir"

    set qlogic $::slicer3::QdecModuleLogic
    set mlogic $::slicer3::ModelsLogic
    set app $::slicer3::GetApplication
    
    $qlogic SetSubjectsDirectory $FSdir 
    set retval [ $qlogic LoadResults $mlogic $app ]

    if { $retval < 0 } {
        QueryAtlasDialog "QueryAtlasLoadQdecResults failed. No results loaded."
        return 0
    }
    return 1
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSelectQdecOverlay { } {
}




    
#----------------------------------------------------------------------------------------------------
#--- switches overlay to the query lut to pull out a label name
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSwitchToQueryLUT { } {

    if { $::QA(annoModelNodeIDs) != ""  } {
        set numModels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } { $m < $numModels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set modelNode [$::slicer3::MRMLScene GetNodeByID $mid ]
            set ::QA(saveOverlay) ""
            set ::QA(saveOverlay) [ $modelNode GetActiveScalarName ]
            set nodes [ $::slicer3::MRMLScene GetNodesByName "QueryLUT_$mid" ]
            set i [ $nodes GetNumberOfItems ]
            if { $i != 0 } {
                $nodes InitTraversal
                set lutNode [ $nodes GetNextItemAsObject ]
            }
            if { $lutNode != "" } {
                $modelNode SetActiveScalars "QueryLUT_$mid"
                [$modelNode GetDisplayNode] SetAndObserveColorNodeID [$lutNode GetID]
                [$modelNode GetDisplayNode ] SetActiveScalarName "QueryLUT_$mid" 
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#--- restores whatever scalar lut is displayed on the model after query label is
#--- extracted.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRestoreScalarLUT { } {
    if { $::QA(annoModelNodeIDs) != ""  } {
        set numModels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } { $m < $numModels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set modelNode [$::slicer3::MRMLScene GetNodeByID $mid ]
            #--- restore original LUT

            set nodes [ $::slicer3::MRMLScene GetNodesByName "$::QA(saveOverlay)" ]
            set i [ $nodes GetNumberOfItems ]
            if { $i != 0 } {
                $nodes InitTraversal
                set lutNode [ $nodes GetNextItemAsObject ]
            }
            if { $lutNode != "" } {
                $modelNode SetActiveScalars $::QA(saveOverlay)
                [$modelNode GetDisplayNode] SetAndObserveColorNodeID [$lutNode GetID]
                [$modelNode GetDisplayNode ] SetActiveScalarName $::QA(saveOverlay)
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#--- use the freesurfer annotation code to put 
#--- label scalars onto the model
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddAnnotations {LHAnnoFileName RHAnnoFileName } {
    
    if { $::QA(annoModelNodeIDs) != "" } {
        set numModels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } { $m < $numModels } { incr m } {

            #--- get each query model out of the scene
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set modelNode [$::slicer3::MRMLScene GetNodeByID $mid ]
            set displayNodeID [$modelNode GetDisplayNodeID]
            set displayNode [$::slicer3::MRMLScene GetNodeByID $displayNodeID]
            set storageNode [ $modelNode GetStorageNode ]
            set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
            unset -nocomplain  ::QA(labelMap_$mid)

            if { [ string first "lh." [$modelNode GetName]] >= 0 } {
                set fileName $LHAnnoFileName
            } elseif { [ string first "rh." [$modelNode GetName]] >= 0 } {
                set fileName $RHAnnoFileName
            }
            
            $viewer UpdateFromMRML
            #set actor [ $viewer GetActorByID [$modelNode GetID] ]
            set actor [ $viewer GetActorByID $displayNodeID ]
            if { $actor == "" } {
                puts "can't find model as actor"
                return
            }
            set mapper [$actor GetMapper]

            if { [file exists $fileName] } {
                set polydata [$modelNode GetPolyData]
                set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
                [$modelNode GetDisplayNode] SetActiveScalarName "labels"
                [$modelNode GetDisplayNode] SetScalarVisibility 1

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
                $lutNode SetName "QueryLUT_$mid"
                [$modelNode GetDisplayNode] SetAndObserveColorNodeID [$lutNode GetID]

                set fssar [vtkFSSurfaceAnnotationReader New]

                $fssar SetFileName $fileName
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
                    [$modelNode GetDisplayNode] SetAndObserveColorNodeID [$colorLogic GetDefaultFreeSurferSurfaceLabelsColorNodeID]
                    set lutNode [[$modelNode GetDisplayNode] GetColorNode]
                    # get the names 
                    for {set i 0} {$i < [$lutNode GetNumberOfColors]} {incr i} {
                        set _labels($i) [$lutNode GetColorName $i]
                    }
                } else {
                    # get the colour names from the reader       
                    array set _labels [$fssar GetColorTableNames]
                }
                array unset ::vtkFreeSurferReadersLabels_$mid
                array set ::vtkFreeSurferReadersLabels_$mid [array get _labels]

                # print them out
                set ::QA(labelMap_$mid) [array get _labels]
                puts "$::QA(labelMap_$mid)"

                set entries [lsort -integer [array names _labels]]

                # set the look up table
                $mapper SetLookupTable [$lutNode GetLookupTable]

                # make the scalars visible
                $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
                $mapper SetScalarVisibility 1

                [$modelNode GetDisplayNode] SetScalarRange [lindex $entries 0] [lindex $entries end]

                $lutNode Delete
                $fssar Delete
                [$viewer GetMainViewer] Reset
            }
        }

        #
        # read the freesurfer labels for the aseg+aparc
        #
        set lutFile $::SLICER_BUILD/../Slicer3/Libs/FreeSurfer/FreeSurferColorLUT.txt
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
        } else {
            QueryAtlasMessageDialog "Color lookup table $lutFile not found."
        }
    } 

}




#----------------------------------------------------------------------------------------------------
#--- convert a number to an RGBA 
#--- : A is always 255 (on transp)
#--- : number is incremented first so that 0 means background
#----------------------------------------------------------------------------------------------------
proc QueryAtlasNumberToRGBA {number} {
  set number [expr $number + 1]
  set r [expr $number / (256 * 256)]
  set number [expr $number % (256 * 256)]
  set g [expr $number / 256]
  set b [expr $number % 256]

  return "$r $g $b 255"
}

#----------------------------------------------------------------------------------------------------
#--- convert a RGBA to number
#--- : decrement by 1 to avoid ambiguity, since 0 is background
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRGBAToNumber {rgba} {
  foreach {r g b a} $rgba {}
  return [expr $r * (256*256) + $g * 256 + $b - 1] 
}




#----------------------------------------------------------------------------------------------------
#--- set up a picking version of the polyData that can be used
#--- to render to the back buffer
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCreatePickModels { } {

}


#----------------------------------------------------------------------------------------------------
#--- set up a picking version of the polyData that can be used
#--- to render to the back buffer
#----------------------------------------------------------------------------------------------------
proc QueryAtlasInitializePicker {} {

    #
    # add a prop picker to figure out if the mouse is actually over the model
    # and to identify the slice planes
    # and a pickRenderer and picker to find the actual world space pick point
    # under the mouse for a slice plane
    # - use a SliceMapper and SliceActor as surrogates for the actual 
    #   slice which has been changed to an ImageActor that does not work
    #   with the cell picker
    #

    QueryAtlasTearDownPicker
    set ::QA(propPicker) [vtkPropPicker New]
    set ::QA(cellPicker) [vtkCellPicker New]
    set ::QA(cellPickerSliceActor) [vtkActor New]
    set ::QA(cellPickerSliceMapper) [vtkPolyDataMapper New]
    set ::QA(cellPickerUserMatrix) [vtkMatrix4x4 New]
    $::QA(cellPickerSliceActor) SetMapper $::QA(cellPickerSliceMapper)

    #
    # get the polydata for all query models
    # - model node comes from the scene (retrieved by the ID)
    # - actor comes from the main Viewer
    # - mapper comes from the actor
    #

    set numQmodels [ llength $::QA(annoModelNodeIDs) ]
    for { set m 0 } { $m < $numQmodels } { incr m } {
        set mid [ lindex $::QA(annoModelNodeIDs) $m ]
        set modelNode [$::slicer3::MRMLScene GetNodeByID $mid]
        set ::QA(polyData_$mid) [vtkPolyData New]
        $::QA(polyData_$mid) DeepCopy [$modelNode GetPolyData]
        set ::QA(actor_$mid) [vtkActor New]
        set ::QA(mapper_$mid) [vtkPolyDataMapper New]
        $::QA(mapper_$mid) SetInput $::QA(polyData_$mid)
        $::QA(actor_$mid) SetMapper $::QA(mapper_$mid)
    }


    #
    # instrument the polydata with cell number colors
    # - note: even though the array is named CellNumberColors here,
    #   vtk will (sometimes?) rename it to "Opaque Colors" as part of the first 
    #   render pass
    #

    for { set m 0 } { $m < $numQmodels } { incr m } {
        set mid [ lindex $::QA(annoModelNodeIDs) $m ]
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

    }

    #
    # add the mouse move callback
    # - create the classes that will be used every render and in the callback
    # - add the callback with the current render info
    #
    if { ![info exists ::QA(windowToImage)] } {
        set ::QA(windowToImage) [vtkWindowToImageFilter New]
    }

    QueryAtlasAddInteractorObservers

    set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set renderer [$renderWidget GetRenderer]    
    for { set m 0 } { $m < $numQmodels } { incr m } {
        #--- add the query actor to the scene and
        #--- set its visibility to match that of its model node.
        set mid [ lindex $::QA(annoModelNodeIDs) $m ]
        set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
        set dnode [ $node GetDisplayNode ]
        $renderer AddActor $::QA(actor_$mid)
        set ::QA(actor_$mid,visibility) [ $dnode GetVisibility ]
        $::QA(actor_$mid) SetVisibility $::QA(actor_$mid,visibility)
    }

}


#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddInteractorObservers { } {


    set ::QA(enterEventTag) ""
    set ::QA(leaveEventTag) ""
    set ::QA(moveEventTag) ""
    set ::QA(rightClickEventTag) ""
    set ::QA(rightReleaseEventTag) ""
    set ::QA(startInteractionEventTag) ""
    set ::QA(endInteractionEventTag) ""
    set ::QA(endRenderEventTag) ""

    if {[info exists ::QA(propPicker) ] } {
        set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
        set renderer [$renderWidget GetRenderer]
        set interactor [$renderWidget GetRenderWindowInteractor] 
        set style [$interactor GetInteractorStyle] 
        
        set ::QA(enterEventTag) [ $interactor AddObserver EnterEvent "QueryAtlasCursorVisibility on" ]
        set ::QA(leaveEventTag) [ $interactor AddObserver LeaveEvent "QueryAtlasCursorVisibility off" ]
        set ::QA(moveEventTag) [ $interactor AddObserver MouseMoveEvent "QueryAtlasPickCallback" ]
        set ::QA(rightClickEventTag) [$interactor AddObserver RightButtonPressEvent "QueryAtlasMenuCreate start" ]
        set ::QA(rightReleaseEventTag) [ $interactor AddObserver RightButtonReleaseEvent "QueryAtlasMenuCreate end" ]
        set ::QA(startInteractionEventTag) [ $style AddObserver StartInteractionEvent "QueryAtlasCursorVisibility off" ]
        set ::QA(endInteractionEventTag) [ $style AddObserver EndInteractionEvent "QueryAtlasCursorVisibility on" ]
        set ::QA(endRenderEventTag) [ $style AddObserver EndInteractionEvent "QueryAtlasRenderView" ]
    }
}


#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRemoveInteractorObservers { } {


    set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set renderer [$renderWidget GetRenderer]
    set interactor [$renderWidget GetRenderWindowInteractor] 
    set style [$interactor GetInteractorStyle] 

    if { $::QA(enterEventTag) != "" } {
        $interactor RemoveObserver $::QA(enterEventTag)
    }
    if { $::QA(leaveEventTag) != "" } {
        $interactor RemoveObserver $::QA(leaveEventTag)
    }
    if { $::QA(moveEventTag) != "" } {
        $interactor RemoveObserver $::QA(moveEventTag)
    }
    if { $::QA(rightClickEventTag) != "" } {
        $interactor RemoveObserver $::QA(rightClickEventTag)
    }
    if { $::QA(rightReleaseEventTag) != "" } {
        $interactor RemoveObserver $::QA(rightReleaseEventTag)
    }
    if { $::QA(startInteractionEventTag) != "" } {
        $style RemoveObserver $::QA(startInteractionEventTag)
    }
    if { $::QA(endInteractionEventTag) != "" } {
        $style RemoveObserver $::QA(endInteractionEventTag)
    }
    if { $::QA(endRenderEventTag) != "" } {
        $style RemoveObserver $::QA(endRenderEventTag)
    }
    QueryAtlasCursorVisibility off
}


#----------------------------------------------------------------------------------------------------
#--- re-render the picking model from the current camera location
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRenderView {} {

  #
  # get the renderer related instances
  #
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set renderWindow [$renderWidget GetRenderWindow]
  set renderer [$renderWidget GetRenderer]

  #
  # draw the image and get the pixels
  # - set the render parameters to draw with the cell labels
  # - draw in the back buffer
  # - pull out the pixels
  # - restore the draw state and render
  #
  $renderWindow SetSwapBuffers 0
  set renderState [QueryAtlasOverrideRenderState $renderer]
  $renderWidget Render
  
  $::QA(windowToImage) SetInput [$renderWidget GetRenderWindow]

  set imageSize [lrange [[$::QA(windowToImage) GetOutput] GetDimensions] 0 1]
  if { [$renderWindow GetSize] != $imageSize } {
    $::QA(windowToImage) Delete
    set ::QA(windowToImage) [vtkWindowToImageFilter New]
    $::QA(windowToImage) SetInput [$renderWidget GetRenderWindow]
  }

  $::QA(windowToImage) SetInputBufferTypeToRGBA
  $::QA(windowToImage) ShouldRerenderOn
  $::QA(windowToImage) ReadFrontBufferOff
  $::QA(windowToImage) Modified
  [$::QA(windowToImage) GetOutput] Update


  if { 0 } {
    #
    # make a little preview window for debugging pleasure
    #
    if { [info command viewer] == "" } {
      vtkImageViewer viewer
      vtkImageData viewerImage
    }
    viewerImage DeepCopy [$::QA(windowToImage) GetOutput]
    viewer SetInput viewerImage
    viewer SetColorWindow 200
    viewer SetColorLevel 100
    viewer Render
  }


  $renderWindow SetSwapBuffers 1
  QueryAtlasRestoreRenderState $renderer $renderState
  $renderWidget Render

  #--- need to let go of the RenderWindow to avoid a crash on
  #--- ApplicationGUI->ViewerWidget->Delete()
  #--- Why it crashes instead of generating leaks I'm not sure.
  $::QA(windowToImage) SetInput ""

}

#####################################

#----------------------------------------------------------------------------------------------------
#--- Override/Restore render state 
#--- : set up for rendering the cell picker and then
#---   restore the state afterwards
#----------------------------------------------------------------------------------------------------
proc QueryAtlasOverrideRenderState {renderer} {

  #
  # save the render state before overriding it with the 
  # parameters needed for cell rendering
  # - is just background color and visibility state of all actors
  #

    #--- set all actors in scene invisible
    #--- and turn on only the query actors
    #--- for actors in the scene that the
    #--- user has set visible.

  set actors [$renderer GetActors]
  set numberOfItems [$actors GetNumberOfItems]
  for {set i 0} {$i < $numberOfItems} {incr i} {
      set actor [$actors GetItemAsObject $i]
      set state($i,visibility) [$actor GetVisibility]
      $actor SetVisibility 0
  }

  set state(background) [$renderer GetBackground]
  $renderer SetBackground 0 0 0
  
  set numQmodels [ llength $::QA(annoModelNodeIDs) ]
  for { set m 0 } { $m < $numQmodels } { incr m } {
      set mid [ lindex $::QA(annoModelNodeIDs) $m ]
      $::QA(actor_$mid) SetVisibility $::QA(actor_$mid,visibility)
  }

  return [array get state]
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRestoreRenderState {renderer renderState} {

  array set state $renderState

  eval $renderer SetBackground $state(background)

  set actors [$renderer GetActors]
  set numberOfItems [$actors GetNumberOfItems]
  for {set i 0} {$i < $numberOfItems} {incr i} {
    set actor [$actors GetItemAsObject $i]
    $actor SetVisibility $state($i,visibility)
  }

  set numQmodels [ llength $::QA(annoModelNodeIDs) ]
  for { set m 0 } { $m < $numQmodels } { incr m } {
      set mid [ lindex $::QA(annoModelNodeIDs) $m ]
      $::QA(actor_$mid) SetVisibility 0
  }

}

#----------------------------------------------------------------------------------------------------
#--- utility routine that should be provided by vtkCell
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPCoordsToWorld {cell pCoords} {
    
  if { [$cell GetClassName] != "vtkQuad" } {
    return "0 0 0"
  }

  foreach {r s t} $pCoords {}
  set rm [expr 1. - $r]
  set sm [expr 1. - $s]
  set sf0 [expr $rm * $sm]
  set sf1 [expr $r * $sm]
  set sf2 [expr $r * $s]
  set sf3 [expr $rm * $s]

  set points [$cell GetPoints]
  foreach {x0 x1 x2} "0 0 0" {}
  foreach p "0 1 2 3" {
    set point [$points GetPoint $p]
    foreach c "0 1 2" pp $point {
      set x$c [expr [set x$c] + $pp * [set sf$p]]
    }
  }
  return "$x0 $x1 $x2"
}

#----------------------------------------------------------------------------------------------------
#--- utility routine that should be provided by vtkKWRenderWidget
#----------------------------------------------------------------------------------------------------
proc QueryAtlasWorldToScreen { r a s } {

  set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
  set renderWidget [$viewer GetMainViewer]
  set camera [[$renderWidget GetRenderer] GetActiveCamera]
  set tkwindow [$renderWidget  GetWidgetName]
  set width [winfo width $tkwindow]
  set height [winfo height $tkwindow]

  set m [$camera GetCompositePerspectiveTransformMatrix [expr (1. * $width) / $height] 0 1]
  set vport [eval $m MultiplyPoint $r $a $s 1]
  set w [lindex $vport 3]
  set vx [expr [lindex $vport 0] / $w]
  set vy [expr [lindex $vport 1] / $w]


  set x [expr $width * (1. + $vx)/2.]
  set y [expr $height * (1. + $vy)/2.]

  return "$x $y"
}

proc QueryAtlasDistance { fromXY toXY } {
  set sum 0
  foreach f $fromXY t $toXY {
    set sum [expr $sum + ($f-$t)*($f-$t)]
  }
  return [expr sqrt($sum)]
}

#----------------------------------------------------------------------------------------------------
#--- query the cell number at the mouse location
#----------------------------------------------------------------------------------------------------

proc QueryAtlasPickProp {x y renderer viewer} {

    set ::QA(currentHit) ""
    set ::QA(currentHitProp) ""
    set ::QA(currentHitMRMLID) ""
    set _useMID ""

    if { [$::QA(propPicker) PickProp $x $y $renderer] } {
        set prop [$::QA(propPicker) GetViewProp]
        set ::QA(currentHitProp) $prop
        set ::QA(currentHitMRMLID) [$viewer GetIDByActor $prop]

        set numQmodels [ llength $::QA(annoModelDisplayNodeIDs) ]
        #--- hit query model display nodes?
        for { set m 0 } { $m < $numQmodels } { incr m } {
            if { $prop == $::QA(actor,$m) } {
                set ::QA(currentHit) "QueryModel"
                #--- choose the right label map
                set mid [ lindex $::QA(annoModelNodeIDs) $m ]
                set _useMID $mid
            }
        }

        if { $::QA(currentHit) == "" } {
            if { $::QA(currentHit) == "" } {
                set ::QA(currentHit) "QuerySlice"
            }
        }
    }

    return $_useMID
}

proc QueryAtlasPickOnQuerySlice {x y renderer} {

    set pointLabels ""

    if { $::QA(currentHit) == "QuerySlice" } {
        set node [$::slicer3::MRMLScene GetNodeByID $::QA(currentHitMRMLID)]
        #--- get the corresponding model display node
        set numMnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelNode"]
        for { set zz 0 } { $zz < $numMnodes } { incr zz } {
            set testnode [ $::slicer3::MRMLScene GetNthNodeByClass $zz "vtkMRMLModelNode" ]          
            set testdisplayID [ $testnode GetDisplayNodeID ]
            if { $testdisplayID == $::QA(currentHitMRMLID) } {
                set node $testnode
            }
        }

        if { $node != "" && [$node GetDescription] != "" } {
            array set nodes [$node GetDescription]
            set nodes(sliceNode) [$::slicer3::MRMLScene GetNodeByID $nodes(SliceID)]
            set nodes(compositeNode) [$::slicer3::MRMLScene GetNodeByID $nodes(CompositeID)]
            set propCollection [$::QA(cellPicker) GetPickList]
            $propCollection RemoveAllItems
            
            $::QA(cellPickerUserMatrix) Identity
            set tNode [$node GetParentTransformNode]
            if { $tNode != "" && [$tNode IsLinear] } {
              # TODO: this may be needed again depending on the final 
              # configuration of the vtkSlicerSliceLogic and how 
              # it handles slice model geometry
              #$tNode GetMatrixTransformToWorld $::QA(cellPickerUserMatrix)
            }
            $::QA(cellPickerSliceActor) SetUserMatrix $::QA(cellPickerUserMatrix)
            $::QA(cellPickerSliceMapper) SetInput [$node GetPolyData]
            $propCollection AddItem $::QA(cellPickerSliceActor)
            $::QA(cellPicker) PickFromListOn

            $::QA(cellPicker) Pick $x $y 0 $renderer

            set cellID [$::QA(cellPicker) GetCellId]
            set pCoords [$::QA(cellPicker) GetPCoords]
            if { $cellID != -1 } {
                set polyData [$::QA(cellPickerSliceMapper) GetInput]
                set cell [$polyData GetCell $cellID]
                
                #--- this gets the RAS point we're pointing to.
                set rasPoint [QueryAtlasPCoordsToWorld $cell $pCoords]
                set ::QA(CurrentRASPoint) $rasPoint

                set labelID [$nodes(compositeNode) GetLabelVolumeID]
                if { $labelID == "" } {
                  set pointLabels "No Label Layer"
                } else {
                  set nodes(labelNode) [$::slicer3::MRMLScene GetNodeByID $labelID]
                  set rasToIJK [vtkMatrix4x4 New]
                  $nodes(labelNode) GetRASToIJKMatrix $rasToIJK
                  set ijk [lrange [eval $rasToIJK MultiplyPoint $rasPoint 1] 0 2]
                  set imageData [$nodes(labelNode) GetImageData]
                  foreach var {i j k} val $ijk {
                      set $var [expr int(round($val))]
                  }
                  set labelValue [$imageData GetScalarComponentAsDouble $i $j $k 0]
                  if { [info exists ::QAFS($labelValue,name)] } {
                      if { $::QAFS($labelValue,name) == "Unknown" } {
                          #
                          # we found a part of the slice model that has no label - 
                          # - in this case, turn that slice invisible and return empty string
                          #   which will cause another iteration through the picker to find any
                          #   other models (or slices) visible behind this one.
                          set pointLabels ""
                          $::QA(currentHitProp) SetVisibility 0
                          lappend ::QA(tempInvisibleProps) $::QA(currentHitProp)
                      } else {
                          set pointLabels "$::QAFS($labelValue,name)"
                      }
                  } else {
                      set pointLabels "label: $labelValue (no name available), ijk $ijk"
                  }
                  $rasToIJK Delete
                }
            }
        }
    }

    return $pointLabels
}

proc QueryAtlasPickCallback {} {

    set _useMID ""

    if { ![info exists ::QA(windowToImage)] } {
        return
    }

    if { [ info exists ::QA(cursor,actor) ] } {
        if { [$::QA(cursor,actor) GetVisibility] == 0 } {
            # if the cursor isn't on, don't bother to calculate labels
            return
        }
    }

    #
    # get access to the standard view parts
    #
    set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
    set renderWidget [$viewer GetMainViewer]
    set renderWindow [$renderWidget GetRenderWindow]
    set interactor [$renderWidget GetRenderWindowInteractor] 
    set renderer [$renderWidget GetRenderer]

    set numQmodels [ llength $::QA(annoModelDisplayNodeIDs) ]
    for { set m 0 } { $m < $numQmodels } { incr m } {
        set _mid [ lindex $::QA(annoModelDisplayNodeIDs) $m ]
        set ::QA(actor,$m) [ $viewer GetActorByID $_mid ]
    }

    # if the window size has changed, re-render
    set imageSize [lrange [[$::QA(windowToImage) GetOutput] GetDimensions] 0 1]
    if { [$renderWindow GetSize] != $imageSize } {
        QueryAtlasRenderView
    }

    # 
    # get the event location
    #
    eval $interactor UpdateSize [$renderer GetSize]
    set ::QA(lastWindowXY) [$interactor GetEventPosition]
    foreach {x y} $::QA(lastWindowXY) {}
    set ::QA(lastRootXY) [winfo pointerxy [$renderWidget GetWidgetName]]

    #
    # use the prop picker to see if we're over the model, or the slices
    # - set the 'currentHit' variable accordingly for later processing
    # - sets the pointLabels variable for use in the cursor annotation
    # - NOTE: this code loops to make up for a limitation of the vtkPropPicker
    #   when dealing with the textured slices (vtk does not take into account
    #   the object viewed "through" the transparent part).
    #

    set ::QA(tempInvisibleProps) ""
    set pointLabels ""
    while { $pointLabels == "" } {
      set _useMID [QueryAtlasPickProp $x $y $renderer $viewer]
      if { $::QA(currentHit) == "QuerySlice"} {
        set pointLabels [QueryAtlasPickOnQuerySlice $x $y $renderer]
      } else {
        break
      }
    }

    # restore visibility of any slices that had non-labled sections in the way
    foreach prop $::QA(tempInvisibleProps) {
      $prop SetVisibility 1
    }



    #--- did we hit a model?
    if { $::QA(currentHit) == "QueryModel" } {
        #
        # get the color under the mouse from label image
        #
        set color ""
        foreach c {0 1 2 3} {
            lappend color [[$::QA(windowToImage) GetOutput] GetScalarComponentAsFloat $x $y 0 $c]
        }
        #
        # convert the color to a cell index and get the cooresponding
        # label names from the vertices
        #
        set cellNumber [QueryAtlasRGBAToNumber $color]
        if { $cellNumber >= 0 && $cellNumber < [$::QA(polyData_$_useMID) GetNumberOfCells] } {
            set cell [$::QA(polyData_$_useMID) GetCell $cellNumber]

            set labels [[$::QA(polyData_$_useMID) GetPointData] GetScalars "labels"]
            set points [$::QA(polyData_$_useMID) GetPoints]

            set m $_useMID
            array set labelMap $::QA(labelMap_$m)
            set pointLabels ""
            set numberOfPoints [$cell GetNumberOfPoints]

            set nearestRAS "0 0 0"
            set nearestIndex ""
            set nearestDistance 1000000

            for {set p 0} {$p < $numberOfPoints} {incr p} {
                set index [$cell GetPointId $p]
                set ras [$points GetPoint $index]
                set xy [eval QueryAtlasWorldToScreen $ras]
                set dist [QueryAtlasDistance $xy "$x $y"]
                if { $dist < $nearestDistance } {
                    set nearestDistance $dist
                    set nearestIndex $index
                    set nearestRAS $ras
                }
            }
            
            if { $nearestIndex != "" } {
                set ::QA(CurrentRASPoint) $nearestRAS
                set pointLabel [$labels GetValue $index]
                if { [info exists labelMap($pointLabel)] } {
                    set pointLabels $labelMap($pointLabel)
                } else {
                    lappend pointLabels ""
                }
            }
        }
    }

    # - nothing is hit yet, so check the cards
    if { $pointLabels == "" } {
        set pointLabels "background"
    }

    #---keep raw local label here
    set ::QA(localLabel) $pointLabels

    #---Before modifying freesurfer labelname,
    # get UMLS, Neuronames, BIRNLex mapping.
    #---  
    set transLabel [ QueryAtlasTranslateLabel  $pointLabels ]
    if { ![info exists ::QA(lastLabels)] } {
        set ::QA(lastLabels) ""
    }

    
    if { $transLabel != $::QA(lastLabels) } {
        set ::QA(lastLabels) $transLabel
    }
    
    QueryAtlasUpdateCursor
}


#----------------------------------------------------------------------------------------------------
#--- Generate the text label
#----------------------------------------------------------------------------------------------------
proc QueryAtlasUpdateCursor {} {

  set viewer [$::slicer3::ApplicationGUI GetViewerWidget]

  if { ![info exists ::QA(cursor,actor)] } {
      set ::QA(cursor,actor) [vtkTextActor New]
      set ::QA(cursor,mapper) [vtkTextMapper New]
      $::QA(cursor,actor) SetMapper $::QA(cursor,mapper)
      [$::QA(cursor,actor) GetTextProperty] ShadowOn
      [$::QA(cursor,actor) GetTextProperty] SetFontSize 20
      [$::QA(cursor,actor) GetTextProperty] SetFontFamilyToTimes

      set renderWidget [$viewer GetMainViewer]
      set renderer [$renderWidget GetRenderer]
      $renderer AddActor2D $::QA(cursor,actor)
  }

  if { [info exists ::QA(lastLabels)] && [info exists ::QA(lastWindowXY)] && [info exists ::QA(cursor,mapper)] } {
      $::QA(cursor,mapper) SetInput $::QA(lastLabels) 
      #--- position the text label just higher than the cursor
      foreach {x y} $::QA(lastWindowXY) {}
      set y [expr $y + 15]
      $::QA(cursor,actor) SetPosition $x $y
      $viewer RequestRender
  } 
}


#----------------------------------------------------------------------------------------------------
#--- Turn model visibility off, and its query model's visibility off too.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetQueryModelInvisible { } {

    if { [ info exists ::QA(annoModeNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs)  ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            $dnode SetVisibility 0
            set ::QA(actor_$mid,visibility) 0
        }
    }
}



#----------------------------------------------------------------------------------------------------
#--- Make a model visible, and it's query model visible too.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetQueryModelVisible { } {
    
    if { [ info exists ::QA(annoModeNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs)  ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            $dnode SetVisibility 1
            set ::QA(actor_$mid,visibility) 1
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetLHQueryModelInvisible { } {

    if { [info exists ::QA(annoModelNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            if { [ string first "lh." [$node GetName] ] >= 0 } {
                $dnode SetVisibility 0
                set ::QA(actor_$mid,visibility) 0
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetLHQueryModelVisible { } {
    
    if { [info exists ::QA(annoModelNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            if { [ string first "lh." [$node GetName] ] >= 0 } {
                $dnode SetVisibility 1
                set ::QA(actor_$mid,visibility) 1                    
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetRHQueryModelInvisible { } {

    if { [info exists ::QA(annoModelNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            if { [ string first "rh." [$node GetName] ] >= 0 } {
                $dnode SetVisibility 0
                set ::QA(actor_$mid,visibility) 0                    
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetRHQueryModelVisible { } {
    
    if { [info exists ::QA(annoModelNodeIDs)] } {
        set numQmodels [ llength $::QA(annoModelNodeIDs) ]
        for { set m 0 } {$m < $numQmodels } { incr m } {
            set mid [ lindex $::QA(annoModelNodeIDs) $m ]
            set node [ $::slicer3::MRMLScene GetNodeByID $mid ]
            set dnode [ $node GetDisplayNode ]
            if { [ string first "rh." [$node GetName] ] >= 0 } {
                $dnode SetVisibility 1
                set ::QA(actor_$mid,visibility) 1                    
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPing { } {
    puts "ping"
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetAnnotationTermSet { termset } {
    set ::QA(annotationTermSet) $termset
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTranslateLabel  { label } {
    
    if {$::QA(annotationTermSet) == "local" } {
        #set newlabel [ QueryAtlasVocabularyMapper $label "FreeSurfer" "FreeSurfer" ]
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "FreeSurfer" ]
    } elseif { $::QA(annotationTermSet) == "BIRNLex" } {
        #set newlabel [ QueryAtlasVocabularyMapper $label "FreeSurfer" "BIRN_String" ]
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "BIRN_String" ]
    } elseif { $::QA(annotationTermSet) == "NeuroNames" } {
        #set newlabel [ QueryAtlasVocabularyMapper $label "FreeSurfer" "NN_String" ]
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "NN_String" ]
    } elseif { $::QA(annotationTermSet) == "UMLS" } {
        #set newlabel [ QueryAtlasVocabularyMapper $label "FreeSurfer" "UMLS_CID" ]
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "UMLS_CID" ]
    } elseif { $::QA(annotationTermSet) == "IBVD" } {
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "IBVD" ]        
    } else {
        #--- assume to go local to local
        #set newlabel [ QueryAtlasVocabularyMapper $label "FreeSurfer" "FreeSurfer" ]
        set newlabel [ QueryAtlasMapTerm $label "FreeSurfer" "FreeSurfer" ]
    }
    return $newlabel
               
}


#----------------------------------------------------------------------------------------------------
#--- master switch to turn off annotations in main viewer
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetAnnotationsInvisible { } {
    set ::QA(annotationVisibility) 0
}

#----------------------------------------------------------------------------------------------------
#--- master switch to turn on annotations in main viewer
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetAnnotationsVisible { } {
    set ::QA(annotationVisibility) 1
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCursorVisibility { onoff } {

  if { $onoff == "on" } {
      #--- allows a master "switch" to turn off annotations
      #--- by default, they are on.
      if { $::QA(annotationVisibility) } {
          if { [ info exists ::QA(cursor,actor) ] } {
              $::QA(cursor,actor) SetVisibility 1
          }
      } else {
          if { [ info exists ::QA(cursor,actor) ] } {
              $::QA(cursor,actor) SetVisibility 0
          }
      }
  } else {
      if { [ info exists ::QA(cursor,actor) ] } {
          $::QA(cursor,actor) SetVisibility 0
      }
  }
  set viewer [$::slicer3::ApplicationGUI GetViewerWidget]
  $viewer RequestRender
}







#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMenuCreate { state } {

  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set interactor [$renderWidget GetRenderWindowInteractor] 
  set position [$interactor GetEventPosition]
  set ::QA(cardRASAnchor) $::QA(CurrentRASPoint)

  #
  # save the event position when the menu action started (when the right mouse
  # button was pressed) and only post the menu if the position is the same.  
  # If they aren't the same, do nothing since this was a dolly(zoom) action.
  #
  switch $state {
    "start" {
      set ::QA(menu,startPosition) $position
    }
    "end" {
      if { $::QA(menu,startPosition) == $position } {

        set ::QA(menuRAS) $::QA(CurrentRASPoint)

        set parent [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetWidgetName]
        set qaMenu $parent.qaMenu
        catch "destroy $qaMenu"
        menu $qaMenu

        if { $::QA(currentHit) == "Card" } { 
          # bring up a card menu
          set topic [file root [$::QA(currentCard) cget -text]]
          $qaMenu insert end command -label "Browse $topic" -command "$::slicer3::Application OpenLink $::QA(url,EntrezLinks)"
        } else {
          #--- bring up a search menu
          $qaMenu insert end command -label "Select structure term" -command "QueryAtlasSetStructureTerm"
          $qaMenu insert end command -label "Add to search terms" -command "QueryAtlasAddToSavedTerms"
          $qaMenu insert end command -label $::QA(lastLabels) -command ""
          $qaMenu insert end separator
          $qaMenu insert end command -label "Google..." -command "QueryAtlasContextQuery google"
          $qaMenu insert end command -label "Wikipedia..." -command "QueryAtlasContextQuery wikipedia"
          $qaMenu insert end command -label "PubMed..." -command "QueryAtlasContextQuery pubmed"
          $qaMenu insert end command -label "J Neuroscience..." -command "QueryAtlasContextQuery jneurosci"
          $qaMenu insert end command -label "IBVD form..." -command "QueryAtlasContextQuery \"ibvd form\""
          $qaMenu insert end command -label "IBVD howbig?..." -command "QueryAtlasContextQuery \"ibvd: howbig?\""
          $qaMenu insert end command -label "BrainInfo..." -command "QueryAtlasContextQuery braininfo"
        }
        
        foreach {x y} $::QA(lastRootXY) {}
        $qaMenu post $x $y
      }
    }
  }

}





#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasLaunchOntologyBrowser {} {

    #--- only open if not already running
    set already_running [ QueryAtlasOntologyViewerCheck ]
    if { $already_running } {
        puts "Ontology browser already appears to be running."
        return
    }

    puts "Launching ontology browser"
    #--- start it up
    set ::QA(ontologyHost) "localhost"
    set ::QA(ontologyPort) 3334
    set ::QA(ontologyViewerPID) ""
    set ::QA(ontologyBrowserRunning) 0
    
    #--- launch the viewer with a dataset
    #--- set directory of java stuff
    set dir $::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/OntologyViz

    #--- launch the browser on windows or other platforms and get PID
    if { $::tcl_platform(platform) == "windows" } {
        set ::QA(ontologyViewerPID) [ OntologyVizLaunch $dir 1 ]
    } else {
        set ::QA(ontologyViewerPID) [ OntologyVizLaunch $dir 0 ]            
    }

    #--- check to see if running....
    if { $::QA(ontologyViewerPID) == "" } {
        puts "QueryAtlasLaunchOntologyBrowser: tried and failed to launch Ontology Browser."
    } else {
        set ::QA(ontologyBrowserRunning) 1
    }

}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc OntologyVizLaunch {ontdir use_semicolon_separator} {

    set progbase "birnlexvis"
    set bindir [file join $ontdir "bin"]
    set datadir [file join $ontdir "data"]
    set extjardir [file join $ontdir "extjars"]
    
    #--- set classpath
    set cpath [list "$bindir/$progbase-support.jar" \
                   "$bindir/$progbase.jar" "$extjardir/json.jar" \
                   "$extjardir/prefuse.jar" "$extjardir/jython.jar"]

    if {$use_semicolon_separator} {
        set cpath [join "$cpath" \;]
    } else {
        set cpath [join "$cpath" :]
    }

    #--- return PID
    return [exec java -cp "$cpath" "$progbase" "$datadir/*.json" & ]

}




#----------------------------------------------------------------------------------------------------
#---TODO: test! Is the BirnLex Ontology viewer running?
#----------------------------------------------------------------------------------------------------
proc QueryAtlasOntologyViewerCheck { } {
    set running 0

    set str ""
    if { [ info exists ::QA(ontologyViewerPID) ] } {
        #--- check to see if the PID still exists
        if { $::tcl_platform(platform) == "windows" } {
            catch { set str [ exec ps --windows | grep java | grep $::QA(ontologyViewerPID) ] }
        } else {
            catch { set str [ exec ps -ef | grep java | grep $::QA(ontologyViewerPID) ] }
        }
        if { $str != "" } {
            set running 1
        }
    }
    return $running
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCloseOntologyBrowser { } {

    if { [ info exists ::QA(ontologyHost) ] && [ info exists ::QA(ontologyPort) ] } {

        set ::QA(socket) ""
        catch { set ::QA(socket) [ socket $::QA(ontologyHost) $::QA(ontologyPort) ] }

        if  { $::QA(socket) != "" } {
            #--- close
            puts $::QA(socket) "@quit"
            flush $::QA(socket)
            close $::QA(socket)
            #--- clean up
            unset -nocomplain ::QA(ontologyHost)
            unset -nocomplain ::QA(ontologyPort)
            unset -nocomplain ::QA(ontologyViewerPID) 
            set ::QA(ontologyBrowserRunning) 0
        }
    }
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSendHierarchyCommand { term ontology } {

    set running [ QueryAtlasOntologyViewerCheck ]
    if { $running } {
        #--- get command from Hierarchy frame widget
        
        puts "sending hierarchy command: term = $term"
        if { $term != "" } {
            set ::QA(socket) ""
            #--- make search reqest to birnlexviz demo if port/host are defined
            if { [ info exists ::QA(ontologyHost) ] && [ info exists ::QA(ontologyPort) ] } {
                catch { set ::QA(socket) [ socket $::QA(ontologyHost) $::QA(ontologyPort) ] }

                if  { $::QA(socket) != "" } {
                    #--- other stuff to do:
                    #puts $::QA(socket) "@listdatasets"
                    #puts $::QA(socket) "@quit"

                    if { $ontology == "BIRN" } {
                        puts "sending $term for BIRN"
                        puts $::QA(socket) "@query $term (birnlex)"
                    } else {
                        puts "sending $term for NN"
                        puts $::QA(socket) "@query $term (neuronames)"                    
                    }
                    flush $::QA(socket)
                    close $::QA(socket)
                }
            }
        }
    }
}


