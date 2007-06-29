#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: vtkFreeSurferReaders.tcl,v $
#   Date:      $Date: 2007/03/15 19:39:36 $
#   Version:   $Revision: 1.56 $
# 
#===============================================================================
# FILE:        vtkFreeSurferReaders.tcl
# PROCEDURES:  
#   vtkFreeSurferReadersInit
#   vtkFreeSurferReadersBuildGUI
#   vtkFreeSurferDemo
#   vtkFreeSurferReaderBuildVTK
#   vtkFreeSurferReadersEnter
#   vtkFreeSurferReadersExit
#   vtkFreeSurferReadersSetVolumeFileName
#   vtkFreeSurferReadersSetModelFileName
#   vtkFreeSurferReadersSetAnnotationFileName
#   vtkFreeSurferReadersSetAnnotColorFileName
#   vtkFreeSurferReadersApply
#   vtkFreeSurferReadersCORApply
#   vtkFreeSurferReadersMGHApply
#   vtkFreeSurferReadersMGHUpdateMRML
#   vtkFreeSurferReadersShowMGH i
#   vtkFreeSurferReadersBApply
#   vtkFreeSurferReadersBuildSurface m
#   vtkFreeSurferReadersSetSurfaceVisibility i vis
#   vtkFreeSurferReadersSetModelScalar modelID scalarName
#   vtkFreeSurferReadersSetModelVector modelID vectorName
#   vtkFreeSurferReadersMainFileCloseUpdate
#   vtkFreeSurferReadersAddColors tags
#   vtkFreeSurferReadersCORHeaderRead file
#   vtkFreeSurferReadersSetUMLSMapping
#   vtkFreeSurferReadersReadAnnotations  _id
#   vtkFreeSurferReadersReadAnnotation a _id annotFileName
#   vtkFreeSurferReadersCheckAnnotError
#   vtkFreeSurferReadersCheckWError
#   vtkFreeSurferReadersModelApply
#   vtkFreeSurferReadersModelCancel
#   vtkFreeSurferReadersSetLoad param
#   vtkFreeSurferReadersLoadVolume filename labelMap name
#   vtkFreeSurferReadersLoadModel filename name
#   FreeSurferReadersFiducialsPointCreatedCallback type fid pid
#   vtkFreeSurferReadersReadMGH v
#   vtkFreeSurferReadersReadBfloat v
#   vtkFreeSurferReadersSetCast
#   vtkFreeSurferReadersCast v toType
#   vtkFreeSurferReadersSetLoadColours
#   vtkFreeSurferReadersSetColourFileName
#   vtkFreeSurferReadersLoadColour overwriteFlag
#   vtkFreeSurferReadersGDFInit
#   vtkFreeSurferReadersGDFPlotBuildWindow iID
#   vtkFreeSurferReadersGDFPlotBuildDynamicWindowElements iID
#   vtkFreeSurferReadersPlotParseHeader ifnHeader
#   vtkFreeSurferReadersPlotPlotData iID dID
#   vtkFreeSurferReadersPlotCalculateSubjectMeasurement iID inSubject
#   vtkFreeSurferReadersGDFPlotHilightElement iID iElement
#   vtkFreeSurferReadersGDFPlotUnhilightElement iID iElement
#   vtkFreeSurferReadersGDFPlotToggleVisibility iID iElement
#   vtkFreeSurferReadersGDFPlotUnfocusElement iID
#   vtkFreeSurferReadersGDFPlotFocusElement iID iElement inSubjInClass iX iY
#   vtkFreeSurferReadersGDFPlotFindMousedElement iID iX iY
#   vtkFreeSurferReadersGDFPlotGetSubjectIndexFromID iID iSubjID
#   vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel iID iLabel
#   vtkFreeSurferReadersGDFPlotCBCloseWindow iID
#   vtkFreeSurferReadersGDFPlotCBLegendEnter iID igw
#   vtkFreeSurferReadersGDFPlotCBLegendLeave iID igw
#   vtkFreeSurferReadersGDFPlotCBLegendClick iID igw
#   vtkFreeSurferReadersGDFPlotCBGraphMotion iID igw iX iY
#   vtkFreeSurferReadersGDFPlotRead ifnHeader
#   vtkFreeSurferReadersGDFPlotPrint iID
#   vtkFreeSurferReadersPlotShowWindow iID
#   vtkFreeSurferReadersPlotHideWindow iID
#   vtkFreeSurferReadersPlotSetVariable iID vID
#   vtkFreeSurferReadersPlotSetMode iID iMode
#   vtkFreeSurferReadersPlotSetNthClassMarker iID inClass iMarker
#   vtkFreeSurferReadersPlotSetNthClassColor iID inClass iColor
#   vtkFreeSurferReadersPlotSetPoint iID iX iY iZ
#   vtkFreeSurferReadersPlotBeginPointList iID
#   vtkFreeSurferReadersPlotAddPoint iID iX iY iZ
#   vtkFreeSurferReadersPlotEndPointList iID
#   vtkFreeSurferReadersPlotSetInfo iID isInfo
#   vtkFreeSurferReadersPlotSaveToTable iID ifnTable
#   vtkFreeSurferReadersPlotSaveToPostscript iID ifnPS
#   vtkFreeSurferReadersSetPlotFileName
#   vtkFreeSurferReadersPlotApply
#   vtkFreeSurferReadersPlotCancel
#   vtkFreeSurferReadersUncompressMGH 
#   vtkFreeSurferReadersSetQADirName startdir
#   vtkFreeSurferReadersSetQASubjectsFileName
#   vtkFreeSurferReadersSetQASubjects
#   vtkFreeSurferReadersLoadQA fname
#   vtkFreeSurferReadersQASetLoad voltype
#   vtkFreeSurferReadersQASetLoadAddNew
#   vtkFreeSurferReadersStartQA
#   vtkFreeSurferReadersQAResetSubjectsListBox
#   vtkFreeSurferReadersSetQAEdit
#   vtkFreeSurferReadersBuildQAInteractor subject vol
#   vtkFreeSurferReadersBuildQAInteractorNextSubject subject
#   vtkFreeSurferReadersRecordSubjectQA subject vol eval
#   vtkFreeSurferReadersResetTimeScale
#   vtkFreeSurferReadersQAResetTaskDelay
#   vtkFreeSurferReadersQASetTime t
#   vtkFreeSurferReadersStepFrame
#   vtkFreeSurferReadersReviewNextSubject subject
#   vtkFreeSurferReadersQAReviewSubject subject
#   vtkFreeSurferReadersBuildQAInteractorStop
#   vtkFreeSurferReadersQAStop
#   vtkFreeSurferReadersQASummary
#   vtkFreeSurferReadersQAMakeNewSubjectsCsh subjectsDir subset
#   vtkFreeSurferReadersPlotBuildPointList  pointID scalarVar
#   vtkFreeSurferReadersPickPlot widget x y
#   vtkFreeSurferReadersPickScalar widget x y
#   vtkFreeSurferReadersShowScalarValue mid pid val col
#   vtkFreeSurferReadersBuildModelScalarsGui
#   vtkFreeSurferReadersScalarSetLoadAddNew
#   vtkFreeSurferReadersAddLuts
#   vtkFreeSurferReadersPickScalarsLut parentButton
#   vtkFreeSurferReadersSetScalarFileName
#   vtkFreeSurferReadersEditScalarsLut
#   vtkFreeSurferReadersSetLutParam param
#   vtkFreeSurferReadersReadScalars m fileName
#   vtkFreeSurferReadersLoadScalarFile fileName
#   vtkFreeSurferReadersLoadAnnotationFile fileName
#==========================================================================auto=

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersInit
#  The "Init" procedure is called automatically by the slicer.  
#  It puts information about the module into a global array called Module, 
#  and it also initializes module-level variables.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersInit {} {
    global vtkFreeSurferReaders Module Volume Model env

    set m vtkFreeSurferReaders

    # Initialize module-level variables
    #------------------------------------
    # Description:
    #   Keep a global array with the same name as the module.
    #   This is a handy method for organizing the global variables that
    #   the procedures in this module and others need to access.
    #
    set vtkFreeSurferReaders(assocFiles) ""
    set vtkFreeSurferReaders(scalars) "thickness curv avg_curv sulc area"
    set vtkFreeSurferReaders(scalarsNew) ""
    set vtkFreeSurferReaders(surfaces) "inflated pial smoothwm sphere white orig"
    set vtkFreeSurferReaders(annots) "aparc cma_aparc" 
    set vtkFreeSurferReaders(castToShort) 1
    # flag to load in free surfer colour file when loading a label map
    set vtkFreeSurferReaders(loadColours) 1
    # flag to say that they've been loaded (is reset to zero when MainFileClose is called)
    set vtkFreeSurferReaders(coloursLoaded) 0
    # Set up the file name of the free surfer module's colour xml file, 
    # it's in the Module's tcl directory. Try setting it from the slicer home
    # environment variable first, otherwise, assume search  is starting from slicer home
    if {[info exists env(SLICER_HOME)] == 1} {
        set vtkFreeSurferReaders(colourFileName) [file join $env(SLICER_HOME) Modules vtkFreeSurferReaders tcl FreeSurferColorLUT.txt]
    } else {
        set vtkFreeSurferReaders(colourFileName) [file join Modules vtkFreeSurferReaders tcl FreeSurferColorLUT.txt]
    }

    # the default colour table file name
    set vtkFreeSurferReaders(colorTableFilename) [ExpandPath [file join $::PACKAGE_DIR_VTKFREESURFERREADERS ".." ".." ".." tcl "Simple_surface_labels2002.txt"]]

    # for loading in a specific scalar file
    set vtkFreeSurferReaders(scalarFileName) ""

    # freesurfer specific colour scales
    set vtkFreeSurferReaders(lutNames) "RedGreen GreenRed Heat BlueRed"
    # when add the LUTs, append their ids to this list
    set vtkFreeSurferReaders(lutIDS) ""

    # for closing out a scene
    set vtkFreeSurferReaders(idList) ""
    set Module($m,procMainFileCloseUpdateEntered) vtkFreeSurferReadersMainFileCloseUpdate

    # set up the mapping between the surface labels and umls ids
    vtkFreeSurferReadersSetUMLSMapping


    # for QA 
    if {[info exist ::env(SUBJECTS_DIR)]} {
        set vtkFreeSurferReaders(QADirName) $::env(SUBJECTS_DIR)
        # try to read SUBJECTS env var from $SUBJECTS_DIR/scripts/subjects.csh
        if {[file exists [file join $::env(SUBJECTS_DIR) scripts subjects.csh]]} {
            if {$::Module(verbose)} { puts "Trying to read from subjects.csh in scripts subdir of $::env(SUBJECTS_DIR)" }
            set vtkFreeSurferReaders(QASubjectsFileName) [file join $::env(SUBJECTS_DIR) scripts subjects.csh]
            set vtkFreeSurferReaders(QAUseSubjectsFile) 1
            vtkFreeSurferReadersSetQASubjects
        }
    } else {
        set vtkFreeSurferReaders(QADirName) ""
    }

    # if this is not set to 1, will query user if they wish to look for subjects in the SUBJECTS_DIR dir
    # if too many subjects (fails with 2k) are there, slicer may hang
    set vtkFreeSurferReaders(QAAlwaysGlob) 0

    set vtkFreeSurferReaders(QAVolTypes) {aseg brain filled nu norm orig T1 wm}
    set vtkFreeSurferReaders(QADefaultVolTypes) {aseg norm}
    set vtkFreeSurferReaders(QAVolTypeNew) ""
    set vtkFreeSurferReaders(QAVolFiles) ""
    set vtkFreeSurferReaders(QAUseSubjectsFile) 0
    set vtkFreeSurferReaders(QADefaultView) "Normal"
    set vtkFreeSurferReaders(QAResultsList) {Approve Exclude Resegment Review}
    set vtkFreeSurferReaders(QAResultsListTooltips) {"Approve this segmentation as is" "Exclude this volume from further processing" "Flag this volume to be resegmented, as there are errors" "Get a second opinion on this volume"}
    set vtkFreeSurferReaders(scan) 0
    set vtkFreeSurferReaders(scanStep) 1
    set vtkFreeSurferReaders(scanStartCOR) -128
    set vtkFreeSurferReaders(scanStartSAG) -128
    set vtkFreeSurferReaders(scanMs) 2000
    set vtkFreeSurferReaders(QAEdit) 0
    set  vtkFreeSurferReaders(QAClose) 1
    set vtkFreeSurferReaders(QASubjectFileName) "QA.log"
    set vtkFreeSurferReaders(QASubjects) ""
    set vtkFreeSurferReaders(MGHDecompressorExec) "/local/os/bin/gunzip"
    set vtkFreeSurferReaders(QAOpacity) "0.5"
    set vtkFreeSurferReaders(QAtime) 0
    # save these settings and put them back after done qa
    set vtkFreeSurferReaders(QAcast) $vtkFreeSurferReaders(castToShort)
    set vtkFreeSurferReaders(sliceOpacity) $::Slice(opacity)
    set vtkFreeSurferReaders(QAviewmode) $::View(mode)

#    set vtkFreeSurferReaders(PlotFileName) [file normalize [file join .. .. freesurfer data fsgd y_doss-thickness-250rh.fsgd]]

    lappend Module($m,fiducialsPointCreatedCallback) FreeSurferReadersFiducialsPointCreatedCallback

    # Module Summary Info
    #------------------------------------
    # Description:
    #  Give a brief overview of what your module does, for inclusion in the 
    #  Help->Module Summaries menu item.
    set Module($m,overview) "This module allows you to read in FreeSurfer volume, model, and scalar files."
    #  Provide your name, affiliation and contact information so you can be 
    #  reached for any questions people may have regarding your module. 
    #  This is included in the  Help->Module Credits menu item.
    set Module($m,author) "Nicole Aucoin, BWH, nicole@bwh.harvard.edu"
    set Module($m,category) "IO"

    # Define Tabs
    #------------------------------------
    # Description:
    #   Each module is given a button on the Slicer's main menu.
    #   When that button is pressed a row of tabs appear, and there is a panel
    #   on the user interface for each tab.  If all the tabs do not fit on one
    #   row, then the last tab is automatically created to say "More", and 
    #   clicking it reveals a second row of tabs.
    #
    #   Define your tabs here as shown below.  The options are:
    #   row1List = list of ID's for tabs. (ID's must be unique single words)
    #   row1Name = list of Names for tabs. Names appear on the user interface
    #              and can be non-unique with multiple words.
    #   row1,tab = ID of initial tab
    #   row2List = an optional second row of tabs if the first row is too small
    #   row2Name = like row1
    #   row2,tab = like row1 
    #

    set Module($m,row1List) "Help Display Volumes Models Plot QA"
    set Module($m,row1Name) "{Help} {Display} {Volumes} {Models} {Plot} {QA}"
    set Module($m,row1,tab) Volumes

    # Define Procedures
    #------------------------------------
    # Description:
    #   The Slicer sources *.tcl files, and then it calls the Init
    #   functions of each module, followed by the VTK functions, and finally
    #   the GUI functions. A MRML function is called whenever the MRML tree
    #   changes due to the creation/deletion of nodes.
    #   
    #   While the Init procedure is required for each module, the other 
    #   procedures are optional.  If they exist, then their name, which
    #   can be anything, is registered with a line like this:
    #
    #   set Module($m,procVTK) vtkFreeSurferReadersBuildVTK
    #
    #   All the options are:

    #   procGUI   = Build the graphical user interface
    #   procVTK   = Construct VTK objects
    #   procMRML  = Update after the MRML tree changes due to the creation
    #               of deletion of nodes.
    #   procEnter = Called when the user enters this module by clicking
    #               its button on the main menu
    #   procExit  = Called when the user leaves this module by clicking
    #               another modules button
    #   procCameraMotion = Called right before the camera of the active 
    #                      renderer is about to move 
    #   procStorePresets  = Called when the user holds down one of the Presets
    #               buttons.
    #               
    #   Note: if you use presets, make sure to give a preset defaults
    #   string in your init function, of the form: 
    #   set Module($m,presets) "key1='val1' key2='val2' ..."
    #   
    set Module($m,procGUI) vtkFreeSurferReadersBuildGUI
    set Module($m,procVTK) vtkFreeSurferReadersBuildVTK
    set Module($m,procEnter) vtkFreeSurferReadersEnter
    set Module($m,procExit) vtkFreeSurferReadersExit
    set Module($m,procMainFileCloseUpdateEntered) vtkFreeSurferReadersMainFileCloseUpdate

    # Define Dependencies
    #------------------------------------
    # Description:
    #   Record any other modules that this one depends on.  This is used 
    #   to check that all necessary modules are loaded when Slicer runs.
    #   
    set Module($m,depend) ""

    # register the procedures in this file that will read in volumes
    set Module($m,readerProc,MGH) vtkFreeSurferReadersReadMGH
    set Module($m,readerProc,bfloat) vtkFreeSurferReadersReadBfloat

    # Set version info
    #------------------------------------
    # Description:
    #   Record the version number for display under Help->Version Info.
    #   The strings with the $ symbol tell CVS to automatically insert the
    #   appropriate revision number and date when the module is checked in.
    #   
    lappend Module(versions) [ParseCVSInfo $m \
        {$Revision: 1.56 $} {$Date: 2007/03/15 19:39:36 $}]
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildGUI
# Builds the GUI for the free surfer readers as it's own module.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildGUI {} {
    global Gui Volume Module vtkFreeSurferReaders

    if {$Module(verbose) == 1} {
        puts  "vtkFreeSurferReadersBuildGUI"
    }

    #-------------------------------------------
    # Help frame
    #-------------------------------------------
    set help "The vtkFreeSuferReaders module allows you to read in FreeSufer format files, and display the data.<P>Description by tab:<BR><UL><LI><B>Display</B>: Change the display settings for FreeSurfer volumes and models: colour definitions (segmentation and parcellation), scalar overlays for models  and their color palettes (load .w files here).<LI><B>Volumes</B>: Load in COR, mgh and mgz, bshort and bfloat volumes. Segmentations (label maps) can auto-load FS colours from a separate file.<LI><B>Models</B>: Load in model files (${vtkFreeSurferReaders(surfaces)}) and associate scalars with them.(${vtkFreeSurferReaders(scalars)}). Use a right mouse button click on models with scalar overlays to view the scalar values and colour mapping.<LI><B>Plot</B>: plot statistical data from group studies. Load in a group descriptor file and associate it with a model, right click on the model to plot data at that vertex.<LI><B>QA</B>: under development: Load in a series of freesurfer volume files for quality assurance - scans through the volume slices according to user set options, writes out notes and decision by reviewer to files in the subjects directory (Summarise QA results button will pop up all subjects, allow set up of subject subsets for further review). Select a subjects directory and volume types to load then press Start QA."
    regsub -all "\n" $help {} help
    MainHelpApplyTags vtkFreeSurferReaders $help
    MainHelpBuildGUI vtkFreeSurferReaders

    #-------------------------------------------
    # Display Frame
    #-------------------------------------------
    set fDisplay $Module(vtkFreeSurferReaders,fDisplay)
    set f $fDisplay
    DevAddLabel $f.lDisplay "Display Options:"
    pack $f.lDisplay -side top -padx $Gui(pad) -pady 0

    foreach subf {Colors AnnotColor Scalars} {
        frame $f.f${subf} -bg $Gui(activeWorkspace)
        pack $f.f${subf} -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    }

    #-------------------------------------------
    # Display-> Color Frame
    #-------------------------------------------
    set f $fDisplay.fColors

    eval {checkbutton $f.cLoadColours \
              -text "Auto Load FS Colors w/ASEG" -variable vtkFreeSurferReaders(loadColours) -width 27 \
              -indicatoron 0 -command "vtkFreeSurferReadersSetLoadColours"} $Gui(WCA)
    TooltipAdd $f.cLoadColours "Load in a FreeSurfer colour definition file when loading a label map.\nWARNING: will override other colours, use at your own risk."
    pack $f.cLoadColours -side top -padx $Gui(pad)
    DevAddFileBrowse $f vtkFreeSurferReaders "colourFileName" "Colour file:" "vtkFreeSurferReadersSetColourFileName" "txt xml" "\$Volume(DefaultDir)" "Open" "Browse for a FreeSurfer colors file (xml txt)"


    DevAddButton $f.bLoadColours "Load" "vtkFreeSurferReadersLoadColour 1"
    TooltipAdd $f.bLoadColours "Load the colour file now. Warning: deletes other colours, previous models' colours will be reset"
    pack $f.bLoadColours -side top 

    #-------------------------------------------
    # Display->Annotation Color Frame
    #-------------------------------------------
    DevAddFileBrowse $fDisplay.fAnnotColor vtkFreeSurferReaders "colorTableFilename" "Annotation Color file:" "vtkFreeSurferReadersSetAnnotColorFileName" "txt" {[file dirname $::Model(FileName)]} "Open" "Browse for a FreeSurfer annotation colour table file (txt)"
    TooltipAdd $fDisplay.fAnnotColor.efile "Set the annotation colour file containing the colour look up table used when a parcellation file is read"

    #-------------------------------------------
    # Display->Scalars Frame
    #-------------------------------------------
    set f $fDisplay.fScalars

    foreach subf {Active Scalar Picking Annot} {
        frame $f.f${subf} -bg $Gui(activeWorkspace)
        pack $f.f${subf} -side top -padx 1 -pady 1 -fill x -expand 1
    }

    #-------------------------------------------
    # Display->Scalars->Active Frame
    #-------------------------------------------
    set f $fDisplay.fScalars.fActive

    # for picking the active model
    eval {label $f.lActive -text "Active Model: "} $Gui(WLA)
    eval {menubutton $f.mbActive -text "None" -relief raised -bd 2 -width 20 \
            -menu $f.mbActive.m} $Gui(WMBA)
    eval {menu $f.mbActive.m} $Gui(WMA)
    pack $f.lActive $f.mbActive -side top
    # Append widgets to list that gets refreshed during UpdateMRML
    lappend ::Model(mbActiveList) $f.mbActive
    lappend ::Model(mActiveList)  $f.mbActive.m

    #-------------------------------------------
    # Display->Scalars->Scalar Frame
    #-------------------------------------------
    set f $fDisplay.fScalars.fScalar
    DevAddFileBrowse $f vtkFreeSurferReaders "scalarFileName" "Scalar (Overlay) file:" "vtkFreeSurferReadersSetScalarFileName" "thickness curv avg_curv sulc area w" {[file dirname $::Model(FileName)]} "Open" "Browse for a FreeSurfer scalar overlay file for the active model (thickness curv avg_curv sulc area w)"
    eval {button $f.bLoad -text "Load Scalar File" -width 12 -command "vtkFreeSurferReadersLoadScalarFile"} $Gui(WBA)
    TooltipAdd $f.bLoad "Load the scalar file and associate it with the active model"
    pack $f.bLoad  -side top -pady 1 -padx 1

    #-------------------------------------------
    # Display->Scalars->Picking Frame
    #-------------------------------------------
    set f $fDisplay.fScalars.fPicking
    DevAddButton $f.bPick "Pick Scalars" "ModelsPickScalars $f.bPick; Render3D" 12
    TooltipAdd $f.bPick "Pick scalars to display for active model"
    DevAddButton $f.bPickLut "Pick Palette" "vtkFreeSurferReadersPickScalarsLut $f.bPickLut; Render3D" 12
    TooltipAdd $f.bPickLut "Pick which look up table to use to map scalars for active model"
    DevAddButton $f.bEditLut "Edit Palette" "vtkFreeSurferReadersEditScalarsLut" 13
    pack $f.bPick $f.bPickLut $f.bEditLut -side left -pady 1 -padx 1

    #-------------------------------------------
    # Display->Scalars->Annot Frame
    #-------------------------------------------
    set f $fDisplay.fScalars.fAnnot
    DevAddFileBrowse $f vtkFreeSurferReaders "annotFileName" "Annotation file:" "vtkFreeSurferReadersSetAnnotationFileName" "" {[file dirname $::Model(FileName)]} "Open" "Browse for a FreeSurfer annotation file for the active model (annot)"
    eval {button $f.bLoad -text "Load Annotation File" -width 23 -command "vtkFreeSurferReadersReadAnnotation annot"} $Gui(WBA)
    TooltipAdd $f.bLoad "Load the scalar file and associate it with the active model"
    pack $f.bLoad  -side top -pady 1 -padx 1

    #-------------------------------------------
    # Volumes Frame
    #-------------------------------------------
    set fVolumes $Module(vtkFreeSurferReaders,fVolumes)
    set f $fVolumes

    frame $f.fVolume  -bg $Gui(activeWorkspace) -relief groove -bd 1
    frame $f.fLogo -bg $Gui(activeWorkspace)
    frame $f.fDemo -bg $Gui(activeWorkspace) -relief groove -bd 1
    pack $f.fLogo $f.fVolume $f.fDemo \
        -side top -fill x -pady $Gui(pad)

    #-------------------------------------------
    # Logo frame
    #-------------------------------------------
    set f $fVolumes.fLogo
    set logoFile [ExpandPath [file join $::PACKAGE_DIR_VTKFREESURFERREADERS ".." ".." ".." tcl images "FreeSurferlogo.ppm"]]
    if {[file exists $logoFile]} {
        image create photo iFSLogo -file $logoFile
        eval {label $f.lLogo -image iFSLogo -width 179 -height 30 -anchor center} \
            -bg $Gui(activeWorkspace)   -padx 0 -pady 0 
    } else {
        eval {label $f.lLogo -text "FreeSurfer" -anchor center} $Gui(WLA)
    }
    pack $f.lLogo 

    #-------------------------------------------
    # fVolume frame
    #-------------------------------------------

    set f $fVolumes.fVolume

    DevAddFileBrowse $f  vtkFreeSurferReaders "VolumeFileName" "FreeSurfer File:" "vtkFreeSurferReadersSetVolumeFileName" "mgz mgh info bhdr" "\$Volume(DefaultDir)" "Open" "Browse for a FreeSurfer volume file (.info, .mgh, .mgz, .bhdr)" 

    frame $f.fLabelMap -bg $Gui(activeWorkspace)
    frame $f.fCast  -bg $Gui(activeWorkspace)
    # frame $f.fColours -bg $Gui(activeWorkspace)
    frame $f.fDesc     -bg $Gui(activeWorkspace)
    frame $f.fName -bg $Gui(activeWorkspace)
    frame $f.fApply  -bg $Gui(activeWorkspace)

    pack $f.fLabelMap -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    pack $f.fCast -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    # pack $f.fColours -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    pack $f.fDesc -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    pack $f.fName -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    pack $f.fApply -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    #---------
    # Volume->Name 
    #---------
    set f $fVolumes.fVolume.fName
    eval {label $f.lName -text "Name:"} $Gui(WLA)
    eval {entry $f.eName -textvariable Volume(name) -width 13} $Gui(WEA)
    pack  $f.lName -side left -padx $Gui(pad) 
    pack $f.eName -side left -padx $Gui(pad) -expand 1 -fill x
    pack $f.lName -side left -padx $Gui(pad) 

    #---------
    # Volume->Desc
    #---------
    set f $fVolumes.fVolume.fDesc

    eval {label $f.lDesc -text "Optional Description:"} $Gui(WLA)
    eval {entry $f.eDesc -textvariable Volume(desc)} $Gui(WEA)
    pack $f.lDesc -side left -padx $Gui(pad)
    pack $f.eDesc -side left -padx $Gui(pad) -expand 1 -fill x

    #---------
    # Volume->LabelMap
    #---------
    set f $fVolumes.fVolume.fLabelMap

    frame $f.fTitle -bg $Gui(activeWorkspace)
    frame $f.fBtns -bg $Gui(activeWorkspace)

    pack $f.fTitle $f.fBtns -side left -pady 5

    DevAddLabel $f.fTitle.l "Image Data:"
    pack $f.fTitle.l -side left -padx $Gui(pad) -pady 0

    foreach text "{Grayscale} {Label Map}" \
        value "0 1" \
        width "9 9 " {
        eval {radiobutton $f.fBtns.rMode$value -width $width \
            -text "$text" -value "$value" -variable Volume(labelMap) \
            -indicatoron 0 } $Gui(WCA)
        pack $f.fBtns.rMode$value -side left -padx 0 -pady 0
    }
    if {$Module(verbose) == 1} {
        puts "Done packing the label map stuff"
    }

    #------------
    # Volume->Cast 
    #------------
    set f $fVolumes.fVolume.fCast
    eval {checkbutton $f.cCastToShort \
        -text "Cast to short" -variable vtkFreeSurferReaders(castToShort) -width 13 \
        -indicatoron 0 -command "vtkFreeSurferReadersSetCast"} $Gui(WCA)
    TooltipAdd $f.cCastToShort "Cast this volume to short when reading it in. This allows use of the editing tools."
    pack $f.cCastToShort -side top -padx 0

    #------------
    # Volume->Apply 
    #------------
    set f $fVolumes.fVolume.fApply
        
    DevAddButton $f.bApply "Apply" "vtkFreeSurferReadersApply; RenderAll" 8
    DevAddButton $f.bCancel "Cancel" "VolumesPropsCancel" 8
    grid $f.bApply $f.bCancel -padx $Gui(pad)

    #-------------------------------------------
    # Demo Button
    #-------------------------------------------
    set f $fVolumes.fDemo
    eval button $f.bDemo -text Demo -command "vtkFreeSurferDemo" $Gui(WBA)
#    pack $f.bDemo  -side left -padx $Gui(pad) -pady 0

    #-------------------------------------------
    # Model frame
    #-------------------------------------------
    set fModel $Module(vtkFreeSurferReaders,fModels)
    set f $fModel

    DevAddFileBrowse $f  vtkFreeSurferReaders "ModelFileName" "Model File:" "vtkFreeSurferReadersSetModelFileName" "orig ${vtkFreeSurferReaders(surfaces)}" {[file dirname $::Model(FileName)]} "Open" "Browse for a FreeSurfer surface file (orig ${vtkFreeSurferReaders(surfaces)})"
    frame $f.fName -bg $Gui(activeWorkspace)
    frame $f.fSurface -bg $Gui(activeWorkspace)
    frame $f.fScalar -bg $Gui(activeWorkspace)
    frame $f.fScalarAdd  -bg $Gui(activeWorkspace)
    frame $f.fAnnotation -bg $Gui(activeWorkspace)
    # frame $f.fAnnotColor -bg $Gui(activeWorkspace)
    frame $f.fApply  -bg $Gui(activeWorkspace)
 
    pack $f.fName -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    pack $f.fSurface -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    pack $f.fScalar -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    pack $f.fScalarAdd -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    pack $f.fAnnotation -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
   # pack $f.fAnnotColor -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
   # DevAddFileBrowse $f.fAnnotColor vtkFreeSurferReaders "colorTableFilename" "Annotation Color file:" "vtkFreeSurferReadersSetAnnotColorFileName" "txt" "\$Volume(DefaultDir)" "Open" "Browse for a FreeSurfer annotation colour table file (txt)"

    pack $f.fApply -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1

    #------------
    # Model->Name
    #------------
    set f $Module(vtkFreeSurferReaders,fModels).fName
    eval {label $f.lName -text "Name:"} $Gui(WLA)
    eval {entry $f.eName -textvariable vtkFreeSurferReaders(ModelName) -width 8} $Gui(WEA)
    pack  $f.lName -side left -padx $Gui(pad) 
    pack $f.eName -side left -padx $Gui(pad) -expand 1 -fill x
    pack $f.lName -side left -padx $Gui(pad) 

    if {0} {
        # this is implicit in the browsing for a model file
    #------------
    # Model->Surface 
    #------------
    # surface files (mesh): e.g., lh.inflated, lh.pial, lh.smoothwm, lh.sphere
    set f $Module(vtkFreeSurferReaders,fModels).fSurface
    DevAddLabel $f.lTitle "Load Associated Surface Models:"
    pack $f.lTitle -side top -padx $Gui(pad) -pady 0
    foreach surface $vtkFreeSurferReaders(surfaces) {
        eval {checkbutton $f.c$surface \
                  -text $surface -command "vtkFreeSurferReadersSetLoad $surface " \
                  -variable vtkFreeSurferReaders(assocFiles,$surface) \
                  -width 9 \
                  -indicatoron 0} $Gui(WCA)
        pack $f.c$surface -side top -padx 0
    }
}

    #------------
    # Model->Scalar 
    #------------
    # curvature (scalar): e.g., lh.thickness, lh.curv, lh.sulc, lh.area
    vtkFreeSurferReadersBuildModelScalarsGui
    
    #------------
    # Model->ScalarAdd
    #------------
    set f $Module(vtkFreeSurferReaders,fModels).fScalarAdd
    # let the user specify other scalar file extensions
    DevAddLabel $f.lScalarAdd "Add a scalar:"
    pack $f.lScalarAdd -side left -padx $Gui(pad)
    eval {entry $f.eScalarAdd -textvariable vtkFreeSurferReaders(scalarsNew)  -width 5} $Gui(WEA)
    pack $f.eScalarAdd -side left -padx $Gui(pad) -expand 1 -fill x
    TooltipAdd $f.eScalarAdd "Put the suffix of the new scalar type that you wish to load here"
    # now bind a proc to add this to the assocFiles list.
    bind $f.eScalarAdd <Return> "vtkFreeSurferReadersScalarSetLoadAddNew"

    #------------
    # Model->Annotation 
    #------------
    # annotation files: lh.xxx.annot
    set f $Module(vtkFreeSurferReaders,fModels).fAnnotation
    DevAddLabel $f.lTitle "Load Associated Annotation files:"
    pack $f.lTitle -side top -padx $Gui(pad) -pady 0
    foreach annot $vtkFreeSurferReaders(annots) {
        eval {checkbutton $f.c$annot \
                  -text "$annot" -command "vtkFreeSurferReadersSetLoad $annot" \
                  -variable vtkFreeSurferReaders(assocFiles,$annot) -width 9 \
                  -indicatoron 0} $Gui(WCA)
        pack $f.c$annot -side top -padx 0
    }

    #------------
    # Model->Apply 
    #------------
    set f $Module(vtkFreeSurferReaders,fModels).fApply
        
    DevAddButton $f.bApply "Apply" "vtkFreeSurferReadersModelApply; RenderAll" 8
    DevAddButton $f.bCancel "Cancel" "vtkFreeSurferReadersModelCancel" 8
    grid $f.bApply $f.bCancel -padx $Gui(pad)

    #-------------------------------------------
    # Plot frame
    #-------------------------------------------
    set fPlot $Module(vtkFreeSurferReaders,fPlot)
    set f $fPlot

    DevAddFileBrowse $f vtkFreeSurferReaders "PlotFileName" "Plot header file:" "vtkFreeSurferReadersSetPlotFileName" "fsgd" {[file dirname $::Model(FileName)]} "Open" "Browse for a plot header file (fsgd)"

    frame $f.fApply -bg $Gui(activeWorkspace)
    frame $f.fModel -bg $Gui(activeWorkspace)

    pack $f.fModel $f.fApply -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1

    #------------
    # Plot->Model
    #------------
    set f $Module(vtkFreeSurferReaders,fPlot).fModel

    DevAddLabel $f.lActive "Active Model: "
    TooltipAdd $f.lActive "Which model to associate this plot data with"

    eval {menubutton $f.mbActive -text "None" -relief raised -bd 2 -width 20 \
            -menu $f.mbActive.m} $Gui(WMBA)
    eval {menu $f.mbActive.m} $Gui(WMA)
    pack $f.lActive $f.mbActive -side left

    # Append widgets to list that gets refreshed during UpdateMRML
    lappend ::Model(mbActiveList) $f.mbActive
    lappend ::Model(mActiveList)  $f.mbActive.m

    #------------
    # Plot->Apply 
    #------------
    set f $Module(vtkFreeSurferReaders,fPlot).fApply

    DevAddButton $f.bApply "Apply" "vtkFreeSurferReadersPlotApply" 8
    DevAddButton $f.bCancel "Cancel" "vtkFreeSurferReadersPlotCancel" 8
    grid $f.bApply $f.bCancel -padx $Gui(pad)

    #-------------------------------------------
    # QA frame
    #-------------------------------------------
    set fQA $Module(vtkFreeSurferReaders,fQA)
    set f $fQA

    DevAddLabel $f.lWarning "UNDER CONSTRUCTION"
    pack $f.lWarning  -side top -padx $Gui(pad) -pady 0 -expand 1

    foreach subframe {Subjects Volumes Btns Options} {
        frame $f.f${subframe} -bg $Gui(activeWorkspace)
        pack $f.f${subframe} -side top  -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    }
    
    #---------------
    # QA -> Subjects
    #---------------
    set f $fQA.fSubjects

    DevAddLabel $f.lCurDir "Current Subjects directory:"
    eval {entry $f.eCurDir -textvariable vtkFreeSurferReaders(QADirName) -width 50} $Gui(WEA)
    bind $f.eCurDir <Return> "vtkFreeSurferReadersSetQADirName [$f.eCurDir get]"

    pack $f.lCurDir -padx $Gui(pad) -pady $Gui(pad) -expand 1 -fill x
    pack $f.eCurDir -padx $Gui(pad) -pady $Gui(pad) -expand 1 -fill x

    DevAddButton $f.bSetDir "Set the Subjects directory" {vtkFreeSurferReadersSetQADirName $vtkFreeSurferReaders(QADirName)} 
    TooltipAdd $f.bSetDir "Select a directory containing subjects for which you wish to load FreeSurfer data.\nValid subject directories contain an mri subdirectory"
    pack $f.bSetDir -padx $Gui(pad) -pady $Gui(pad)
    
    # a list of the subjects found in the directory
    set qaSubjectNameListBox [ScrolledListbox $f.slbQASubjects 0 0 -height 4 -bg $Gui(activeWorkspace) -selectmode multiple]
    # make the scroll bars a bit skinnier when they appear
    $f.slbQASubjects.xscroll configure -width 10
    $f.slbQASubjects.yscroll configure -width 10
    set vtkFreeSurferReaders(qaSubjectsListbox) $qaSubjectNameListBox
    pack $f.slbQASubjects  -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    # if the list of subjects has already been set, populate it (use subjects file flag is 0 until pick one)
    vtkFreeSurferReadersSetQASubjects
    vtkFreeSurferReadersQAResetSubjectsListBox

    DevAddLabel $f.lClick "Click on Subjects you wish to load for QA\nOR set a subjects file name below"
    pack $f.lClick  -side top -padx $Gui(pad) -pady 0

    # alternately, read in the list of subjects from a file
    DevAddFileBrowse $f vtkFreeSurferReaders "QASubjectsFileName" "File with subject list:" {vtkFreeSurferReadersSetQASubjectsFileName ; vtkFreeSurferReadersSetQASubjects ; vtkFreeSurferReadersQAResetSubjectsListBox } "csh" "\$vtkFreeSurferReaders(QADirName)" "Open" "Browse for the subjects.csh containing a list of subjects"
    TooltipAdd $f.f "Browse for subjects.csh containing list of subjects (can be generated through 'Summarise QA Results')"

    DevAddButton $f.bSummary "Summarise QA results" vtkFreeSurferReadersQASummary
    TooltipAdd $f.bSummary "Summarise QA tests run in this subjects directory, set up new subjects.csh"
    pack $f.bSummary

    #---------------
    # QA -> Volumes
    #---------------
    set f $fQA.fVolumes

    frame $f.fVolSelect  -bg $Gui(activeWorkspace)
    frame $f.fVolAdd  -bg $Gui(activeWorkspace)

    pack $f.fVolSelect  -side top -padx $Gui(pad) -pady 0 -fill x -expand 1
    pack  $f.fVolAdd  -side top -padx $Gui(pad) -pady 0 -fill x -expand 1

    #----------------------------
    # QA -> Volumes  -> VolSelect
    #----------------------------

    set f $fQA.fVolumes.fVolSelect

    DevAddLabel $f.lVolumesSelect "Volumes you wish to load for each subject:"
    pack $f.lVolumesSelect  -side top -padx $Gui(pad) -pady 0 -expand 1

    foreach voltype $vtkFreeSurferReaders(QAVolTypes) {
        # turn on the default ones
        if {[lsearch $vtkFreeSurferReaders(QADefaultVolTypes) $voltype] == -1} {
            set defaultOn 0
        } else {
            set defaultOn 1
        }
        eval {checkbutton $f.c$voltype \
                  -text $voltype -command "vtkFreeSurferReadersQASetLoad $voltype" \
                  -variable vtkFreeSurferReaders(QAVolFiles,$voltype) \
                  -indicatoron 0} $Gui(WCA)
        set vtkFreeSurferReaders(QAVolFiles,$voltype) $defaultOn
        vtkFreeSurferReadersQASetLoad $voltype
        pack $f.c$voltype -side left -padx 0
    }

    #-------------------------
    # QA -> Volumes  -> VolAdd
    #-------------------------

    set f $fQA.fVolumes.fVolAdd

    # let the user specify other volume names
    DevAddLabel $f.lVolumesAdd "Add a volume name:"
    pack $f.lVolumesAdd -side left -padx $Gui(pad)
    eval {entry $f.eVolumesAdd -textvariable vtkFreeSurferReaders(QAVolTypeNew)  -width 5} $Gui(WEA)
    pack $f.eVolumesAdd -side left -padx $Gui(pad) -expand 1 -fill x
    TooltipAdd $f.eVolumesAdd "Put the prefix of a volume you wish to load here, no extension (ie aseg2)"
    # now bind a proc to add this to the vol types list
    bind $f.eVolumesAdd <Return> "vtkFreeSurferReadersQASetLoadAddNew"

    #-----------
    # QA -> Btns
    #-----------
    set f $fQA.fBtns

    DevAddButton $f.bStart "Run QA" vtkFreeSurferReadersStartQA
    TooltipAdd $f.bStart "Start the QA process, loading the volumes for the selected subjects (if they exist)"
    pack $f.bStart -side top -padx $Gui(pad) -expand 1

    DevAddLabel $f.lTime "Scan through slices manually"
    eval {scale $f.sTime -from 0 -to 9 \
              -length 220 -resolution 1 \
              -command vtkFreeSurferReadersQASetTime } \
        $::Gui(WSA) {-sliderlength 22}
    set ::vtkFreeSurferReaders(timescale) $f.sTime
    if { ![catch "package require iSlicer"] } {
        if { [info command istask] != "" } { 
            istask $f.play \
                -taskcommand vtkFreeSurferReadersStepFrame \
                -taskdelay $vtkFreeSurferReaders(scanMs) \
                -labeltext "Auto scanning display:" \
                -labelfont {helvetica 8} \
                -background $Gui(activeWorkspace)
            # make the on off button look like a regular slicer one
            eval $Module(vtkFreeSurferReaders,fQA).fBtns.play.lwchildsite.f.oob configure $Gui(WBA)
            TooltipAdd $Module(vtkFreeSurferReaders,fQA).fBtns.play.lwchildsite.f.oob "Will scan through the current volume's slices"
            pack $f.play
        }
    }
    pack $f.lTime $f.sTime 

    #-----------
    # QA -> Options
    #-----------
    set f $fQA.fOptions
    $f configure -bd 1 -relief sunken

    eval {label $f.ltitle -text "Options:"} $Gui(WLA)
    pack $f.ltitle -expand 1

    foreach subf { ScanPause ScanStep ScanStart Edit Opacity } {
        frame $f.f${subf} -bg $Gui(activeWorkspace)
        pack $f.f${subf}  -side top -padx $Gui(pad) -pady $Gui(pad) -fill x -expand 1
    }

    # QA -> Options -> ScanPause
    set f $fQA.fOptions.fScanPause

    # auto scroll time interval
    eval {label $f.lscanpause -text "Scan pause milliseconds:"} $Gui(WLA)
    eval {entry $f.escanpause -textvariable vtkFreeSurferReaders(scanMs) -width 3} $Gui(WEA)
    # if the pause length changes, need to reset the delay on the scanning task
    bind $f.escanpause <Return> "vtkFreeSurferReadersQAResetTaskDelay"
    pack $f.lscanpause -side left -padx $Gui(pad) 
    pack $f.escanpause -side left -padx $Gui(pad) -expand 1 -fill x

    # QA -> Options -> ScanStep
    set f $fQA.fOptions.fScanStep

    # auto scroll slice step increment
    eval {label $f.lscanstep -text "Scan slice step:"} $Gui(WLA)
    eval {entry $f.escanstep -textvariable vtkFreeSurferReaders(scanStep) -width 3} $Gui(WEA)
    pack $f.lscanstep -side left -padx $Gui(pad) 
    pack $f.escanstep -side left -padx $Gui(pad) -expand 1 -fill x
    # reset the time scale on the slider
    bind $f.escanstep <Return> vtkFreeSurferReadersResetTimeScale

    # QA -> Options -> ScanStart
    set f $fQA.fOptions.fScanStart
    # which slice to start at?
    eval {label $f.lscanstartCOR -text "Start at COR slice"} $Gui(WLA)
    eval {entry $f.escanstartCOR -textvariable vtkFreeSurferReaders(scanStartCOR) -width 4} $Gui(WEA)
    pack $f.lscanstartCOR -side left -padx 0
    pack $f.escanstartCOR -side left -padx $Gui(pad) -expand 1 -fill x

    eval {label $f.lscanstartSAG -text "at SAG"} $Gui(WLA)
    eval {entry $f.escanstartSAG -textvariable vtkFreeSurferReaders(scanStartSAG) -width 4} $Gui(WEA)
    pack $f.lscanstartSAG -side left -padx 0
    pack $f.escanstartSAG -side left -padx $Gui(pad) -expand 1 -fill x

    # reset the scale on the slider
    bind $f.escanstartCOR <Return> vtkFreeSurferReadersResetTimeScale
    bind $f.escanstartSAG <Return> vtkFreeSurferReadersResetTimeScale


    # QA -> Options -> Edit
    set f $fQA.fOptions.fEdit
    # have editing enabled?
    eval {checkbutton $f.cQAEdit \
              -text "Allow Editing" -command "vtkFreeSurferReadersSetQAEdit" \
              -variable vtkFreeSurferReaders(QAEdit) \
              -width 13 \
              -indicatoron 0} $Gui(WCA)
    # close the volumes after done with QA?
    eval {checkbutton $f.cQAClose \
              -text "Unload volumes" -command "" \
              -variable vtkFreeSurferReaders(QAClose) \
              -width 15 \
              -indicatoron 0} $Gui(WCA)
    TooltipAdd $f.cQAClose "Remove subject volumes from the scene when done QA"
    pack $f.cQAEdit $f.cQAClose -side left -padx 5 -expand 1

    # QA -> Options -> Opacity
    set f $fQA.fOptions.fOpacity
    eval {label $f.lOpacity -text "Opacity of aseg"} $Gui(WLA)
    eval {entry $f.eOpacity -textvariable vtkFreeSurferReaders(QAOpacity) -width 4} $Gui(WEA)
    TooltipAdd $f.eOpacity "Opacity of foreground aseg volume, 0-1"
    pack $f.lOpacity -side left -padx 0
    pack $f.eOpacity -side left -padx $Gui(pad) -expand 1 -fill x

    # once everything's set up, reset the slider scale 
    vtkFreeSurferReadersResetTimeScale
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferDemo
# A hack for testing/development.  Assumes you have the correct data available 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferDemo {} {
    global env

    source $env(SLICER_HOME)/Modules/vtkFreeSurferReaders/tcl/regions.tcl
    source $env(SLICER_HOME)/Modules/vtkFreeSurferReaders/tcl/ccdb.tcl
    set r [regions #auto]
           
    $r demo
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReaderBuildVTK
# Does nothing right now, vtk models are built when model files are read in.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildVTK {} {
    global Module
    if {$Module(verbose) == 1} {puts "proc vtkFreeSurferReaders Build VTK"}

    # add our stuff to the look up table list
    vtkFreeSurferReadersAddLuts

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersEnter
# Does nothing yet
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersEnter {} {
    global Module vtkFreeSurferReaders
    if {$Module(verbose) == 1} {puts "proc vtkFreeSurferReaders ENTER"}

#    pushEventManager vtkFreeSurferReadersEventMgr
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersExit
# Deletes any gdfReaders
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersExit {} {
    global vtkFreeSurferReaders Module
    if {$Module(verbose) == 1} {puts "proc vtkFreeSurferReaders EXIT"}
    if {[info exist vtkFreeSurferReaders(gdfReader)]} {
        vtkFreeSurferReaders(gdfReader) Delete
    }

#    popEventManager vtkFreeSurferReadersEventMgr
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetVolumeFileName
# The filename is set elsehwere, in variable vtkFreeSurferReaders(VolumeFileName)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetVolumeFileName {} {
    global Volume Module vtkFreeSurferReaders

    if {$Module(verbose) == 1} {
        puts "FreeSurferReaders filename: $vtkFreeSurferReaders(VolumeFileName)"
    }
    if {[string equal [file extension $vtkFreeSurferReaders(VolumeFileName)] ".mgh"] ||
        [string equal [file extension $vtkFreeSurferReaders(VolumeFileName)] ".mgz"]} { 
        set Volume(name) [file rootname [file tail $vtkFreeSurferReaders(VolumeFileName)]]
    } elseif {[string equal [file extension $vtkFreeSurferReaders(VolumeFileName)] ".bfloat"] ||
              [string equal [file extension $vtkFreeSurferReaders(VolumeFileName)] ".bshort"]} {
        # use the stem of the file, the part between the last directory separator and the underscore
        set bdir [file dirname $vtkFreeSurferReaders(VolumeFileName)]
        regexp "$bdir/(.*)_.*" $vtkFreeSurferReaders(VolumeFileName) match Volume(name)
#        set Volume(name) [file rootname [file tail $vtkFreeSurferReaders(VolumeFileName)]]
    } else {
        # COR: make the volume name be the name of the directory rather than COR-.info
        set Volume(name) [file tail [file dirname $vtkFreeSurferReaders(VolumeFileName)]]
        # replace . with -
        regsub -all {[.]} $Volume(name) {-} Volume(name)
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetModelFileName
# The filename is set elsehwere, in variable vtkFreeSurferReaders(ModelFileName)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetModelFileName {} {
    global vtkFreeSurferReaders Volume Model Module

    if {$Module(verbose) == 1} {
        puts "FreeSurferReaders filename: $vtkFreeSurferReaders(ModelFileName)"
    }
    set Model(name) [file tail $vtkFreeSurferReaders(ModelFileName)]
    # replace . with -
    regsub -all {[.]} $Model(name) {-} Model(name)
    set vtkFreeSurferReaders(ModelName) $Model(name)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetAnnotationFileName
# The filename is set elsehwere, in variable vtkFreeSurferReaders(annotFilename)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetAnnotationFileName {} {
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetAnnotationFileName: file name was set to $::vtkFreeSurferReaders(annotFileName)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetAnnotColorFileName
# The filename is set elsehwere, in variable vtkFreeSurferReaders(colorTableFilename)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetAnnotColorFileName {} {
    global vtkFreeSurferReaders Volume
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetAnnotColorFileName: annotation colour file name set to $vtkFreeSurferReaders(colorTableFilename)"
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersApply
# Read in the freesurfer volume specified by calling the appropriate reader function.
# Return the volume id.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersApply {} {
    global vtkFreeSurferReaders Module Volume

    set vid ""
    # switch on the file name, it can be:
    # a COR file (*.info that defines the volume) 
    # an mgh file (*.mgh, volume in one file)
    # an mgz file (*.mgz or *.mgh.gz - a gzippped mgh volume)
    # a bfloat file (*.bfloat)
    if {[string match *.info $vtkFreeSurferReaders(VolumeFileName)]} {
        set vid [vtkFreeSurferReadersCORApply]
    } elseif {[string match *.mgh $vtkFreeSurferReaders(VolumeFileName)]} {
        set vid [vtkFreeSurferReadersMGHApply]
    } elseif {[string match *.mgz $vtkFreeSurferReaders(VolumeFileName)]} {
        # the mgh reader will read from a compressed file
        set vid [vtkFreeSurferReadersMGHApply]
    } elseif {[string match *.mgh.gz $vtkFreeSurferReaders(VolumeFileName)]} {
    set vid [vtkFreeSurferReadersMGHApply]
    } elseif {[string match *.bhdr $vtkFreeSurferReaders(VolumeFileName)]} {
        set vid [vtkFreeSurferReadersBApply]
    } else {
        DevErrorWindow "ERROR: Invalid file extension, file $vtkFreeSurferReaders(VolumeFileName) does not match info or mgh extensions for COR or MGH files, or bfloat or bshort extensions for binary files."
    }

    # allow use of other module GUIs
    set Volumes(freeze) 0

    # If tabs are frozen, then return to the "freezer"
    if {$Module(freezer) != ""} {
        set cmd "Tab $Module(freezer)"
        set Module(freezer) ""
        eval $cmd
    }

    return $vid
}
#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersCORApply
# Read in the freesurfer COR volume specified
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersCORApply {} {
    global Volume vtkFreeSurferReaders Module View Color

    # Validate name
    if {$Volume(name) == ""} {
        tk_messageBox -message "Please enter a name that will allow you to distinguish this volume."
        return
    }
    if {[ValidateName $Volume(name)] == 0} {
        tk_messageBox -message "The name can consist of letters, digits, dashes, or underscores"
        return
    }

    # add a mrml node
    set n [MainMrmlAddNode Volume]
    set i [$n GetID]
    # NOTE:
    # this normally happens in MainVolumes.tcl
    # this is needed here to set up reading
    # this should be fixed - the node should handle this somehow
    # so that MRML can be read in with just a node and this will
    # work
    # puts "vtkFreeSurferReadersApply: NOT calling MainVolumes create on $i"
    # MainVolumesCreate $i
  
    # read in the COR file
    # Set up the reading
    if {[info command Volume($i,vol,rw)] != ""} {
        # have to delete it first, needs to be cleaned up
        if {$::Module(verbose)} {
            DevErrorWindow "Problem: reader for this new volume number $i already exists, deleting it"
        } else {
            puts "Problem: reader for this new volume number $i already exists, deleting it"
        }
        Volume($i,vol,rw) Delete
    }

    vtkCORReader Volume($i,vol,rw)
    #  read the header first: sets the Volume array values we need
    vtkFreeSurferReadersCORHeaderRead $vtkFreeSurferReaders(VolumeFileName)
    
    # get the directory name from the filename
    Volume($i,vol,rw) SetFilePrefix [file dirname $vtkFreeSurferReaders(VolumeFileName)]
    if {$Module(verbose) == 1} {
        puts "vtkFreeSurferReadersApply: set prefix to [Volume($i,vol,rw) GetFilePrefix]\nCalling Update on volume $i"
    }
    Volume($i,vol,rw) Update

    # set the name and description of the volume
    $n SetName $Volume(name)
    $n SetDescription $Volume(desc)
    
   
    # set the volume properties: read the header first: sets the Volume array values we need
    # vtkFreeSurferReadersCORHeaderRead $vtkFreeSurferReaders(VolumeFileName)
    Volume($i,node) SetName $Volume(name)
    Volume($i,node) SetDescription $Volume(desc)
    Volume($i,node) SetLabelMap $Volume(labelMap)
    eval Volume($i,node) SetSpacing $Volume(pixelWidth) $Volume(pixelHeight) \
            [expr $Volume(sliceSpacing) + $Volume(sliceThickness)]
    Volume($i,node) SetTilt $Volume(gantryDetectorTilt)

    Volume($i,node) SetFilePattern $Volume(filePattern) 
    Volume($i,node) SetScanOrder $Volume(scanOrder)
    Volume($i,node) SetNumScalars $Volume(numScalars)
    Volume($i,node) SetLittleEndian $Volume(littleEndian)
    # this is the file prefix that will be used to build the image file names, needs to go up to COR
    Volume($i,node) SetFilePrefix [string trimright [file rootname $vtkFreeSurferReaders(VolumeFileName)] "-"]
# [Volume($i,vol,rw) GetFilePrefix]
    Volume($i,node) SetImageRange [lindex $Volume(imageRange) 0] [lindex $Volume(imageRange) 1]
    Volume($i,node) SetScalarTypeToUnsignedChar
    Volume($i,node) SetDimensions [lindex $Volume(dimensions) 0] [lindex $Volume(dimensions) 1]
    Volume($i,node) ComputeRasToIjkFromScanOrder $Volume(scanOrder)

    # so can read in the volume
    if {$Module(verbose) == 1} {
        puts "vtkFreeSurferReaders: setting full prefix for volume node $i"
    }
    Volume($i,node) SetFullPrefix [string trimright [file rootname $vtkFreeSurferReaders(VolumeFileName)] "-"]

    if {$Module(verbose) == 1} {
        puts "vtkFreeSurferReaders: set up volume node for $i:"
        Volume($i,node) Print
        set badval [[Volume($i,node) GetPosition] GetElement 1 3]
        puts "vtkFreeSurferReaders: volume $i position 1 3: $badval"
    
        puts "vtkFreeSurferReaders: calling MainUpdateMRML"
    }
    # Reads in the volume via the Execute procedure
    MainUpdateMRML
    # If failed, then it's no longer in the idList
    if {[lsearch $Volume(idList) $i] == -1} {
        puts "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        DevErrorWindow "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        return
    }
    if {$Module(verbose) == 1} {
        puts "vtkFreeSurferReaders: after mainupdatemrml volume node  $i:"
        Volume($i,node) Print
        set badval [[Volume($i,node) GetPosition] GetElement 1 3]
        puts "vtkFreeSurferReaders: volume $i position 1 3: $badval"
    }
 
    # set active volume on all menus
    MainVolumesSetActive $i

    # save the id for later use
    set m $i

    # if we are successful set the FOV for correct display of this volume
    set dim     [lindex [Volume($i,node) GetDimensions] 0]
    set spacing [lindex [Volume($i,node) GetSpacing] 0]
    set fov     [expr $dim*$spacing]
    set View(fov) $fov
    MainViewSetFov

    set iCast -1
    if {$vtkFreeSurferReaders(castToShort)} {
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersCORApply: Casting volume to short."
        }
        set iCast [vtkFreeSurferReadersCast $i Short]
        if {$iCast != -1} {
            puts "Cast input volume to Short, use ${Volume(name)}-Short for editing."
            if {$::Module(verbose)} {
                DevInfoWindow "Cast input volume to Short, use ${Volume(name)}-Short for editing."
            }
        }
    } 

    # load in free surfer colours for a label map?
    if {[Volume($i,node) GetLabelMap] == 1 &&
        $vtkFreeSurferReaders(loadColours) &&
        $vtkFreeSurferReaders(coloursLoaded) != 1} {
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersCORApply: loading colour file $vtkFreeSurferReaders(colourFileName) by calling vtkFreeSurferReadersLoadColour."
        }
        # the argument will delete (1, default) or append (0) to the current colour list
        vtkFreeSurferReadersLoadColour
    }


    # display the new volume in the background of all slices if not a label map
    if {$iCast == -1} {
        if {[Volume($i,node) GetLabelMap] == 1} {
            MainSlicesSetVolumeAll Label $i
        } else {
            MainSlicesSetVolumeAll Fore $i
        }
    } else {
        if {[Volume($iCast,node) GetLabelMap] == 1} {
            MainSlicesSetVolumeAll Label $iCast
        } else {
            MainSlicesSetVolumeAll Fore $iCast
        }
    }

    # Update all fields that the user changed (not stuff that would need a file reread)

    # return the volume id 
    return $i
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersMGHApply
# Read in the freesurfer MGH volume specified
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersMGHApply {} {
    global vtkFreeSurferReaders Module Volume View

    if {![info exists Volume(name)] } { set Volume(name) "MGH"}

    # Validate name
    if {$Volume(name) == ""} {
        tk_messageBox -message "Please enter a name that will allow you to distinguish this volume."
        return
    }
    if {[ValidateName $Volume(name)] == 0} {
        tk_messageBox -message "The name can consist of letters, digits, dashes, or underscores"
        return
    }

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersMGHApply:\n\tLoading MGH file $vtkFreeSurferReaders(VolumeFileName)"
    }

    # add a mrml node
    set n [MainMrmlAddNode Volume]
    set i [$n GetID]

    # read in the MGH file
    # Set up the reading
    if {[info command Volume($i,vol,rw)] != ""} {
        # have to delete it first, needs to be cleaned up
        puts "Problem: reader for this new volume number $i already exists, deleting it"
        Volume($i,vol,rw) Delete
    }
    vtkMGHReader Volume($i,vol,rw)
    
    Volume($i,vol,rw) SetFilePrefix [file dirname $vtkFreeSurferReaders(VolumeFileName)]
    # set the filename
    Volume($i,vol,rw) SetFileName $vtkFreeSurferReaders(VolumeFileName)

#----------------
set usePos 0
set useMatrices 0

    # have to fudge it a little here, read the header to get the info needed
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersMGHApply:\n\tReading volume header"
    }

    # add some progress reporting
    Volume($i,vol,rw) AddObserver StartEvent MainStartProgress
    Volume($i,vol,rw) AddObserver ProgressEvent "MainShowProgress Volume($i,vol,rw)"
    Volume($i,vol,rw) AddObserver EndEvent       MainEndProgress
    set ::Gui(progressText) "Reading [file tail $vtkFreeSurferReaders(VolumeFileName)] header"


    Volume($i,vol,rw) ReadVolumeHeader

    set updateReturn [Volume($i,vol,rw) Update]
    if {$updateReturn == 0} {
        DevErrorWindow "vtkMGHReader: update on volume $i failed."
    }

    # set the name and description of the volume
    $n SetName $Volume(name)
    $n SetDescription $Volume(desc)

   
    set Volume(isDICOM) 0
    set Volume($i,type) "MGH"


    # Set up the Volume(x) variables

    set dims [Volume($i,vol,rw) GetDataDimensions]
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) dimensions: [lindex $dims 0] [lindex $dims 1]  [lindex $dims 2]"
        # DevErrorWindow "$vtkFreeSurferReaders(VolumeFileName) dimensions: [lindex $dims 0] [lindex $dims 1]  [lindex $dims 2]"
    }
    set Volume(lastNum) [expr [lindex $dims 2] - 1]
    set Volume(width) [expr [lindex $dims 0] - 1]
    set Volume(height) [expr [lindex $dims 1] - 1]

    set spc [Volume($i,vol,rw) GetDataSpacing]
    set Volume(pixelWidth) [lindex $spc 0]
    set Volume(pixelHeight) [lindex $spc 1]
    set Volume(sliceThickness) [lindex $spc 2]
    # use a slice spacing of 0 since we don't want it doubled in the fov calc
    set Volume(sliceSpacing) 0
# [lindex $spc 2]

    set Volume(gantryDetectorTilt) 0
    set Volume(numScalars) 1
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) numScalars = $Volume(numScalars)"
    }
    set Volume(littleEndian) 0

    # Sag:LR RL Ax:SI IS Cor: AP PA
    # set Volume(scanOrder) {RL}
    set Volume(scanOrder) {PA}

    set scalarType [Volume($i,vol,rw) GetScalarType]
    # Scalar type can be VTK_UNSIGNED_CHAR (3),  VTK_INT (6), VTK_FLOAT (10), VTK_SHORT (4), 
    # set it to the valid volume values of $Volume(scalarTypeMenu)
    switch $scalarType {
        "3" { set Volume(scalarType) UnsignedChar}
        "4" { set  Volume(scalarType) Short}
        "6" { set  Volume(scalarType) Int}
        "10" { set  Volume(scalarType) Float }
        default {
            puts "Unknown scalarType $scalarType, using Float"
            DevErrorWindow "vtkFreeSurferReadersMGHApply: Unknown scalarType $scalarType, using Float"
            set Volume(scalarType) Float 
        }
    }
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) scalarType $Volume(scalarType)"
    }

    set Volume(readHeaders) 0
    set Volume(filePattern) %s
    set Volume(dimensions) "[lindex $dims 0] [lindex $dims 1]"

    set Volume(imageRange) "1 [lindex $dims 2]"
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) imageRange $Volume(imageRange)"
    }




    # Set the volume node properties
    Volume($i,node) SetName $Volume(name)
    Volume($i,node) SetDescription $Volume(desc)
    Volume($i,node) SetLabelMap $Volume(labelMap) 
    eval Volume($i,node) SetSpacing $Volume(pixelWidth) $Volume(pixelHeight) \
            [expr $Volume(sliceSpacing) + $Volume(sliceThickness)]
    Volume($i,node) SetTilt $Volume(gantryDetectorTilt)

    Volume($i,node) SetFilePattern $Volume(filePattern) 
    Volume($i,node) SetScanOrder $Volume(scanOrder)
    Volume($i,node) SetNumScalars $Volume(numScalars)
    Volume($i,node) SetLittleEndian $Volume(littleEndian)
    Volume($i,node) SetFilePrefix [Volume($i,vol,rw) GetFileName] ;# NB: just one file, not a pattern
    Volume($i,node) SetFullPrefix [Volume($i,vol,rw) GetFileName] ;# needed in the range check
    Volume($i,node) SetImageRange [lindex $Volume(imageRange) 0] [lindex $Volume(imageRange) 1]
    Volume($i,node) SetScalarType $scalarType
    Volume($i,node) SetDimensions [lindex $Volume(dimensions) 0] [lindex $Volume(dimensions) 1]
    # without these, getting a seg fault when debug is turned on in the vtkMrmlVolumeNode
    Volume($i,node) SetLUTName ""
    Volume($i,node) SetFileType  $Volume($i,type)


    # now compute the RAS to IJK matrix from the values set above
    # Volume($i,node) ComputeRasToIjkFromScanOrder $Volume(scanOrder)
    # if that doesn't work, calculate corners
    # it doesn't as this isn't a standard scan order

    # read in the matrix from the headers and use it
    
    # get the IJK to RAS matrix from the volume:
    # x_r x_a x_s y_r y_a y_s z_r z_a z_s c_r c_a c_s
    set ijkmat [Volume($i,vol,rw) GetRASMatrix]
    if {$::Module(verbose)} {
        puts "MGH Reader: IJK matrix for volume $i: $ijkmat"
    }

    # calcualte tr, ta, ts from cr, ca, cs - from solving this equation for t_ras
    # xr yr zr tr    xspacing    0        0     0      width/2     cr
    # xa ya za ta  *   0      yspacing    0     0   *  height/2  = ca
    # xs ys zs ts      0         0     zspacing 0      depth/2     cs
    # 0  0  0  1       0         0        0     1         1        1
    # which yeilds
    # xr*xspacing*width/2 + yr*yspacing*height/2 + zr*zspacing*depth/2 + tr = cr
    # and similarly for xa... xs...
    set xr [lindex $ijkmat 0]
    set xa [lindex $ijkmat 1]
    set xs [lindex $ijkmat 2]
    set yr [lindex $ijkmat 3]
    set ya [lindex $ijkmat 4]
    set ys [lindex $ijkmat 5]
    set zr [lindex $ijkmat 6]
    set za [lindex $ijkmat 7]
    set zs [lindex $ijkmat 8]
    set cr [lindex $ijkmat 9]
    set ca [lindex $ijkmat 10]
    set cs [lindex $ijkmat 11]
    set xspacing [lindex $spc 0]
    set yspacing [lindex $spc 1]
    set zspacing [lindex $spc 2]
    set w2 [expr [lindex $dims 0] / 2.0]
    set h2 [expr [lindex $dims 1] / 2.0]
    set d2 [expr [lindex $dims 2] / 2.0]
        
    # try something - zero out the cras to take out the mgh shift away from origin
    set cr 0
    set ca 0
    set cs 0

    set tr [expr $cr - $xr*$xspacing*$w2 - $yr*$yspacing*$h2 - $zr*$zspacing*$d2]
    set ta [expr $ca - $xa*$xspacing*$w2 - $ya*$yspacing*$h2 - $za*$zspacing*$d2]
    set ts [expr $cs - $xs*$xspacing*$w2 - $ys*$yspacing*$h2 - $zs*$zspacing*$d2]

    if {$::Module(verbose)} {
        puts "MGH: tr = $tr, ta = $ta, ts = $ts"
    }

    # there's a problem with getting the MGH volume to display properly,
    # it comes up okay in slices mode but not when the RasToIjk matrix is set
    # in the volume (ijkmat) they have:
    # x_r x_a x_s
    # y_r y_a y_s
    # z_r z_a z_s
    # c_r c_a c_s
    # which, to compute the corners (ftl, ftr, fbr, ltl: first slice top left and right corners, first slice bottom right corner, and last slice top left corner) to pass to 
    # ComputeRasToIjkFromCorners, we need to apply the transform
    # xr yr zr tr
    # xa ya za ta
    # xs ys zs ts
    # 0  0  0  1
    # to the corners of the volume

    catch "rasmat$i Delete"
    vtkMatrix4x4 rasmat$i
    rasmat$i Identity

    # by rows:
    # x_r
    rasmat$i SetElement 0 0 [lindex $ijkmat 0]
    # y_r
    rasmat$i SetElement 0 1 [lindex $ijkmat 3]
    # z_r
    rasmat$i SetElement 0 2 [lindex $ijkmat 6]
    # t_r 
    rasmat$i SetElement 0 3 $tr

    # x_a
    rasmat$i SetElement 1 0 [lindex $ijkmat 1]
    # y_a
    rasmat$i SetElement 1 1 [lindex $ijkmat 4]
    # z_a
    rasmat$i SetElement 1 2 [lindex $ijkmat 7]
    # t_a
    rasmat$i SetElement 1 3 $ta

    # x_s
    rasmat$i SetElement 2 0 [lindex $ijkmat 2]
    # y_s
    rasmat$i SetElement 2 1 [lindex $ijkmat 5]
    # z_s
    rasmat$i SetElement 2 2 [lindex $ijkmat 8]
    # t_s
    rasmat$i SetElement 2 3 $ts

    # now include the scaling factor, from the voxel size
    catch "scalemat$i Delete"
    vtkMatrix4x4 scalemat$i
    scalemat$i Identity
    # s_x
    scalemat$i SetElement 0 0 $Volume(pixelWidth)
    # s_y
    scalemat$i SetElement 1 1 $Volume(pixelHeight)
    # s_z
    scalemat$i SetElement 2 2 $Volume(sliceThickness)
    
    # now apply it to the rasmat
    rasmat$i Multiply4x4 rasmat$i scalemat$i rasmat$i
    
    if {$::Module(verbose)} {
        if {[info command DevPrintMatrix4x4] != ""} {
            DevPrintMatrix4x4 rasmat$i "MGH vol $i RAS -> IJK (with scale)"
        }
    }

    # To get the corners, find the max values of the volume, assume mins are 0
    set maxx [lindex $dims 0]
    set maxy [lindex $dims 1]
    set maxz [lindex $dims 2]
    # set maxx $w2
    # set maxy $h2
    # set maxz $d2

    # first slice, top left corner = (minx,maxy,minz) 0,1,0 y axis
    set ftl [rasmat$i MultiplyPoint 0 0 0 1]

    # first slice, top right corner = (maxx,maxy,minz) 1,1,0
    set ftr [rasmat$i MultiplyPoint $maxx 0 0 1]

    # first slice, bottom right corner =(maxx,miny,minz) 1,0,0 x axis
    set fbr [rasmat$i MultiplyPoint $maxx $maxy 0 1]

    # last slice, top left corner = (minx,maxy,maxz) 0,0,1
    set ltl [rasmat$i MultiplyPoint 0 0 $maxz 1]

    # these aren't used
    set fc [rasmat$i MultiplyPoint 0 0 0 1]
    set lc [rasmat$i MultiplyPoint 0 0 0 1]

    if {$::Module(verbose)} {
        if {[info command DevPrintMatrix4x4] != ""} {
            DevPrintMatrix4x4 rasmat$i "MGH vol $i RAS -> IJK (with scaling, t_ras)"
        } 
        puts "dims $dims"
        puts "spc $spc"
        puts  "ftl $ftl"
        puts  "ftr $ftr"
        puts  "fbr $fbr"
        puts  "ltl $ltl"
    }
    # now do the magic bit
    # Volume($i,node) ComputeRasToIjkFromCorners $fc $ftl $ftr $fbr $lc $ltl
    Volume($i,node) ComputeRasToIjkFromCorners \
        [lindex $fc 0]  [lindex $fc 1]  [lindex $fc 2] \
        [lindex $ftl 0] [lindex $ftl 1] [lindex $ftl 2] \
        [lindex $ftr 0] [lindex $ftr 1] [lindex $ftr 2] \
        [lindex $fbr 0] [lindex $fbr 1] [lindex $fbr 2]  \
        [lindex $lc 0]  [lindex $lc 1]  [lindex $lc 2] \
        [lindex $ltl 0] [lindex $ltl 1] [lindex $ltl 2]


    if {0} {
        # this is taken care of by flipping the image upon reading it
        # Turn off using the ras to vtk matrix, as otherwise the MGH volume is flipped in Y
        if {$::Module(verbose)} {
            puts "Turning off UseRasToVtkMatrix on volume $i"
        }
        Volume($i,node) UseRasToVtkMatrixOff
    } 

    # when making models from these volumes, they're coming out wrong, since the RasToWld matrix
    # is still identity. Try setting it to the RasToIjk matrix, minus the position vector
    for {set mi 0} {$mi < 4} {incr mi} {
        # skip the position in last column
        for {set mj 0} { $mj < 3} {incr mj} {
            [Volume($i,node) GetRasToWld] SetElement $mi $mj [[Volume($i,node) GetRasToIjk] GetElement $mi $mj]
        }
    }
    if {$::Module(verbose)} {
        puts "Set Volume($i,node)'s RasToWld matrix from the RasToIjk matrix"
        DevPrintMatrix4x4 [Volume($i,node) GetRasToWld] "Volume $i RAS -> WLD"
    }


    # Reads in the volume via the Execute procedure
    MainUpdateMRML

    # Try doing some manual reading here - this is necessary to show the data legibly
    # doesn't seem to be necessary after using compute ras to ijk  from corners
    # Volume($i,vol) SetImageData [Volume($i,vol,rw) GetOutput]


    # if the volume type isn't short, try casting to short so can edit it
    set iCast -1
    if {$Volume(scalarType) != "Short"} {
        if {$vtkFreeSurferReaders(castToShort)} {
            if {$::Module(verbose)} {
                puts "vtkFreeSurferReadersMGHApply: Casting volume to short."
            }
            set iCast [vtkFreeSurferReadersCast $i Short]
            if {$iCast != -1} {
                DevInfoWindow "Cast input volume to Short, use [Volume($i,node) GetName]-Short for editing."
            } else {
                puts "Tried casting volume to short, returned $iCast"
            }
        }
    } else {
        if {$::Module(verbose)} {
            puts "Scalar type is short, not casting"
        }
    }

    # Clean up

    # If failed, then it's no longer in the idList
    if {[lsearch $Volume(idList) $i] == -1} {
        puts "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        DevErrorWindow "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        return
    }
     
    # allow use of other module GUIs
    set Volume(freeze) 0
    if {$Module(freezer) != ""} {
        set cmd "Tab $Module(freezer)"
        set Module(freezer) ""
        eval $cmd
    }

    # set active volume on all menus
    MainVolumesSetActive $i

    # save the id for later use
    set m $i

    # if we are successful set the FOV for correct display of this volume - tbd calc the max
    # set dim     [lindex [Volume($i,node) GetImageRange] 1]
    # set spacing [lindex [Volume($i,node) GetSpacing] 2]
    set dim     [lindex [Volume($i,node) GetDimensions] 0]
    set spacing [lindex [Volume($i,node) GetSpacing] 0]

    set fov     [expr $dim * $spacing]
    if {$::Module(verbose)} { 
        puts "MGH Reader setting fov to $fov - (dim $dim spacing $spacing)"
    }
    set View(fov) $fov
    MainViewSetFov

    # load in free surfer colours for a label map?
    if {[Volume($i,node) GetLabelMap] == 1 &&
        $vtkFreeSurferReaders(loadColours) &&
        $vtkFreeSurferReaders(coloursLoaded) != 1} {
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersMGHApply: loading colour file $vtkFreeSurferReaders(colourFileName)."
        }
        vtkFreeSurferReadersLoadColour
    }

    # display the new volume in the foreground of all slices if not a label map
    if {$iCast == -1} {
        if {[Volume($i,node) GetLabelMap] == 1} {
            MainSlicesSetVolumeAll Label $i
        } else {
            MainSlicesSetVolumeAll Fore $i
        }
    } else {
        if {[Volume($iCast,node) GetLabelMap] == 1} {
             MainSlicesSetVolumeAll Label $iCast
        } else {
            MainSlicesSetVolumeAll Fore $iCast
        }
    }

    # Update all fields that the user changed (not stuff that would need a file reread)
    return $i
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersMGHUpdateMRML
# Takes the necessary updates from MainUpdateMrml
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersUpdateMRML {} {
    global Module vtkFreeSurferReaders

    if {$Module(verbose)} { 
        puts "vtkFreeSurferReadersMGHUpdateMRML"
    }

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersShowMGH
# A debugging tool, opens a new window with slices shown of the MGH volume i
# .ARGS
# int i the volume id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersShowMGH {i} {
    global Volume Module

    if {[info command Volume($i,vol,rw)] == ""} {
        puts "vtkFreeSurferReadersShowMGH: no reader exists for volume $i"
        return
    }
    scan [[Volume($i,vol,rw) GetOutput] GetWholeExtent] "%d %d %d %d %d %d" \
        xMin xMax yMin yMax zMin zMax
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersShowMGH: vol $i: whole extent $xMin $xMax $yMin $yMax $zMin $zMax"
    }
    if {[info command viewer$i] == ""} {
        vtkImageViewer viewer$i
    }
    viewer$i SetInput [Volume($i,vol,rw) GetOutput]
    viewer$i SetZSlice [expr $zMax / 2]
    viewer$i SetColorWindow 2000
    viewer$i SetColorLevel 1000
    toplevel .top$i
    # wm protocol .top$i WM_DELETE_WINDOW {puts "exit, i=$i"; wm withdraw .top$i; destroy .top$i; viewer$i Delete}
    frame .top$i.f1 
    set vtkiw vtkiw$i
    set $vtkiw [vtkTkImageViewerWidget .top$i.f1.r1 \
                   -width [expr ($xMax - $xMin + 1)] \
                   -height [expr ($yMax - $yMin + 1)] \
                   -iv viewer$i]
    ::vtk::bind_tk_imageviewer_widget [subst "$$vtkiw"]
    set slice_number$i [viewer$i GetZSlice]
    
    
    scale .top$i.slice \
        -from $zMin \
        -to $zMax \
        -orient horizontal \
        -command "SetSlice $i"\
        -variable slice_number$i \
        -label "Z Slice"
    
    proc SetSlice {i slice} {
        if {$::Module(verbose)} {
            puts "SetSlice slice = $slice, i = $i"
        }
        viewer$i SetZSlice $slice
        viewer$i Render
    }
    pack [subst "$$vtkiw"] \
        -side left -anchor n \
        -padx 3 -pady 3 \
        -fill x -expand f
    pack .top$i.f1 \
        -fill both -expand t
    pack .top$i.slice \
        -fill x -expand f
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBApply
# Read in the freesurfer Bfloat or Bshort volume specified
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBApply {} {
    global vtkFreeSurferReaders Module Volume View

    if {![info exists Volume(name)] } { set Volume(name) "FSB"}

    set Volume(isDICOM) 0

    # Validate name
    if {$Volume(name) == ""} {
        tk_messageBox -message "Please enter a name that will allow you to distinguish this volume."
        return -1
    }
    if {[ValidateName $Volume(name)] == 0} {
        tk_messageBox -message "The name can consist of letters, digits, dashes, or underscores"
        return -1
    }

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersBApply:\n\tLoading B file $vtkFreeSurferReaders(VolumeFileName)"
    }


    # add a mrml node
    set n [MainMrmlAddNode Volume]
    set i [$n GetID]

    # Set up the reading
    if {[info command Volume($i,vol,rw)] != ""} {
        # have to delete it first, needs to be cleaned up
        puts "Problem: reader for this new volume number $i already exists, deleting it"
        Volume($i,vol,rw) Delete
    }
    vtkBVolumeReader Volume($i,vol,rw)

    if {$::Module(verbose)} {
        puts "Turning debug on for vtkBVolumeReader Volume($i,vol,rw)"
        Volume($i,vol,rw) DebugOn
    }


    # set the filename  stem
    set stem [file rootname $vtkFreeSurferReaders(VolumeFileName)]
    if {$::Module(verbose)} {
        puts "Set stem to $stem"
    }

    Volume($i,vol,rw) SetFileName $vtkFreeSurferReaders(VolumeFileName)
    Volume($i,vol,rw) SetFilePrefix $stem
    Volume($i,vol,rw) SetStem $stem


    # read the header to get the info we need
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersBApply:\n\tReading volume header"
    }
    set headerReturn [Volume($i,vol,rw) ReadVolumeHeader]
    if {$headerReturn != 1} {
        DevErrorWindow "Error reading volume header for $vtkFreeSurferReaders(VolumeFileName)"
        return
    }

    # gets all the info needed from the header, read it in MainUpdateMrml
    if {0} {
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersBApply:\n\tNOT Reading volume"
        }
        set updateReturn [Volume($i,vol,rw) Update]
        if {$updateReturn == 0} {
            DevErrorWindow "vtkFreeSurferReadersBApply: update on volume $i failed."
        }


        if {$::Module(verbose)} {
            set newstem [Volume($i,vol,rw) GetStem]
            puts "After reading, new stem = $newstem (orig stem = $stem)"
        }
    }

    # try setting the registration filename
    if {[file exist ${stem}.dat] == 1} {
        Volume($i,vol,rw) SetRegistrationFileName ${stem}.dat
        set regmat [Volume($i,vol,rw) GetRegistrationMatrix]
    } else {
        set regmat ""
    }
    if {$::Module(verbose)} {
        puts "Got registration matrix $regmat"
        if {$regmat != ""} {
            puts "[$regmat GetElement 0 0] [$regmat GetElement 1 0] [$regmat GetElement 2 0] [$regmat GetElement 3 0]"
            puts "[$regmat GetElement 0 1] [$regmat GetElement 1 1] [$regmat GetElement 2 1] [$regmat GetElement 3 1]"
            puts "[$regmat GetElement 0 2] [$regmat GetElement 1 2] [$regmat GetElement 2 2] [$regmat GetElement 3 2]"
            puts "[$regmat GetElement 0 3] [$regmat GetElement 1 3] [$regmat GetElement 2 3] [$regmat GetElement 3 3]"
        } else {
            puts "\tregmat is empty"
        }
    }

    # set the name and description of the volume
    $n SetName $Volume(name)
    $n SetDescription $Volume(desc)


    #--------------------------
    # Set the Volume variables
    #-------------------------

    set scalarType [Volume($i,vol,rw) GetScalarType]
    # Scalar type can be VTK_FLOAT (10), VTK_SHORT (4), 
    # set it to either bfloat or bshort
    switch $scalarType {
        "4" { set Volume($i,type) "bshort" }
        "10" { set Volume($i,type) "bfloat" }
        default {
            puts "Unknown scalarType $scalarType, using bfloat"
            set Volume($i,type) "bfloat"
        }
    }

    set dims [Volume($i,vol,rw) GetDataDimensions]
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) dimensions: [lindex $dims 0] [lindex $dims 1]  [lindex $dims 2]"
    }
    set Volume(lastNum) [expr [lindex $dims 2] - 1]
    set Volume(width) [expr [lindex $dims 0] - 1]
    set Volume(height) [expr [lindex $dims 1] - 1]

    set spc [Volume($i,vol,rw) GetDataSpacing]
    if {$::Module(verbose)} { 
        puts "B reader apply: got data spacing $spc"
    }
    set Volume(pixelWidth) [lindex $spc 0]
    set Volume(pixelHeight) [lindex $spc 1]
    set Volume(sliceThickness) [lindex $spc 2]
    # use a slice spacing of 0 since we don't want it doubled in the fov calc
    set Volume(sliceSpacing) 0
# [lindex $spc 2]

    set Volume(gantryDetectorTilt) 0

    # this may need to change if we've got lots of data in the one b volume
    set Volume(numScalars) 1
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) numScalars = $Volume(numScalars)"
        puts "$vtkFreeSurferReaders(VolumeFileName) numComponents = [[Volume($i,vol,rw) GetOutput] GetNumberOfScalarComponents]"
    }

    set Volume(littleEndian) 0

    # Sag:LR RL Ax:SI IS Cor: AP PA
    # this gets reset when calc the scan order from the corners
    set Volume(scanOrder) {PA}
    set scalarType [Volume($i,vol,rw) GetScalarType]
    # Scalar type can be VTK_UNSIGNED_CHAR (3),  VTK_INT (6), VTK_FLOAT (10), VTK_SHORT (4), 
    # set it to the valid volume values of $Volume(scalarTypeMenu)
    switch $scalarType {
        "3" { set Volume(scalarType) UnsignedChar}
        "4" { set  Volume(scalarType) Short}
        "6" { set  Volume(scalarType) Int}
        "10" { set  Volume(scalarType) Float }
        default {
            puts "Unknown scalarType $scalarType, using Float"
            DevErrorWindow "vtkFreeSurferReadersBApply: Unknown scalarType $scalarType, using Float"
            set Volume(scalarType) Float 
        }
    }

    set Volume(readHeaders) 0

    set Volume(filePattern) %s_%03d.$Volume($i,type)
    set Volume(dimensions) "[lindex $dims 0] [lindex $dims 1]"

    set Volume(imageRange) "0 [expr [lindex $dims 2] - 1]"
    if {$::Module(verbose)} {
        puts "$vtkFreeSurferReaders(VolumeFileName) imageRange $Volume(imageRange)"
    }


    #---------------------
    # Set up the MRML node
    #---------------------


    # set the volume properties
    Volume($i,node) SetName $Volume(name)
    Volume($i,node) SetDescription $Volume(desc)
    Volume($i,node) SetLabelMap $Volume(labelMap)
    eval Volume($i,node) SetSpacing $Volume(pixelWidth) $Volume(pixelHeight) \
            [expr $Volume(sliceSpacing) + $Volume(sliceThickness)]
    Volume($i,node) SetTilt $Volume(gantryDetectorTilt)

    Volume($i,node) SetFilePattern $Volume(filePattern) 
    Volume($i,node) SetScanOrder $Volume(scanOrder)
    Volume($i,node) SetNumScalars $Volume(numScalars)
    Volume($i,node) SetLittleEndian $Volume(littleEndian)
    Volume($i,node) SetFilePrefix [Volume($i,vol,rw) GetStem] 
    Volume($i,node) SetFullPrefix [Volume($i,vol,rw) GetStem] ;# needed in the range check
    Volume($i,node) SetImageRange [lindex $Volume(imageRange) 0] [lindex $Volume(imageRange) 1]
    Volume($i,node) SetScalarType $scalarType
    Volume($i,node) SetDimensions [lindex $Volume(dimensions) 0] [lindex $Volume(dimensions) 1]
    # without these, getting a seg fault when debug is turned on in the vtkMrmlVolumeNode
    Volume($i,node) SetLUTName ""
    Volume($i,node) SetFileType $Volume($i,type) 


    #----------------------------------
    # now compute the RAS to IJK matrix
    #----------------------------------
    # get the IJK to RAS matrix from the volume:
    # x_r x_a x_s y_r y_a y_s z_r z_a z_s c_r c_a c_s
    set ijkmat [Volume($i,vol,rw) GetRASMatrix]
    # get the corners from the volume:
    set topR [Volume($i,vol,rw) GetTopR]
    set topL [Volume($i,vol,rw) GetTopL]
    set bottomR [Volume($i,vol,rw) GetBottomR]
    set normal [Volume($i,vol,rw) GetNormal]

    set nonzero 0
    foreach n $normal { if {$n != 0} { incr nonzero}}
    if {$nonzero != 1} {
        DevWarningWindow "Volume $i normal is skewed:\n $normal \nVolume may not display properly."
    }

    # Calculate the last top left
    set v1 [list [expr [lindex $topR 0] - [lindex $topL 0]] [expr [lindex $topR 1] - [lindex $topL 1]] [expr [lindex $topR 2] - [lindex $topL 2]]]
    set v2 [list [expr [lindex $bottomR 0] - [lindex $topR 0]] [expr [lindex $bottomR 1] - [lindex $topR 1]] [expr [lindex $bottomR 2] - [lindex $topR 2]]]
    package require tclVectorUtils
    set v3 [tclVectorUtils::VCross $v2 $v1]
    set v3 [tclVectorUtils::VNorm $v3]
    set v4 [tclVectorUtils::VScale [expr  [lindex $dims 2] *  [lindex $spc 2]] $v3 ]
    set lTopL [tclVectorUtils::VAdd $topL $v4]
    if {$::Module(verbose)} {
        puts "Bfloat Reader: lTopL $lTopL"
    }

    
    # see the comments in MGH apply proc 
    set xr [lindex $ijkmat 0]
    set xa [lindex $ijkmat 1]
    set xs [lindex $ijkmat 2]
    set yr [lindex $ijkmat 3]
    set ya [lindex $ijkmat 4]
    set ys [lindex $ijkmat 5]
    set zr [lindex $ijkmat 6]
    set za [lindex $ijkmat 7]
    set zs [lindex $ijkmat 8]
    set cr [lindex $ijkmat 9]
    set ca [lindex $ijkmat 10]
    set cs [lindex $ijkmat 11]
    set xspacing [lindex $spc 0]
    set yspacing [lindex $spc 1]
    set zspacing [lindex $spc 2]
    set w2 [expr [lindex $dims 0] / 2.0]
    set h2 [expr [lindex $dims 1] / 2.0]
    set d2 [expr [lindex $dims 2] / 2.0]
        
    # try something - zero out the cras to take out the shift away from origin
    set cr 0
    set ca 0
    set cs 0

    if {0} {
        # calculate cras from the vectors to the corner points
    # this is not used, as don't use tras in the rasmat
    # calculate the centre point of the volume, cras, as it's actually zero in the matrix since the bvolume reader doesn't calculate it
    # the vector pointing from the origin to the centre of the first slice is bottomR + (topL - bottomR)/2 = A = cf
    # the vector pointing from the top left corner to the centre of the depth is (last top left - front topL)/2 = B = cd
    # so to get to the centre of the volume take A + B
    set cf [tclVectorUtils::VSub $topL $bottomR]
    set cf [tclVectorUtils::VScale 0.5 $cf]
    set cf [tclVectorUtils::VAdd $bottomR $cf]

    set cd [tclVectorUtils::VSub $lTopL $topL]
    set cd [tclVectorUtils::VScale 0.5 $cd]

    set cras [tclVectorUtils::VAdd $cf $cd]
    set cr [lindex $cras 0]
    set ca [lindex $cras 1]
    set cs [lindex $cras 2]
    if {$::Module(verbose)} {
        puts "New Centre Calc:"
        puts "cf $cf"
        puts "cd $cd"
        puts "cras $cras"
    }
}
    # calculate centre points from volume size
    if {0} {
    set cr $w2
    set ca $h2
    set cs $d2
    }
    # Calculate the translation
    set tr [expr $cr - $xr*$xspacing*$w2 - $yr*$yspacing*$h2 - $zr*$zspacing*$d2]
    set ta [expr $ca - $xa*$xspacing*$w2 - $ya*$yspacing*$h2 - $za*$zspacing*$d2]
    set ts [expr $cs - $xs*$xspacing*$w2 - $ys*$yspacing*$h2 - $zs*$zspacing*$d2]

    if {$::Module(verbose)} {
        puts "BReader: tr = $tr, ta = $ta, ts = $ts"
    }

    # calculate the transform from the corners of the volume
    catch "rasmat$i Delete"
    vtkMatrix4x4 rasmat$i
    rasmat$i Identity

    # by rows:
    # x_r
    rasmat$i SetElement 0 0 [lindex $ijkmat 0]
    # y_r
    rasmat$i SetElement 0 1 [lindex $ijkmat 3]
    # z_r
    rasmat$i SetElement 0 2 [lindex $ijkmat 6]
    # t_r 
    rasmat$i SetElement 0 3  $tr

    # x_a
    rasmat$i SetElement 1 0 [lindex $ijkmat 1]
    # y_a
    rasmat$i SetElement 1 1 [lindex $ijkmat 4]
    # z_a
    rasmat$i SetElement 1 2 [lindex $ijkmat 7]
    # t_a
    rasmat$i SetElement 1 3 $ta

    # x_s
    rasmat$i SetElement 2 0 [lindex $ijkmat 2]
    # y_s
    rasmat$i SetElement 2 1 [lindex $ijkmat 5]
    # z_s
    rasmat$i SetElement 2 2 [lindex $ijkmat 8]
    # t_s
    rasmat$i SetElement 2 3 $ts

    # the scaling factor is included in the RAS Matrix in the B volume reader
    if {1} {
        # now include the scaling factor, from the voxel size
        catch "scalemat$i Delete"
        vtkMatrix4x4 scalemat$i
        scalemat$i Identity
        # s_x
        scalemat$i SetElement 0 0 $Volume(pixelWidth)
        # s_y
        scalemat$i SetElement 1 1 $Volume(pixelHeight)
        # s_z
        scalemat$i SetElement 2 2 $Volume(sliceThickness)
        
        # now apply it to the rasmat
        rasmat$i Multiply4x4 rasmat$i scalemat$i rasmat$i

        if {$::Module(verbose)} {
            if {[info command DevPrintMatrix4x4] != ""} {
                DevPrintMatrix4x4 rasmat$i "Bfloat vol $i RAS -> IJK (with scale)"
            }
        }
    }

    if {0} {
    # To get the corners, the B volume is centered about zero, so use the (number of rows, number of cols, number of slices) / 2 +/- the origin at 0,0,0.
    set maxx [expr [lindex $dims 0] / 2.0]
    set maxy [expr [lindex $dims 1] / 2.0]
    set maxz [expr [lindex $dims 2] / 2.0]
    set minx [expr 0.0 - $maxx]
    set miny [expr 0.0 - $maxy]
    set minz [expr 0.0 - $maxz]
} else {
    # To get the corners, the B volume is cornered at zero, so use the (number of rows, number of cols, number of slices), and 0,0,0 as the corners
    set maxx [lindex $dims 0]
    set maxy [lindex $dims 1]
    set maxz [lindex $dims 2] 
    set minx 0.0
    set miny 0.0
    set minz 0.0
}
    
    if {$::Module(verbose)} {
        puts "Calculating min/max on a b volume:"
        puts "Dimensions: $dims"
        puts "maxx $maxx"
        puts "maxy $maxy"
        puts "maxz $maxz"
        puts "minx $minx"
        puts "miny $miny"
        puts "minz $minz"
        puts "maxx - minx = [expr $maxx - $minx]"
        puts "maxy - miny = [expr $maxy - $miny]"
        puts "maxz - minz = [expr $maxz - $minz]"
    }

if {0} {
    # first slice, top left corner = (minx,maxy,minz) 0,1,0 y axis
    set ftl [rasmat$i MultiplyPoint 0 0 0 1]

    # first slice, top right corner = (maxx,maxy,minz) 1,1,0
    set ftr [rasmat$i MultiplyPoint $maxx 0 0 1]

    # first slice, bottom right corner =(maxx,miny,minz) 1,0,0 x axis
    set fbr [rasmat$i MultiplyPoint $maxx $maxy 0 1]

    # last slice, top left corner = (minx,miny,maxz) 0,0,1
    puts "B reader: maxz = $maxz, normal = $normal"
    set ltl [rasmat$i MultiplyPoint 0 0 $maxz 1]
    puts "rasmat$i times '0 0 maxz 1' = $ltl"
    # set ltl [rasmat$i MultiplyPoint [lindex $normal 0] [lindex $normal 1] [lindex $normal 2] 1]
    # puts "rasmat$i times normal = $ltl"

} else {
    # using bvolume values: first = minz, last = maxz, top = maxy, bottom = miny, right = maxx, left = minx
    # first slice, top left corner 
    set ftl [rasmat$i MultiplyPoint $minx $maxy $minz 1]

    # first slice, top right corner  
    set ftr [rasmat$i MultiplyPoint $maxx $maxy $minz 1]

    # first slice, bottom right corner
    set fbr [rasmat$i MultiplyPoint $maxx $miny $minz 1]

    # last slice, top left corner
    set ltl [rasmat$i MultiplyPoint $minx $maxy $maxz 1]
}
    # 
    # All I need is the top left corner of the last slice, the others are in the header file
    #


    # these aren't used
    set fc [rasmat$i MultiplyPoint 0 0 0 1]
    set lc [rasmat$i MultiplyPoint 0 0 0 1]

    if {$::Module(verbose)} {
        if {[info command DevPrintMatrix4x4] != ""} {
            DevPrintMatrix4x4 rasmat$i "Bfloat vol $i RAS -> IJK (with scaling, t_ras)"
        } 
        puts "dims $dims"
        puts "spc $spc"
        puts  "ftl $ftl"
        puts  "ftr $ftr"
        puts  "fbr $fbr"
        puts  "ltl $ltl"
    }
    # now do the magic bit
if {1} {
    # Volume($i,node) ComputeRasToIjkFromCorners $fc $ftl $ftr $fbr $lc $ltl
    Volume($i,node) ComputeRasToIjkFromCorners \
        [lindex $fc 0]  [lindex $fc 1]  [lindex $fc 2] \
        [lindex $ftl 0] [lindex $ftl 1] [lindex $ftl 2] \
        [lindex $ftr 0] [lindex $ftr 1] [lindex $ftr 2] \
        [lindex $fbr 0] [lindex $fbr 1] [lindex $fbr 2]  \
        [lindex $lc 0]  [lindex $lc 1]  [lindex $lc 2] \
        [lindex $ltl 0] [lindex $ltl 1] [lindex $ltl 2]

} else {
    # TODO: Calculate the last slices's top left corner, same RA as the front top left, 
    # S caculated from the number of slices times the slice spacing
    # set ltl [list [lindex $topL 0] [lindex $topL 1] [expr [lindex $dims 2] *  [lindex $spc 2]]]
    # set ltl [list 0 0 0]
    
    # redoing ltl: || topR - topL || x || bottomR - topR || scaled by  num slices times spacing
    set v1 [list [expr [lindex $topR 0] - [lindex $topL 0]] [expr [lindex $topR 1] - [lindex $topL 1]] [expr [lindex $topR 2] - [lindex $topL 2]]]
    set v2 [list [expr [lindex $bottomR 0] - [lindex $topR 0]] [expr [lindex $bottomR 1] - [lindex $topR 1]] [expr [lindex $bottomR 2] - [lindex $topR 2]]]
    package require tclVectorUtils
    set v3 [tclVectorUtils::VCross $v2 $v1]
    set v3 [tclVectorUtils::VNorm $v3]
# use normal from the volume
    set v3 $normal
    set v4 [tclVectorUtils::VScale [expr [lindex $dims 2] *  [lindex $spc 2]] $v3 ]
    set ltl [tclVectorUtils::VAdd $topL $v4]
    if {$::Module(verbose)} {
        puts "About to call ComputeRasToIjkFromCorners:"
        puts "v1 $v1"
        puts "v2 $v2"
        puts "v3 $v3; normal = $normal"
        puts "v4 $v4"
        puts "ltl $ltl"
    }

    Volume($i,node) ComputeRasToIjkFromCorners \
        0 0 0 \
        [lindex $topL 0] [expr -1 * [lindex $topL 1]] [lindex $topL 2] \
        [lindex $topR 0] [expr -1 * [lindex $topR 1]] [lindex $topR 2] \
        [lindex $bottomR 0] [expr -1 * [lindex $bottomR 1]] [lindex $bottomR 2]  \
        0 0 0 \
        [lindex $ltl 0] [expr -1 * [lindex $ltl 1]] [lindex $ltl 2]
}
    # Turn off using the ras to vtk matrix, as otherwise the volume is flipped in Y
    if {$::Module(verbose)} {
        puts "Turning off UseRasToVtkMatrix on volume $i"
    }
    Volume($i,node) UseRasToVtkMatrixOff

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReaders: About to call main update mrml for a B  volume, $i"
        puts "\tFile prefix = [Volume($i,node) GetFilePrefix]"
        puts "\tFull prefix = [Volume($i,node) GetFullPrefix]"
        puts "\tFile pattern = [Volume($i,node) GetFilePattern]"
    }

if {$::Module(verbose)} { 
    set c 0
    puts "Right before MainUpdateMRML: vol $i number of components: "
    set o [Volume($i,vol,rw) GetOutput]
    if {$o != ""} { 
        set pd [$o GetPointData]
        if {$pd != ""} {
            set s [$pd GetScalars]
            if {$s != ""} {
                set c [$s GetNumberOfComponents]
            }
        }
    }
    puts "$c"
}
                

    # Reads in the volume via the Execute procedure
    MainUpdateMRML

puts "Right after MainUpdateMRML, number of components in Volume($i,vol)'s point scalars is [[[[Volume($i,vol) GetOutput] GetPointData] GetScalars ] GetNumberOfComponents]"


    # If failed, then it's no longer in the idList
    if {[lsearch $Volume(idList) $i] == -1} {
        puts "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        DevErrorWindow "vtkFreeSurferReaders: failed to read in the volume $i, $vtkFreeSurferReaders(VolumeFileName)"
        return
    }

    # try doing some manual reading here - necessary to show the data legibly
    if {0} {
        if {$::Module(verbose)} {
            puts "BfloatReaderApply: calling SetImageData"
        }
        Volume($i,vol) SetImageData [Volume($i,vol,rw) GetOutput]
    }

    # mark the volume as saved
    set Volume($i,dirty) 0

    # allow use of other module GUIs
    set Volume(freeze) 0
    if {$Module(freezer) != ""} {
        set cmd "Tab $Module(freezer)"
        set Module(freezer) ""
        eval $cmd
    }

    # set active volume on all menus
    MainVolumesSetActive $i

    # save the id for later use
    set m $i

    # if we are successful set the FOV for correct display of this volume
    set dim     [lindex [Volume($i,node) GetDimensions] 0]
    set spacing [lindex [Volume($i,node) GetSpacing] 0]
    set fov     [expr $dim * $spacing]

if {$fov < 2000} {
    if {$::Module(verbose)} { 
        puts "Bfloat Reader setting fov to $fov (dim $dim spacing $spacing)"
    }
    set View(fov) $fov
    MainViewSetFov
} else {
    puts "Bfloat reader: warning, not resetting fov, too large: $fov"
}
    # display the new volume in the foreground of all slices if not a label map
    if {[Volume($i,node) GetLabelMap] == 1} {
        MainSlicesSetVolumeAll Label $i
    } else {
        MainSlicesSetVolumeAll Fore $i
    }

    # Update all fields that the user changed (not stuff that would need a file reread)
    return $i
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildSurface
# Builds a model, a surface from a Freesurfer file.
# .ARGS
# int m the model id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildSurface {m} {
    global vtkFreeSurferReaders viewRen Volume Module Model RemovedModels 

    if {$Module(verbose) == 1} { 
        puts "\nvtkFreeSurferReadersBuildSurface\n"
    }

    # set up the reader
    catch "Model($m,reader) Delete"
    vtkFSSurfaceReader Model($m,reader)

    Model($m,reader) SetFileName $vtkFreeSurferReaders(ModelFileName)

    Model($m,reader) AddObserver StartEvent MainStartProgress
    Model($m,reader) AddObserver ProgressEvent "MainShowProgress Model($m,reader)"
    Model($m,reader) AddObserver EndEvent       MainEndProgress
    set ::Gui(progressText) "Reading [file tail $vtkFreeSurferReaders(ModelFileName)]"


    vtkPolyDataNormals Model($m,normals)
    Model($m,normals) SetSplitting 0
    Model($m,normals) SetInput [Model($m,reader) GetOutput]

    vtkStripper Model($m,stripper)
    Model($m,stripper) SetInput [Model($m,normals) GetOutput]

    # should be vtk model
    foreach r $Module(Renderers) {
        # Mapper
        if {$::Module(verbose)} { puts "Deleting Model($m,mapper,$r)" }
        catch "Model($m,mapper,$r) Delete"
        vtkPolyDataMapper Model($m,mapper,$r)
#Model($m,mapper,$r) SetInput [Model($m,reader) GetOutput]
    }

    # Delete the src, leaving the data in Model($m,polyData)
    # polyData will survive as long as it's the input to the mapper
    #
    Model($m,node) SetName $vtkFreeSurferReaders(ModelName)
    Model($m,node) SetFileName $vtkFreeSurferReaders(ModelFileName)
    set Model($m,polyData) [Model($m,stripper) GetOutput]

    if {$::Module(verbose)} { puts "Setting observer on polyData" }
    $Model($m,polyData) AddObserver StartEvent MainStartProgress
    $Model($m,polyData) AddObserver ProgressEvent "MainShowProgress Model($m,polyData)"
    $Model($m,polyData) AddObserver EndEvent       MainEndProgress

    $Model($m,polyData) Update

    #-------------------------
    # read in the scalar files
    vtkFreeSurferReadersReadScalars $m

    foreach r $Module(Renderers) {
        Model($m,mapper,$r) SetInput $Model($m,polyData)
   
        if {0} {
            # decimate
            vtkDecimate Model($m,decimate,$r) 
            Model($m,decimate,$r) SetInput $Model($m,polyData)
            Model($m,decimate,$r) SetMaximumIterations 6
            Model($m,decimate,$r)  SetMaximumSubIterations 0 
            Model($m,decimate,$r) PreserveEdgesOn
            Model($m,decimate,$r) SetMaximumError 1
            Model($m,decimate,$r) SetTargetReduction 1
            Model($m,decimate,$r) SetInitialError .0002
            Model($m,decimate,$r) SetErrorIncrement .0002
            [ Model($m,decimate,$r) GetOutput] ReleaseDataFlagOn
            vtkSmoothPolyDataFilter smoother
            smoother SetInput [Model($m,decimate,$r) GetOutput]
            set p smoother
            $p SetNumberOfIterations 2
            $p SetRelaxationFactor 0.33
            $p SetFeatureAngle 60
            $p FeatureEdgeSmoothingOff
            $p BoundarySmoothingOff
            $p SetConvergence 0
            [$p GetOutput] ReleaseDataFlagOn

            set Model($m,polyData) [$p GetOutput]
            Model($m,polyData) Update
            foreach r $Module(Renderers) {
                Model($m,mapper,$r) SetInput $Model($m,polyData)
            }
        }
    }
    Model($m,reader) SetOutput ""
    Model($m,reader) Delete
    Model($m,normals) SetOutput ""
    Model($m,normals) Delete
    Model($m,stripper) SetOutput ""
    Model($m,stripper) Delete

    # Clipper
    vtkClipPolyData Model($m,clipper)
    Model($m,clipper) SetClipFunction Slice(clipPlanes)
    Model($m,clipper) SetValue 0.0
    
    vtkMatrix4x4 Model($m,rasToWld)
    
    foreach r $Module(Renderers) {

        # Actor
        vtkActor Model($m,actor,$r)
        Model($m,actor,$r) SetMapper Model($m,mapper,$r)
        # Registration
        Model($m,actor,$r) SetUserMatrix [Model($m,node) GetRasToWld]

        # Property
        set Model($m,prop,$r)  [Model($m,actor,$r) GetProperty]

        # For now, the back face color is the same as the front
        Model($m,actor,$r) SetBackfaceProperty $Model($m,prop,$r)
    }
    set Model($m,clipped) 0
    set Model($m,displayScalarBar) 0

    # init gui vars
    set Model($m,visibility)       [Model($m,node) GetVisibility]
    set Model($m,opacity)          [Model($m,node) GetOpacity]
    set Model($m,scalarVisibility) [Model($m,node) GetScalarVisibility]
    set Model($m,backfaceCulling)  [Model($m,node) GetBackfaceCulling]
    set Model($m,clipping)         [Model($m,node) GetClipping]
    # set expansion to 1 if variable doesn't exist
    if {[info exists Model($m,expansion)] == 0} {
        set Model($m,expansion)    1
    }
    # set RemovedModels to 0 if variable doesn't exist
    if {[info exists RemovedModels($m)] == 0} {
        set RemovedModels($m) 0
    }


    # figure out the colour from the model file name
    set colourName ""

    if {[file rootname [file tail $vtkFreeSurferReaders(ModelFileName)]] == "rh"} {
        set colourName "Right-"
    } elseif {[file rootname [file tail $vtkFreeSurferReaders(ModelFileName)]] == "lh"} {
        set colourName "Left-"
    }
    
    if {[file extension [file tail $vtkFreeSurferReaders(ModelFileName)]] == ".pial"} {
        append colourName "Cerebral-Cortex"
    } elseif {[file extension [file tail $vtkFreeSurferReaders(ModelFileName)]] == ".white" || 
        [file extension [file tail $vtkFreeSurferReaders(ModelFileName)]] == ".smoothwm"} {
        append colourName "Cerebral-White-Matter"
    } elseif {[file extension [file tail $vtkFreeSurferReaders(ModelFileName)]] == ".sphere" || 
        [file extension [file tail $vtkFreeSurferReaders(ModelFileName)]] == ".inflated"} {
        append colourName "Cerebral-Exterior"
    } else {
        append colourName "undetermined"
    }
    if {$::Module(verbose)} {
        puts "Using colour name $colourName for $vtkFreeSurferReaders(ModelFileName)"
    }
    MainModelsSetColor $m $colourName


    MainAddModelActor $m
    set Model($m,dirty) 1
    set Model($m,fly) 1

    MainModelsSetClipping $m $Model(clipping)
    MainModelsSetVisibility $m $Model(visibility)
    MainModelsSetOpacity $m $Model(opacity)
    MainModelsSetCulling $m $Model(culling)
    MainModelsSetScalarVisibility $m $Model(scalarVisibility)
    MainModelsSetScalarRange $m $Model(scalarLo) $Model(scalarHi)
    # MainModelsSetColor $m $Label(name)

    MainUpdateMRML

    MainModelsSetActive $m
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetSurfaceVisibility
# Set the model's visibility flag.
# .ARGS
# int i the model id
# boolean vis the boolean flag determining this model's visibility
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetSurfaceVisibility {i vis} {
    global vtkFreeSurferReaders Volume
    # set the visibility of volume i to visibility vis
    vtkFreeSurferReaders($i,curveactor) SetVisibility $vis
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetModelScalar
# Set the model's active scalars. If an invalid scalar name is passed, will pop up an info box.
# .ARGS
# int modelID the model id
# string scalarName the name given to the scalar field
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetModelScalar {modelID {scalarName ""}} {
    global Volume vtkFreeSurferReaders
    if {$scalarName == ""} {
        foreach s "$vtkFreeSurferReaders(scalars) $vtkFreeSurferReaders(annots)" {
            if {$vtkFreeSurferReaders(current$s)} {
                set scalarName $s
                puts "Displaying $s"
            }
        }
    }
    if {$::Module(verbose)} {
        puts "set model scalar, model $modelID -> $scalarName"
    }
    if {$scalarName != ""} {
        set retval [[$::Model(${modelID},polyData) GetPointData] SetActiveScalars $scalarName]
        if {$retval == -1} {
            DevInfoWindow "Model $modelID does not have a scalar field named $scalarName.\nPossible options are: $::vtkFreeSurferReaders(scalars)"
        } else {
            # reset the scalar range here
            Render3D
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetModelVector
# Set the model's active vector. If an invalid vector name is passed, will pop up an info box.
# .ARGS
# int modelID the model id
# string vectorName the name given to the vector field
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetModelVector {modelID {vectorName ""}} {
    if {$vectorName == ""} {
        set vectorName $vtkFreeSurferReaders(currentSurface)
    }
    if {$::Module(verbose)} {
        puts "set model vector, model $modelID -> $vectorName"
    }
    set retval [[$::Model(${modelID},polyData) GetPointData] SetActiveVectors $vectorName]
    if {$retval == -1} {
        DevInfoWindow "Model $modelID does not have a vector field named $vectorName.\nPossible options are: orig $::vtkFreeSurferReaders(surfaces)"
    } else {
        Render3D
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersMainFileCloseUpdate
# Called to clean up anything created in this sub module. Deletes Volumes read in, 
# along with their actors. Deletes matrices, and sets the colours loaded flag to 0
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersMainFileCloseUpdate {} {
    global vtkFreeSurferReaders Volume viewRen Module

    # delete stuff that's been created in this module
    if {$Module(verbose) == 1} {
        puts "vtkFreeSurferReadersMainFileCloseUpdate"
    }
    
    # no id's in the volume idlist. So, find the stuff we made:
    foreach ca [info command vtkFreeSurferReaders(*,curveactor)] {
        if {$::Module(verbose) == 1} {
            puts "Removing surface actor for free surfer $ca"
        }
        viewRen RemoveActor  $ca
        $ca Delete
    }
    foreach ma [info command vtkFreeSurferReaders(*,mapper)] {
        if {$::Module(verbose)} {
            puts "Deleting $ma"
        }
        $ma Delete
    }
    foreach rw [info command Volume(*,vol,rw)] {
        if {$::Module(verbose)} {
            puts "Deleting $rw"
        }
        $rw Delete
    }



    # Delete RAS matrices from MGH files
    set rasmats [info commands rasmat*]
    foreach rasmat $rasmats {
        if {$::Module(verbose)} {
            puts "Deleting ras matrix $rasmat"
        }
        $rasmat Delete
    }

    # let myself know that the FreeSurfer colours were unloaded
    if {$::Module(verbose)} {
        puts "setting coloursLoaded to 0"
    }
    set ::vtkFreeSurferReaders(coloursLoaded) 0

    # delete any scalar label arrays
    foreach sla [info globals *vtkFreeSurferReadersLabels] {
        global $sla
        array unset $sla
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersAddColors
# Reads in the freesurfer colour file, ColorsFreesurfer.xml, and appends the color tags to the input argument, returning them all. TODO: check for duplicate colour tags and give a warning. 
# .ARGS
# list tags the already set tags, append to this variable.
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersAddColors {tags} {
    global vtkFreeSurferReaders env Module
    # read, and append default freesurfer colors.
    # Return a new list of tags, possibly including default colors.
    # TODO: check for conflicts in color names

    set filename $vtkFreeSurferReaders(colourFileName)

    if {$Module(verbose) == 1} {
        puts "Trying to read Freesurfer colours from \"$fileName\""
    }
    set tagsFSColors [MainMrmlReadVersion2.x $fileName]
    if {$tagsFSColors == 0} {
        set msg "Unable to read file default MRML Freesurfer color file '$fileName'"
        puts $msg
        tk_messageBox -message $msg
        return "$tags"
    }

    return "$tags $tagsFSColors"
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersCORHeaderRead
# Reads in the freesurfer coronal volume's header file.
# .ARGS
# path file the full path to the COR-.info file
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersCORHeaderRead {filename} {
    global vtkFreeSurferReaders Volume CORInfo

    # Check that a file name string has been passed in
    if {[string equal filename ""]} {
        DevErrorWindow "vtkFreeSurferReadersCORHeaderRead: empty file name"
        return 1
    }
    
    # Open the file for reading
    if {[catch {set fid [open $filename]} errMsg]} {
        puts "Error opening COR header file $filename for reading"
        return 1
    }

    # Read and parse the file into name/value pairs
    set fldPat {([a-zA-Z_]+[0-9]*)}
    set numberPat {([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?}
    set i 0
    set fld ""
    while { ![eof $fid] } {
        # Read a line from the file and analyse it.
        gets $fid line 
        # puts "Line $i: $line"
        set i [expr $i + 1]
        # get the field name
        if { [regexp $fldPat $line fld]} {
            set CORInfo($fld) {}
            # puts "Found field $fld"
        
            # get the values, first test for three floats
            if { [regexp "$numberPat +$numberPat +$numberPat" $line val]} {
                set CORInfo($fld) $val
            } else {
                # one float
                if { [regexp "$numberPat" $line val]} {
                    set CORInfo($fld) $val
                } else {
                    puts "Unable to match line $line"
                }
            }
        } else {
            # only print warning if not an empty line
            if {![regexp {()|( +)} $line]} {
                puts "WARNING: no field name found at the beginning of line: $line"
            }
        }

    }
    close $fid

    # set the global vars
#    set Volume(activeID) 
    set Volume(isDICOM) 0
    if {[info exists Volume(name)] == 0} { set Volume(name) "COR"}
    set Volume(lastNum) $CORInfo(imnr1)
    set Volume(width) $CORInfo(x)
    set Volume(height) $CORInfo(y)
    # distances are in m in the COR info file, we need mm
    set Volume(pixelWidth) [expr 1000.0 * $CORInfo(psiz)]
    set Volume(pixelHeight) [expr 1000.0 * $CORInfo(psiz)]
    set Volume(sliceThickness) [expr 1000.0 * $CORInfo(thick)]
    # use the x dimension to calculate spacing from the field of view
    # set Volume(sliceSpacing) [expr 1000.0 * [expr $CORInfo(fov) /  $Volume(width)]]
    # don't use the slice spacing to anything but 0 for cor files
    set Volume(sliceSpacing) 0
    set Volume(gantryDetectorTilt) 0
    set Volume(numScalars) 1
    set Volume(littleEndian) 1
    # scan order is Coronal   Posterior Anterior
    set Volume(scanOrder) {PA}
    set Volume(scalarType) {UnsignedChar}
    set Volume(readHeaders) 0
    set Volume(filePattern) {%s-%03d}
    set Volume(dimensions) {256 256}
    set Volume(imageRange) {1 256}
    # puts "vtkFreeSurferReadersCORHeaderRead: set slice spacing to $Volume(sliceSpacing), slice thickness to $Volume(sliceThickness)"
    return 0
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetUMLSMapping
# Set the surface label to UMLS mapping, generated via /home/nicole/bin/readumls.sh
# .ARGS
# 
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetUMLSMapping {} {
    # set up Freesurfer surface labels to UMLS mapping
    global vtkFreeSurferReadersSurface
    set vtkFreeSurferReadersSurface(0,surfacelabel) Unknown
    set vtkFreeSurferReadersSurface(0,umls) Unknown
    set vtkFreeSurferReadersSurface(1,surfacelabel) Corpus_callosum
    set vtkFreeSurferReadersSurface(1,umls) C0010090
    set vtkFreeSurferReadersSurface(2,surfacelabel) G_and_S_Insula_ONLY_AVERAGE
    set vtkFreeSurferReadersSurface(3,surfacelabel) G_cingulate-Isthmus
    set vtkFreeSurferReadersSurface(3,umls) C0175192
    set vtkFreeSurferReadersSurface(4,surfacelabel) G_cingulate-Main_part
    set vtkFreeSurferReadersSurface(4,umls) C0018427
    set vtkFreeSurferReadersSurface(5,surfacelabel) G_cuneus
    set vtkFreeSurferReadersSurface(5,umls) C0152307
    set vtkFreeSurferReadersSurface(6,surfacelabel) G_frontal_inf-Opercular_part
    set vtkFreeSurferReadersSurface(6,umls) C0262296
    set vtkFreeSurferReadersSurface(7,surfacelabel) G_frontal_inf-Orbital_part
    set vtkFreeSurferReadersSurface(7,umls) C0262300
    set vtkFreeSurferReadersSurface(8,surfacelabel) G_frontal_inf-Triangular_part
    set vtkFreeSurferReadersSurface(8,umls) C0262350
    set vtkFreeSurferReadersSurface(9,surfacelabel) G_frontal_middle
    set vtkFreeSurferReadersSurface(9,umls) C0152297
    set vtkFreeSurferReadersSurface(10,surfacelabel) G_frontal_superior
    set vtkFreeSurferReadersSurface(10,umls) C0152296
    set vtkFreeSurferReadersSurface(11,surfacelabel) G_frontomarginal
    set vtkFreeSurferReadersSurface(12,surfacelabel) G_insular_long
    set vtkFreeSurferReadersSurface(12,umls) C0228261
    set vtkFreeSurferReadersSurface(13,surfacelabel) G_insular_short
    set vtkFreeSurferReadersSurface(13,umls) C0262329
    set vtkFreeSurferReadersSurface(14,surfacelabel) G_occipital_inferior
    set vtkFreeSurferReadersSurface(14,umls) C0228231
    set vtkFreeSurferReadersSurface(15,surfacelabel) G_occipital_middle
    set vtkFreeSurferReadersSurface(15,umls) C0228220
    set vtkFreeSurferReadersSurface(16,surfacelabel) G_occipital_superior
    set vtkFreeSurferReadersSurface(16,umls) C0228230
    set vtkFreeSurferReadersSurface(17,surfacelabel) G_occipit-temp_lat-Or_fusiform
    set vtkFreeSurferReadersSurface(17,umls) C0228243
    set vtkFreeSurferReadersSurface(18,surfacelabel) G_occipit-temp_med-Lingual_part
    set vtkFreeSurferReadersSurface(18,umls) C0228244
    set vtkFreeSurferReadersSurface(19,surfacelabel) G_occipit-temp_med-Parahippocampal_part
    set vtkFreeSurferReadersSurface(19,umls) C0228249
    set vtkFreeSurferReadersSurface(20,surfacelabel) G_orbital
    set vtkFreeSurferReadersSurface(20,umls) C0152301
    set vtkFreeSurferReadersSurface(21,surfacelabel) G_paracentral
    set vtkFreeSurferReadersSurface(21,umls) C0228203
    set vtkFreeSurferReadersSurface(22,surfacelabel) G_parietal_inferior-Angular_part
    set vtkFreeSurferReadersSurface(22,umls) C0152305
    set vtkFreeSurferReadersSurface(23,surfacelabel) G_parietal_inferior-Supramarginal_part
    set vtkFreeSurferReadersSurface(23,umls) C0458319
    set vtkFreeSurferReadersSurface(24,surfacelabel) G_parietal_superior
    set vtkFreeSurferReadersSurface(24,umls) C0152303
    set vtkFreeSurferReadersSurface(25,surfacelabel) G_postcentral
    set vtkFreeSurferReadersSurface(25,umls) C0152302
    set vtkFreeSurferReadersSurface(26,surfacelabel) G_precentral
    set vtkFreeSurferReadersSurface(26,umls) C0152299
    set vtkFreeSurferReadersSurface(27,surfacelabel) G_precuneus
    set vtkFreeSurferReadersSurface(27,umls) C0152306
    set vtkFreeSurferReadersSurface(28,surfacelabel) G_rectus
    set vtkFreeSurferReadersSurface(28,umls) C0152300
    set vtkFreeSurferReadersSurface(29,surfacelabel) G_subcallosal
    set vtkFreeSurferReadersSurface(29,umls) C0175231
    set vtkFreeSurferReadersSurface(30,surfacelabel) G_subcentral
    set vtkFreeSurferReadersSurface(31,surfacelabel) G_temp_sup-G_temp_transv_and_interm_S
    set vtkFreeSurferReadersSurface(31,umls) C0152309
    set vtkFreeSurferReadersSurface(32,surfacelabel) G_temp_sup-Lateral_aspect
    set vtkFreeSurferReadersSurface(32,umls) C0152309
    set vtkFreeSurferReadersSurface(33,surfacelabel) G_temp_sup-Planum_polare
    set vtkFreeSurferReadersSurface(33,umls) C0152309
    set vtkFreeSurferReadersSurface(34,surfacelabel) G_temp_sup-Planum_tempolale
    set vtkFreeSurferReadersSurface(34,umls) C0152309
    set vtkFreeSurferReadersSurface(35,surfacelabel) G_temporal_inferior
    set vtkFreeSurferReadersSurface(35,umls) C0152311
    set vtkFreeSurferReadersSurface(36,surfacelabel) G_temporal_middle
    set vtkFreeSurferReadersSurface(36,umls) C0152310
    set vtkFreeSurferReadersSurface(37,surfacelabel) G_transverse_frontopolar
    set vtkFreeSurferReadersSurface(38,surfacelabel) Lat_Fissure-ant_sgt-ramus_horizontal
    set vtkFreeSurferReadersSurface(38,umls) C0262190
    set vtkFreeSurferReadersSurface(39,surfacelabel) Lat_Fissure-ant_sgt-ramus_vertical
    set vtkFreeSurferReadersSurface(39,umls) C0262186
    set vtkFreeSurferReadersSurface(40,surfacelabel) Lat_Fissure-post_sgt
    set vtkFreeSurferReadersSurface(40,umls) C0262310
    set vtkFreeSurferReadersSurface(41,surfacelabel) Medial_wall
    set vtkFreeSurferReadersSurface(42,surfacelabel) Pole_occipital
    set vtkFreeSurferReadersSurface(42,umls) C0228217
    set vtkFreeSurferReadersSurface(43,surfacelabel) Pole_temporal
    set vtkFreeSurferReadersSurface(43,umls) C0149552
    set vtkFreeSurferReadersSurface(44,surfacelabel) S_calcarine
    set vtkFreeSurferReadersSurface(44,umls) C0228224
    set vtkFreeSurferReadersSurface(45,surfacelabel) S_central
    set vtkFreeSurferReadersSurface(45,umls) C0228188
    set vtkFreeSurferReadersSurface(46,surfacelabel) S_central_insula
    set vtkFreeSurferReadersSurface(46,umls) C0228260
    set vtkFreeSurferReadersSurface(47,surfacelabel) S_cingulate-Main_part
    set vtkFreeSurferReadersSurface(47,umls) C0228189
    set vtkFreeSurferReadersSurface(48,surfacelabel) S_cingulate-Marginalis_part
    set vtkFreeSurferReadersSurface(48,umls) C0259792
    set vtkFreeSurferReadersSurface(49,surfacelabel) S_circular_insula_anterior
    set vtkFreeSurferReadersSurface(49,umls) C0228258
    set vtkFreeSurferReadersSurface(50,surfacelabel) S_circular_insula_inferior
    set vtkFreeSurferReadersSurface(50,umls) C0228258
    set vtkFreeSurferReadersSurface(51,surfacelabel) S_circular_insula_superior
    set vtkFreeSurferReadersSurface(51,umls) C0228258
    set vtkFreeSurferReadersSurface(52,surfacelabel) S_collateral_transverse_ant
    set vtkFreeSurferReadersSurface(52,umls) C0228226
    set vtkFreeSurferReadersSurface(53,surfacelabel) S_collateral_transverse_post
    set vtkFreeSurferReadersSurface(53,umls) C0228226
    set vtkFreeSurferReadersSurface(54,surfacelabel) S_frontal_inferior
    set vtkFreeSurferReadersSurface(54,umls) C0262251
    set vtkFreeSurferReadersSurface(55,surfacelabel) S_frontal_middle
    set vtkFreeSurferReadersSurface(55,umls) C0228199
    set vtkFreeSurferReadersSurface(56,surfacelabel) S_frontal_superior
    set vtkFreeSurferReadersSurface(56,umls) C0228198
    set vtkFreeSurferReadersSurface(57,surfacelabel) S_frontomarginal
    set vtkFreeSurferReadersSurface(57,umls) C0262269
    set vtkFreeSurferReadersSurface(58,surfacelabel) S_intermedius_primus-Jensen
    set vtkFreeSurferReadersSurface(59,surfacelabel) S_intracingulate
    set vtkFreeSurferReadersSurface(59,umls) C0262263
    set vtkFreeSurferReadersSurface(60,surfacelabel) S_intraparietal-and_Parietal_transverse
    set vtkFreeSurferReadersSurface(60,umls) C0228213
    set vtkFreeSurferReadersSurface(61,surfacelabel) S_occipital_anterior
    set vtkFreeSurferReadersSurface(61,umls) C0262194
    set vtkFreeSurferReadersSurface(62,surfacelabel) S_occipital_inferior
    set vtkFreeSurferReadersSurface(62,umls) C0262253
    set vtkFreeSurferReadersSurface(63,surfacelabel) S_occipital_middle_and_Lunatus
    set vtkFreeSurferReadersSurface(63,umls) C0262278
    set vtkFreeSurferReadersSurface(64,surfacelabel) S_occipital_superior_and_transversalis
    set vtkFreeSurferReadersSurface(64,umls) C0262348
    set vtkFreeSurferReadersSurface(65,surfacelabel) S_occipito-temporal_lateral
    set vtkFreeSurferReadersSurface(65,umls) C0228245
    set vtkFreeSurferReadersSurface(66,surfacelabel) S_occipito-temporal_medial_and_S_Lingual
    set vtkFreeSurferReadersSurface(66,umls) C0447427
    set vtkFreeSurferReadersSurface(67,surfacelabel) S_orbital_lateral
    set vtkFreeSurferReadersSurface(67,umls) C0228229
    set vtkFreeSurferReadersSurface(68,surfacelabel) S_orbital_medial-Or_olfactory
    set vtkFreeSurferReadersSurface(68,umls) C0262286
    set vtkFreeSurferReadersSurface(69,surfacelabel) S_orbital-H_shapped
    set vtkFreeSurferReadersSurface(69,umls) C0228206
    set vtkFreeSurferReadersSurface(70,surfacelabel) S_paracentral
    set vtkFreeSurferReadersSurface(70,umls) C0228204
    set vtkFreeSurferReadersSurface(71,surfacelabel) S_parieto_occipital
    set vtkFreeSurferReadersSurface(71,umls) C0228191
    set vtkFreeSurferReadersSurface(72,surfacelabel) S_pericallosal
    set vtkFreeSurferReadersSurface(73,surfacelabel) S_postcentral
    set vtkFreeSurferReadersSurface(73,umls) C0228212
    set vtkFreeSurferReadersSurface(74,surfacelabel) S_precentral-Inferior-part
    set vtkFreeSurferReadersSurface(74,umls) C0262257
    set vtkFreeSurferReadersSurface(75,surfacelabel) S_precentral-Superior-part
    set vtkFreeSurferReadersSurface(75,umls) C0262338
    set vtkFreeSurferReadersSurface(76,surfacelabel) S_subcentral_ant
    set vtkFreeSurferReadersSurface(76,umls) C0262197
    set vtkFreeSurferReadersSurface(77,surfacelabel) S_subcentral_post
    set vtkFreeSurferReadersSurface(77,umls) C0262318
    set vtkFreeSurferReadersSurface(78,surfacelabel) S_suborbital
    set vtkFreeSurferReadersSurface(79,surfacelabel) S_subparietal
    set vtkFreeSurferReadersSurface(79,umls) C0228216
    set vtkFreeSurferReadersSurface(80,surfacelabel) S_supracingulate
    set vtkFreeSurferReadersSurface(81,surfacelabel) S_temporal_inferior
    set vtkFreeSurferReadersSurface(81,umls) C0228242
    set vtkFreeSurferReadersSurface(82,surfacelabel) S_temporal_superior
    set vtkFreeSurferReadersSurface(82,umls) C0228237
    set vtkFreeSurferReadersSurface(83,surfacelabel) S_temporal_transverse
    set vtkFreeSurferReadersSurface(83,umls) C0228239
    set vtkFreeSurferReadersSurface(84,surfacelabel) S_transverse_frontopolar

    return
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReadAnnotations 
# Read in the annotations specified for this model id, building file names and calling 
# vtkFreeSurferReadersReadAnnotation for each one to load.
# .ARGS
# int _id model id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReadAnnotations {_id} {
    global vtkFreeSurferReaders Volume Model

    # read in the associated annot file, it's one up from the surf dir, then down into label
    set dir [file split [file dirname $vtkFreeSurferReaders(ModelFileName)]]
    set dir [lrange $dir 0 [expr [llength $dir] - 2]]
    lappend dir label
    set dir [eval file join $dir]
    set fname [lrange [file split [file rootname $vtkFreeSurferReaders(ModelFileName)]] end end]
    
    foreach a $vtkFreeSurferReaders(annots) {
        if {[lsearch $vtkFreeSurferReaders(assocFiles) $a] != -1} {
            set annotFileName [file join $dir $fname.$a.annot]

            vtkFreeSurferReadersReadAnnotation $a $_id $annotFileName 
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReadAnnotation
# Read in the annotation file specified for the model id
# .ARGS
# string a name of the annotation type
# int _id model id, defaults to -1, get it from the active id
# string annotFileName name of the file to load, defaults to empty string, get it from vtkFreeSurferReaders(annotFileName)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReadAnnotation {a {_id -1} {annotFileName ""}} {
    global vtkFreeSurferReaders Volume Model

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersReadAnnotation: before ver: id = $_id, file name = $annotFileName"
    }

    if {$_id == -1} {
        set _id $::Model(activeID)
    }

    if {$annotFileName == ""} {
        set annotFileName $vtkFreeSurferReaders(annotFileName)
    }

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersReadAnnotation: after ver: id = $_id, file name = $annotFileName"
    }

    if [file exists $annotFileName] {
        puts "Model $_id: Reading in $a file associated with this model:\n$annotFileName"
        set scalaridx [[$Model($_id,polyData) GetPointData] SetActiveScalars "labels"] 
                    
        if { $scalaridx == "-1" } {
            if {$::Module(verbose)} {
                puts "labels scalar doesn't exist for model $_id, creating"
            }
            set scalars scalars_$a
            catch "$scalars Delete"
            vtkIntArray $scalars
            $scalars SetName "labels"
            [$Model($_id,polyData) GetPointData] AddArray $scalars
            [$Model($_id,polyData) GetPointData] SetActiveScalars "labels"
        } else {
            set scalars [[$Model($_id,polyData) GetPointData] GetScalars $scalaridx]
        }
        # the look up table should be a label one
        set lut [Model($_id,mapper,viewRen) GetLookupTable]

        # re-allocate the look up table, so that it will work with internal tables
        catch "fssarlut_${_id} Delete"
        set lut [vtkLookupTable fssarlut_${_id}]
                
        set fssar fssar_$a
        catch "$fssar Delete"
        vtkFSSurfaceAnnotationReader $fssar  
        if {$::Module(verbose)} {
            $fssar DebugOn
        }
        $fssar SetFileName $annotFileName
        $fssar SetOutput $scalars
        $fssar SetColorTableOutput $lut
        # try reading an internal colour table first
        $fssar UseExternalColorTableFileOff
        
        # set up a progress observer 
        $fssar AddObserver StartEvent MainStartProgress
        $fssar AddObserver ProgressEvent "MainShowProgress $fssar"
        $fssar AddObserver EndEvent       MainEndProgress
        set ::Gui(progressText) "Reading $a"
        
        set retval [$fssar ReadFSAnnotation]
        if {$retval == 6} {
            # no internal colour table, try external one
            if [file exists $vtkFreeSurferReaders(colorTableFilename)] {
                if {$::Module(verbose)} {
                    puts "No internal colour table, but an external color table file exists: $vtkFreeSurferReaders(colorTableFilename)"
                }
                $fssar SetColorTableFileName $vtkFreeSurferReaders(colorTableFilename)
                $fssar UseExternalColorTableFileOn
            
                if {$::Module(verbose)} { puts "Trying to use external colour table file $vtkFreeSurferReaders(colorTableFilename)"}
                set retval [$fssar ReadFSAnnotation]
            } else {
                puts "No internal colour table, and external one does not exist: $vtkFreeSurferReaders(colorTableFilename)"
            }
        } else {
            if {$::Module(verbose)} { puts "Used internal colour table" }
        }

        MainEndProgress

        if {$::Module(verbose)} {
            puts "Return value from reading $annotFileName = $retval"
        }
        if {[vtkFreeSurferReadersCheckAnnotError $retval] != 0} {
            [$Model($_id,polyData) GetPointData] RemoveArray "labels"
            return
        }

        # set the lut for the model
        if {$::Module(verbose)} { 
            puts "Model mapper for model id $_id, settign lookup table $lut ([$lut GetNumberOfTableValues] values)"
        }
        Model($_id,mapper,viewRen) SetLookupTable $lut

        set ::Model(scalarVisibilityAuto) 0
        
        array unset _labels
        array set _labels [$fssar GetColorTableNames]
        # save the array for future browsing
        array unset ::${_id}vtkFreeSurferReadersLabels
        array set ::${_id}vtkFreeSurferReadersLabels [array get _labels]

        set entries [lsort -integer [array names _labels]]
        

        if {$::Module(verbose)} {
            puts "Label entries:\n$entries"
            puts "0th: [lindex $entries 0], last:  [lindex $entries end]"
        }
        MainModelsSetScalarRange $_id [lindex $entries 0] [lindex $entries end]
        MainModelsSetScalarVisibility $_id 1

        # might need to call ModelsPickScalarsCallback here to get everything right
# Don't call this, as we're using a different look up table that isn't integrated with the models yet
        # ModelsPickScalarsCallback $_id [$Model($_id,polyData) GetPointData] "labels"

        Render3D
    } else {
        DevInfoWindow "Model $_id: $a file cannot be found: $annotFileName"
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersCheckAnnotError
# references vtkFSSurfaceAnnotationReader.h. Returns error value (0 means success)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersCheckAnnotError {val} {
    if {$val == 1} {
        DevErrorWindow "ERROR: error loading colour table"
    }
    if {$val == 2} {
        DevErrorWindow "ERROR: error loading annotation"
    }
    if {$val == 3} {
        DevErrorWindow "ERROR: error parsing color table"
    }
    if {$val == 4} {
        DevErrorWindow "ERROR: error parsing annotation"
    }
    if {$val == 5} {
        DevErrorWindow "WARNING: Unassigned labels"
        return 0
    }
    if {$val == 6} {
        DevErrorWindow "ERROR: no colour table defined"
    }
    return $val
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersCheckWError
# references vtkFSSurfaceWFileReader.h. Returns error value (0 means success)
# after calling DevErrorWindow with a useful message
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersCheckWError {val} {
    if {$val == 1} {
        DevErrorWindow "ERROR: output is null"
    }
    if {$val == 2} {
        DevErrorWindow "ERROR: FileName not specified"
    }
    if {$val == 3} {
        DevErrorWindow "ERROR: Could not open file"
    }
    if {$val == 4} {
        DevErrorWindow "ERROR: Number of values in the file is 0 or negative, or greater than number of vertices in the associated scalar file"
    }
    if {$val == 5} {
        DevErrorWindow "ERROR: Error allocating the array of floats"
    }
    if {$val == 6} {
        DevErrorWindow "ERROR: Unexpected EOF"
    }
    return $val
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersModelApply
# Read in the model specified. Used in place of the temp ModelsFreeSurferPropsApply.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersModelApply {} {
    global vtkFreeSurferReaders Module Model Volume
    # Validate name
    set Model(name) $vtkFreeSurferReaders(ModelName)
    if {$Model(name) == ""} {
        tk_messageBox -message "Please enter a name that will allow you to distinguish this model."
        return
    }
    if {[ValidateName $Model(name)] == 0} {
        tk_messageBox -message "The name can consist of letters, digits, dashes, or underscores"
        return
    }
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersModelApply: Also loading these assoc files if they exist: $vtkFreeSurferReaders(assocFiles)"
    }
    # add a mrml node
    set n [MainMrmlAddNode Model]
    set i [$n GetID]

    vtkFreeSurferReadersBuildSurface $i
    vtkFreeSurferReadersReadAnnotations $i

    # make sure it's pickable
    ::Model($i,actor,viewRen) SetPickable 1
    # and allow browsing scalar values via clicks on it
    set buttonNum 3
    set bindstr [bind $::Gui(fViewWin)]
    if {$bindstr == "" || [regexp ".ButtonRelease-${buttonNum}." $bindstr matchVar] == 0} {
        # only bind if not bound before
        if {$::Module(verbose)} { puts "binding again: $bindstr" }
        bind $::Gui(fViewWin) <ButtonRelease-${buttonNum}> {vtkFreeSurferReadersPickScalar %W %x %y}
    } else {
        if {$::Module(verbose)} { puts "not binding again: $bindstr" }
    }

        
    # allow use of other module GUIs
    set Volumes(freeze) 0
    # If tabs are frozen, then return to the "freezer"
    if {$Module(freezer) != ""} {
        set cmd "Tab $Module(freezer)"
        set Module(freezer) ""
        eval $cmd
    }
    return $i
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersModelCancel
# Cancel reading in the model
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersModelCancel {} {
    global vtkFreeSurferReaders Module Model Volume
    # model this after VolumePropsCancel - ModelPropsCancel?
    if {$Module(verbose)} {
        puts "vtkFreeSurferReadersModelCancel, calling ModelPropsCancel"
    }
    ModelsPropsCancel
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetLoad
# Add this kind of associated file to the list of files to load when reading in a model
# .ARGS
# string param - the kind of associated file to read in, added to vtkFreeSurferReaders(assocFiles)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetLoad {param} {
    global vtkFreeSurferReaders Volume

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetLoad: start: loading: $vtkFreeSurferReaders(assocFiles)"
    }
    set ind [lsearch $vtkFreeSurferReaders(assocFiles) $param]
    if {$vtkFreeSurferReaders(assocFiles,$param) == 1} {
        if {$ind == -1} {
            # add it to the list
            lappend vtkFreeSurferReaders(assocFiles) $param
        }
    } else {
        if {$ind != -1} {
            # remove it from the list
            set vtkFreeSurferReaders(assocFiles) [lreplace $vtkFreeSurferReaders(assocFiles) $ind $ind]
        }
    }
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetLoad: new: loading: $vtkFreeSurferReaders(assocFiles)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadVolume
# Scriptable load function -- tied into command line arguments
# example: 
# ./slicer2-win32.exe --load-freesurfer-model c:/pieper/bwh/data/MGH-Siemens15-SP.1-uw/surf/lh.pial --load-freesurfer-volume c:/pieper/bwh/data/MGH-Siemens15-SP.1-uw/mri/orig/COR-.info --load-freesurfer-label-volume c:/pieper/bwh/data/MGH-Siemens15-SP.1-uw/mri/aseg/COR-.info &
# .ARGS 
# path filename volume file to load
# string labelMap the label map associated with this volume
# string name the volume name
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadVolume { filename {labelMap 0} {name ""} } {

    if {[file exists $filename] == 0} {
        DevErrorWindow "Load FreeSurfer Volume: $filename does not exist!"
        return
    }

    set ::Volume(labelMap) $labelMap
    set ::vtkFreeSurferReaders(VolumeFileName) $filename

    
    vtkFreeSurferReadersSetVolumeFileName 
    if { $name != "" } {
        set ::Volume(name) $name
    }
    set retval [vtkFreeSurferReadersApply]
    if {$::Module(verbose)} { puts "vtkFreeSurferReadersLoadVolume returning id $retval" }
    return $retval
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadModel
# Scriptable load function -- tied into command line arguments
# .ARGS 
# path filename model file to load
# string name name of the model
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadModel { filename {name ""} } {

    if {[file exists $filename] == 0} {
        DevErrorWindow "Load FreeSurfer Model: $filename does not exist!"
        return
    }

    set ::vtkFreeSurferReaders(ModelFileName) $filename
    vtkFreeSurferReadersSetModelFileName
    if { $name != "" } {
        set ::Volume(name) $name
    }
    return [vtkFreeSurferReadersModelApply]
}

#-------------------------------------------------------------------------------
# .PROC FreeSurferReadersFiducialsPointCreatedCallback
# This procedures is a callback procedule called when a Fiducial Point is
# created - use this to update the query atlas window
# .ARGS 
# string type not used
# int fid not used
# int pid not used
# .END
#-------------------------------------------------------------------------------
proc FreeSurferReadersFiducialsPointCreatedCallback {type fid pid} {

    if { [catch "package require Itcl"] } {
        return
    }

    foreach r [itcl::find objects -class regions] {
        $r findptscalars
    }

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReadMGH
# Called by MainVolumes.tcl MainVolumesRead to read in an MGH volume, returns -1
# if there is no vtkMGHReader. Assumes that the volume has been read already
# .ARGS
# int v volume ID
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReadMGH {v} {
    global Volume
    if { [info commands vtkMGHReader] == "" } {
        DevErrorWindow "No MGH Reader available."
        return -1
    }
    catch "mghreader Delete"
    vtkMGHReader mghreader
            
    mghreader AddObserver StartEvent MainStartProgress
    mghreader AddObserver ProgressEvent "MainShowProgress mghreader"
    mghreader AddObserver EndEvent       MainEndProgress
    set ::Gui(progressText) "Reading mgh"


    mghreader SetFileName [Volume($v,node) GetFullPrefix]

    # flip it on reading in
    catch "flipper Delete"
    vtkImageFlip flipper
    flipper SetFilteredAxis 1
    flipper SetInput [mghreader GetOutput]

    mghreader Update
    mghreader ReadVolumeHeader
    [[mghreader GetOutput] GetPointData] SetScalars [mghreader ReadVolumeData]

    # Volume($v,vol) SetImageData [mghreader GetOutput]
    Volume($v,vol) SetImageData [flipper GetOutput]

    mghreader Delete

    MainEndProgress

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReadBfloat
# Called by MainVolumes.tcl MainVolumesRead to read in a bfloat volume
# .ARGS
# int v volume ID
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReadBfloat {v} {
    global Volume

# puts "\n\n\n\nvtkFreeSurferReadersReadBfloat $v - just returns"

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersReadBfloat: volume id = $v"
  #      DevInfoWindow "vtkFreeSurferReadersReadBfloat: volume id = $v - SKIPPING this!"
   #     return
    }

    if { [info commands vtkBVolumeReader] == "" } {
        DevErrorWindow "No MGH B Volume Reader available."
        return -1
    }

    catch "bfloatreader Delete"
    vtkBVolumeReader bfloatreader

    if {$::Module(verbose)} {
        puts "ReadBFloat: setting debug on on the reader"
        bfloatreader DebugOn
    }
    # not going to work, need to recreate the file name from what's saved in the node: FilePattern, FilePrefix, and ImageRange
    # bfloatreader SetFileName  [Volume($v,node) GetFullPrefix]
    set stem [Volume($v,node) GetFullPrefix]
    bfloatreader SetFilePrefix $stem
    bfloatreader SetStem $stem

    # set up the progress reporting
    set ::Gui(progressText) "Reading b data volume"
    bfloatreader AddObserver StartEvent MainStartProgress
    bfloatreader AddObserver ProgressEvent "MainShowProgress bfloatreader"
    bfloatreader AddObserver EndEvent MainEndProgress

    bfloatreader Update
    bfloatreader SetRegistrationFileName ${stem}.dat
    set regmat [bfloatreader GetRegistrationMatrix]

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersReadBfloat: num time points = [bfloatreader GetNumTimePoints]"
    }

#    Volume($v,vol) SetImageData [bfloatreader GetOutput]
    bfloatreader Delete
    MainEndProgress
}

#-------------------------------------------------------------------------------
# .PROC 
# vtkFreeSurferReadersSetCast
# Prints out confirmation of the castToShort flag
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetCast {} {
    global vtkFreeSurferReaders
    if {$::Module(verbose)} {
        puts "cast to short = $vtkFreeSurferReaders(castToShort)"
    }
}

#-------------------------------------------------------------------------------
# .PROC 
# vtkFreeSurferReadersCast
# Casts the given input volume v to toType. Returns new volume id on success, -1 on failure.
# .ARGS
# int v input volume
# string toType the output type, valid values are Float Double Int UnsignedInt Long UnsignedLong Short UnsignedShort Char UnsignedChar
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersCast {v {toType "Short"}} {
    global vtkFreeSurferReaders Volume

    if {$v == $Volume(idNone)} {
        DevErrorWindow "vtkFreeSurferReadersCast: You cannot cast Volume \"None\""
        return -1
    }
    set typeList {Float Double Int UnsignedInt Long UnsignedLong Short UnsignedShort Char UnsignedChar}
    if {[lsearch $typeList $toType] == -1} {
        DevErrorWindow "vtkFreeSurferReadersCast: ERROR: casting only is valid to these types:\n$typeList"
        return -1
    }

    if {$::Module(verbose)} { 
        puts "vtkFreeSurferReadersCast: casting volume $v to $toType"
    }
   
    # prepare the new volume
    set name [Volume($v,node) GetName]-$toType
    set vCast [DevCreateNewCopiedVolume $v "" $name]

    # this seems to not get done in the prior call
    Volume($vCast,node) SetLabelMap [Volume($v,node) GetLabelMap]

    set node [Volume($vCast,vol) GetMrmlNode]
    Mrml(dataTree) RemoveItem $node
    set nodeBefore [Volume($v,vol) GetMrmlNode]
    Mrml(dataTree) InsertAfterItem $nodeBefore $node
    MainUpdateMRML

    # do the cast
    catch "vtkFreeSurferReadersCaster Delete"
    vtkImageCast vtkFreeSurferReadersCaster
    vtkFreeSurferReadersCaster SetInput [Volume($v,vol) GetOutput]
    vtkFreeSurferReadersCaster SetOutputScalarTypeTo$toType

    vtkFreeSurferReadersCaster ClampOverflowOn
    vtkFreeSurferReadersCaster Update

    # Start copying in the output data.
    # Taken from VolumeMathDoCast
    Volume($vCast,vol) SetImageData [vtkFreeSurferReadersCaster GetOutput]
    Volume($vCast,node) SetScalarTypeTo$toType
    MainVolumesUpdate $vCast

    vtkFreeSurferReadersCaster Delete

    return $vCast
}

#-------------------------------------------------------------------------------
# .PROC 
# vtkFreeSurferReadersSetLoadColours
# Prints out confirmation of the loadColours flag
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetLoadColours {} {
    global vtkFreeSurferReaders
    if {$::Module(verbose)} {
        puts "load freesurfer colours = $vtkFreeSurferReaders(loadColours)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetColourFileName
# The filename is set elsewhere, in variable vtkFreeSurferReaders(colourFileName)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetColourFileName {} {
    global vtkFreeSurferReaders Volume
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetColourFileName: colour file name set to $vtkFreeSurferReaders(colourFileName)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadColour
# Reads in an xml or a text FreeSurfer colour file.
# xml reading piggy backs on the Color module by setting Color(fileName) from 
# vtkFreeSurferReaders(colourFileName), then loading.
# Can also read in a text file. The format is
# id<white space>name<white-space>r<w-s>g<w-s>b<w-s>a\n
# where r, g, b go from 0-255. a is not used.
# Comments are preceded by hash marks at the start of the line.
# Converts to colour nodes with the diffuse colours set to the input colours divided by 255.
# Sets the vtkFreeSurferReaders(coloursLoaded) flag to 0, and then to 1 on sucess.
# .ARGS
# int overwriteFlag if 1, delete the old colours, else, append to them. Defaults to 1
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadColour { {overwriteFlag 1}} {
    global vtkFreeSurferReaders

    set numColours 0
    set tagsColours ""
    set vtkFreeSurferReaders(coloursLoaded) 0
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersLoadColour $vtkFreeSurferReaders(colourFileName)"
    }

    # check if have an xml file already
    if {[file ext $vtkFreeSurferReaders(colourFileName)] == ".xml"} {
        # load via the xml loading proc in the Colors module
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersLoadColour: Got an xml file"
        }
        set ::Color(fileName) $vtkFreeSurferReaders(colourFileName)
        ColorsLoadApply $overwriteFlag
    } elseif {[file ext $vtkFreeSurferReaders(colourFileName)] == ".txt"} {
        # open the text file for parsing 
        if {[catch {set fd [open $vtkFreeSurferReaders(colourFileName) r]} errmsg] == 1} {
            DevErrorWindow "ERROR opening $vtkFreeSurferReaders(colourFileName):\n$errmsg"
            return
        }
        # read in 
        while {![eof $fd]} {
            set line [gets $fd]
            # don't process if it's a commented out line
            if {[regexp "^\#(.*)" $line matchVar commentStr] == 1} {
 #                 if {$::Module(verbose)} { puts "vtkFreeSurferReadersLoadColour: got a comment line:\n$line" }
            } else {
                # if {$::Module(verbose)} { puts "vtkFreeSurferReadersLoadColour: working on line:\n$line" }
                set linelist ""
                foreach elem [split $line] {
                    if {$elem != {}} {
                        lappend linelist $elem
                    }
                }
                if {[llength $linelist] == 6} {
                    # got the id, name, rgba
                    incr numColours
                    foreach {id name r g b a} $linelist {
                        if {$::Module(verbose)} {
                            puts "id = $id, name = $name, r = $r, g = $g, b = $b, a = $a"
                        }
                    }
                    # now add to the colour tags list
                    lappend tagColours [list Color [list options ""] [list name $name] [list diffusecolor [expr $r/255.0] [expr $g/255.0] [expr $b/255.0]] [list labels $id]]
                } else {
                    # if {$::Module(verbose)} { puts "skipping line $linelist" }
                }
            }
        }
        # close the file
        close $fd

        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersLoadColour: found $numColours colours in the file"
            puts "colour tags = \n$tagColours\n"
        }
        if {$numColours > 0} {
            
            # got some new colours, saved them in the tagsColours, so build the new nodes

            if {$overwriteFlag == 1} {
                MainMrmlDeleteColors                
            }
            MainMrmlBuildTreesVersion2.0 $tagColours
            
            if {$overwriteFlag == 1} {
                # should check here to make sure all models have valid colour id's
                ColorsVerifyModelColorIDs
            }

            # update the gui's color list
            ColorsDisplayColors
            MainColorsUpdateMRML
            # rebuild the canvas
            LabelsUpdateMRML
            # show changes
            RenderAll
        } else {
            DevWarningWindow "No colours found in $vtkFreeSurferReaders(colourFileName)"
            return
        }
    } else {
        DevErrorWindow "Cannot load the colour file due to an unknown extension: $vtkFreeSurferReaders(colourFileName)"
        return
    }
    set vtkFreeSurferReaders(coloursLoaded) 1
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFInit
# Initializes the list of valid markers and colours for GDF
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFInit {} {
    global vtkFreeSurferReaders 

    set ::Module(verbose) 1

    set vtkFreeSurferReaders(kValid,lMarkers)  {square circle diamond plus cross splus scross triangle}
    set vtkFreeSurferReaders(kValid,lColors) {red blue green yellow black purple orange pink brown}
    set vtkFreeSurferReaders(gGDF,lID) {}
    # should check to see here if the library is loaded
    # incorporate it into the vtkFreeSurferReaders library, then change this to 1
    set vtkFreeSurferReaders(gbLibLoaded) 0
    if {[info command vtkGDFReader] == ""} {
        # the library wasn't loaded in properly
        set vtkFreeSurferReaders(gbLibLoaded) 0
    puts "vtkGDFReader wasn't loaded"
    } else {
        # declare a variable and save it
        catch "vtkFreeSurferReaders(gdfReader) Delete"
        vtkGDFReader vtkFreeSurferReaders(gdfReader)
        set vtkFreeSurferReaders(gbLibLoaded) 1
    puts "declared a vtkGDFReader"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotBuildWindow
# Creates the window for plotting into.
# vtkFreeSurferReaders(gGDF) - information gleaned from the header file.
# <br>   lID - list of IDs
# <br>   ID
# <br>     bReadHeader - whether or not his GDF is parsed correctly
# <br>     title - title of the graph
# <br>     measurementName - label for the measurement
# <br>     subjectName - subject name
# <br>     dataFileName - data file name
# <br>     cClasses - number of classes
# <br>     classes,n - n is 0 -> cClasses
# <br>       label - label for this class
# <br>       marker - marker for this class
# <br>       color - color for this class
# <br>       subjects,n - n is 0 -> num subjects in this class
# <br>         index - index of the subject
# <br>     classes,label - label is the label
# <br>       index - index is the index of this label
# <br>     cVariables - number of variables
# <br>     variables,n - n is 0 -> cVariables
# <br>       label - label for this variable
# <br>     nDefaultVariable - index of default variable
# <br>     cSubjects - number of subjects
# <br>     subjects,n - n is 0 -> cSubjects
# <br>       id - label of this subject
# <br>       nClass - index of class of this subject
# <br>       variables,n - n is 0 -> cVariables
# <br>         value - value for this variable for this subject
# <br> vtkFreeSurferReaders(gPlot) - information about the plot, including current state.n
# <br>   ID
# <br>     state
# <br>       nVariable - the index of the current variable
# <br>       info - the info string displayed in lwInfo
# <br>       lPoints - list of points
# <br>       pointsChanged - dirty flag for points
# <br>       data,subjects,n - where n is 0 -> cSubjects
# <br>         variable - variable value for this subject (for state,nVariable)
# <br>         measurement - measurement value for this subject
# <br>       hiElement - name of hilighted element in plot
# <br>       subjects,n - where n is 0 -> cSubjects
# <br>         visible - whether or not is visible
# <br>       classes,n - where n is 0 -> cClasses
# <br>         visible - whether or not is visible
# <br>       legend - subject or class
# <br>       bTryRegressionLine - whether or not to try getting the offset/slope
# <br> vtkFreeSurferReaders(gWidgets) - names of widgets
# <br>   ID
# <br>     wwTop - the top window
# <br>     gwPlot - the graph widget
# <br>     lwInfo - the info label widget
# <br>     bWindowBuilt - boolean indicating if the window has been built
# <br>     state
# <br>       window
# <br>         geometry - if hidden and reshown, will appear with same geometry
# .ARGS
# int iID window id 
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotBuildWindow { iID } {
    global vtkFreeSurferReaders 

    set wwTop         .fsgdf-$iID
    set gwPlot        $wwTop.gwPlot
    set lwInfo        $wwTop.lwInfo
    set owVar         $wwTop.owVar
    set owLegendMode  $wwTop.owLegendMode
    set fwClassConfig $wwTop.fwClassConfig

    package require Iwidgets

    if {[info command $wwTop] != ""} {
        if {$::Module(verbose)} {
            puts "Already have a $wwTop, rebuilding anyway"
        }
     #   wm deiconify $wwTop
     #   return
    }
    # Make the to window and set its title.
    if {[info command $wwTop] == ""} {
    toplevel $wwTop -height 500 -width 500
    }
    wm title $wwTop $vtkFreeSurferReaders(gGDF,$iID,title)

    if { [catch "package require BLT"] } {
        puts "Must have the BLT package for plotting."
        return
    }
    # Make the graph.
    if {[info command $gwPlot] == ""} {
    blt::graph $gwPlot \
        -title $vtkFreeSurferReaders(gGDF,$iID,title) \
        -plotbackground white \
        -relief raised -border 2
    }

    # Bind our callbacks.
    $gwPlot legend bind all <Enter> [list vtkFreeSurferReadersGDFPlotCBLegendEnter $iID %W]
    $gwPlot legend bind all <Leave> [list vtkFreeSurferReadersGDFPlotCBLegendLeave $iID %W]
    $gwPlot legend bind all <ButtonPress-1> [list vtkFreeSurferReadersGDFPlotCBLegendClick $iID %W]
    bind $gwPlot <Motion> [list vtkFreeSurferReadersGDFPlotCBGraphMotion $iID %W %x %y]
    bind $gwPlot <Destroy> [list vtkFreeSurferReadersGDFPlotCBCloseWindow $iID] 

    # Hooking up the zoom functions seems to break some of the other
    # bindings. Needs more work.  
    # Blt_ZoomStack $gwPlot

    # Set the y axis label to the measurement name.
    $gwPlot axis configure y -title $vtkFreeSurferReaders(gGDF,$iID,measurementName)

    # Make the info label.
    set vtkFreeSurferReaders(gPlot,$iID,state,info) "Vertex number "
    # tkuMakeActiveLabel
    # will need to update this
    # DevAddLabel $lwInfo $vtkFreeSurferReaders(gPlot,$iID,state,info)
    eval {label $lwInfo -textvariable vtkFreeSurferReaders(gPlot,$iID,state,info)}

    # Make the variable menu.
    if {[info command $owVar] == ""} {
    iwidgets::optionmenu $owVar \
        -labeltext "Variable menu" \
        -command "vtkFreeSurferReadersPlotSetVariable $iID" 
        # -labelfont $::Gui(WLA)

    }
    # Make the mode menu.
    if {[info command $owLegendMode] == ""} {
    iwidgets::optionmenu $owLegendMode \
        -labeltext "Mode menu" \
        -command "vtkFreeSurferReadersPlotSetMode $iID" 
        # -labelfont $::Gui(WLA)
    }
    $owLegendMode config -state disabled 
#    $owLegendMode add command subject -label "View by subject"
#    $owLegendMode add command class -label "View by class"
    $owLegendMode insert end subject 
    $owLegendMode insert end class
    $owLegendMode config -state normal 

    # Make a frame for the class controls, which we'll fill in later.
    if {[info command $fwClassConfig] == ""} {
    iwidgets::labeledframe $fwClassConfig -labeltext "Configure Classes"
    

    # Place everything in the window if the widgets weren't created this time through
    grid $gwPlot        -column 0 -row 0 -columnspan 3 -sticky news
    grid $lwInfo        -column 0 -row 1 -sticky nwe
    grid $owLegendMode  -column 1 -row 1 -sticky se
    grid $owVar         -column 2 -row 1 -sticky se
    grid $fwClassConfig -column 0 -row 2 -columnspan 3 -sticky ews
    grid columnconfigure $wwTop 0 -weight 1
    grid columnconfigure $wwTop 1 -weight 0
    grid columnconfigure $wwTop 2 -weight 0
    grid rowconfigure $wwTop 0 -weight 1
    grid rowconfigure $wwTop 1 -weight 0
    grid rowconfigure $wwTop 2 -weight 0
    }

    # Set the names in the gWidgets array.
    set vtkFreeSurferReaders(gWidgets,$iID,wwTop)          $wwTop
    set vtkFreeSurferReaders(gWidgets,$iID,gwPlot)         $gwPlot
    set vtkFreeSurferReaders(gWidgets,$iID,lwInfo)         $lwInfo
    set vtkFreeSurferReaders(gWidgets,$iID,owVar)          $owVar
    set vtkFreeSurferReaders(gWidgets,$iID,owMode)         $owLegendMode
    set vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig)  [$fwClassConfig childsite]

    # Build the dynamic window elements for the window.
    vtkFreeSurferReadersGDFPlotBuildDynamicWindowElements $iID

    # Set the variable menu value to the header's default variable
    # index.
    $owVar select $vtkFreeSurferReaders(gGDF,$iID,nDefaultVariable)

    # Set our initial legen mode to class.
    $owLegendMode select class

    # Create the pen for our active element.
    $gwPlot pen create activeElement \
        -symbol circle -color red -pixels 0.2i -fill ""

    # Note that the window has been built.
    set vtkFreeSurferReaders(gWidgets,$iID,bWindowBuilt) 1
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotBuildDynamicWindowElements
# Builds the window elements that are dependant on data, including the
# variable menu and the class configuration section.
# .ARGS
# int iID the id of the window
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotBuildDynamicWindowElements { iID } {
    global vtkFreeSurferReaders

    # First delete all entries in the menu. Then for each variable,
    # make an entry with that variable's label. The command for the
    # menu has already been set.
    $vtkFreeSurferReaders(gWidgets,$iID,owVar) config -state disabled
    #   set lEntries [$vtkFreeSurferReaders(gWidgets,$iID,owVar) entries]
    #foreach entry $lEntries { 
    #    $vtkFreeSurferReaders(gWidgets,$iID,owVar) delete $entry
    #}

    $vtkFreeSurferReaders(gWidgets,$iID,owVar) delete 0 end
    for { set nVar 0 } { $nVar < $vtkFreeSurferReaders(gGDF,$iID,cVariables) } { incr nVar } {
        $vtkFreeSurferReaders(gWidgets,$iID,owVar) insert $nVar "$vtkFreeSurferReaders(gGDF,$iID,variables,$nVar,label)"
        
    }
    $vtkFreeSurferReaders(gWidgets,$iID,owVar) config -state normal

    # Fill out the class config frame. For each class, make an entry
    # with an option widget for colors and one for markers. Set up the
    # entries appropriately and bind it to the right variable.
    for { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$iID,cClasses) } { incr nClass } {

        set lw       $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig).lw$nClass
        set owMarker $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig).owMarker$nClass
        set owColor  $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig).owColor$nClass

        eval {label $lw -text $vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,label)}
#        DevAddLabel $lw $vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,label) 

        iwidgets::optionmenu $owMarker \
            -command "vtkFreeSurferReadersPlotSetNthClassMarker $iID $nClass" 
            # -labelfont $::Gui(WLA)
        $owMarker config -state disabled
        foreach marker $vtkFreeSurferReaders(kValid,lMarkers) {
            # $owMarker add command $marker -label $marker
            $owMarker insert end $marker
        }
        $owMarker config -state normal
        $owMarker select $vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,marker)
        
        iwidgets::optionmenu $owColor \
            -command "vtkFreeSurferReadersPlotSetNthClassColor $iID $nClass" 
            # -labelfont $::Gui(WLA)
        $owColor config -state disabled
        foreach color $vtkFreeSurferReaders(kValid,lColors) {
            # $owColor add command $color -label $color
            $owColor insert end $color
        }
        $owColor config -state normal
        $owColor select $vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,color)
        
        # We're packing them in two columns (of three columns each).
        set nCol [expr ($nClass % 2) * 3]
        set nRow [expr $nClass / 2]
        grid $lw       -column $nCol            -row $nRow -sticky ew
        grid $owMarker -column [expr $nCol + 1] -row $nRow -sticky ew
        grid $owColor  -column [expr $nCol + 2] -row $nRow -sticky ew
    }
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 0 -weight 1
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 1 -weight 0
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 2 -weight 0
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 3 -weight 1
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 4 -weight 0
    grid columnconfigure $vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig) 5 -weight 0
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotParseHeader
# Parse the header file, using the gdf functions to read it and pull
# data out of it. Returns -1 if there was an error, else it returns an
# ID number for the fsgdf.
#  .ARGS
# path ifnHeader name of header file
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotParseHeader { ifnHeader } {
    global vtkFreeSurferReaders

    # Generate a new ID.
    set ID 0
    while { [lsearch -exact $vtkFreeSurferReaders(gGDF,lID) $ID] != -1 } { incr ID }

    set err [catch {set vtkFreeSurferReaders(gGDF,$ID,object) [vtkFreeSurferReaders(gdfReader) ReadHeader $ifnHeader 1]} errMsg]
    if { $err } {
        puts "vtkFreeSurferReadersPlotParseHeader: Couldn't read header file $ifnHeader (ID = $ID)"
        puts "Error: $errMsg"
        return -1
    }
    # Grab all the data and put it into our TCL object.
    set vtkFreeSurferReaders(gGDF,$ID,title) [vtkFreeSurferReaders(gdfReader) GetTitle]
    set vtkFreeSurferReaders(gGDF,$ID,measurementName) [vtkFreeSurferReaders(gdfReader) GetMeasurementName]
    set vtkFreeSurferReaders(gGDF,$ID,subjectName) [vtkFreeSurferReaders(gdfReader) GetSubjectName]
    set vtkFreeSurferReaders(gGDF,$ID,dataFileName) [vtkFreeSurferReaders(gdfReader) GetDataFileName]

    set vtkFreeSurferReaders(gGDF,$ID,gd2mtx) [vtkFreeSurferReaders(gdfReader) Getgd2mtx]

    set vtkFreeSurferReaders(gGDF,$ID,cClasses) [vtkFreeSurferReaders(gdfReader) GetNumClasses ]
    # If they didn't specify color or marker for the class, use
    # these and increment so all the classes are different.
    set nColor 0
    set nMarker 0
    
    for { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$ID,cClasses) } { incr nClass } {
        set retval [vtkFreeSurferReaders(gdfReader) GetNthClassLabel $nClass]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,label) $retval
        } else {
            puts "WARNING: Could not get ${nClass}th label."
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,label) "Class $nClass"
        }
        set retval [vtkFreeSurferReaders(gdfReader) GetNthClassMarker $nClass]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,marker) $retval
        } else {
            puts "WARNING: Could not get ${nClass}th marker."
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,marker) ""
        }
        # Look for the marker in the array of valid markers. If
        # it's not found, output a warning and set it to the
        # default.
        set n [lsearch -exact $vtkFreeSurferReaders(kValid,lMarkers) \
                   $vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,marker)]
        if { $n == -1 } {
            puts "WARNING: Marker for class $vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,label) was invalid."
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,marker) \
                [lindex $vtkFreeSurferReaders(kValid,lMarkers) $nMarker]
            incr nMarker
            if { $nMarker >= [llength $vtkFreeSurferReaders(kValid,lMarkers)] } {set nMarker 0 }
        }

        set retval [vtkFreeSurferReaders(gdfReader) GetNthClassColor $nClass]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,color) $retval
        } else {
            puts "WARNING: Could not get ${nClass}th colour."
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,color) ""
        }
        # Look for the color in the array of valid color. If
        # it's not found, output a warning and set it to the
        # default.
        set n [lsearch -exact $vtkFreeSurferReaders(kValid,lColors) \
                   $vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,color)]
        if { $n == -1 } {
            puts "WARNING: Color for class $vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,label) was invalid."
            set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,color) \
                [lindex $vtkFreeSurferReaders(kValid,lColors) $nColor]
            incr nColor
            if { $nColor >= [llength $vtkFreeSurferReaders(kValid,lColors)] } { set nColor 0 }
        }

        # This is the reverse lookup for a class label -> index.
        set vtkFreeSurferReaders(gGDF,$ID,classes,$vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,label),index) $nClass

        # Initialize all classes as visible.
        set vtkFreeSurferReaders(gPlot,$ID,state,classes,$nClass,visible) 1
   
    }

    set vtkFreeSurferReaders(gGDF,$ID,cVariables) [vtkFreeSurferReaders(gdfReader) GetNumVariables]
    for { set nVariable 0 } { $nVariable < $vtkFreeSurferReaders(gGDF,$ID,cVariables) } { incr nVariable } {
        set retval [vtkFreeSurferReaders(gdfReader) GetNthVariableLabel $nVariable]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,variables,$nVariable,label) $retval
        } else {
            puts "WARNING: Could not get ${nClass}th label."
            set vtkFreeSurferReaders(gGDF,$ID,variables,$nVariable,label)  "Variable $nVariable"
        }
    }

    set retval [vtkFreeSurferReaders(gdfReader) GetDefaultVariable]
    if {$retval == "null"} {
        puts "WARNING: Could not get default variable."
        set vtkFreeSurferReaders(gGDF,$ID,defaultVariable) $vtkFreeSurferReaders(gGDF,$ID,variables,0,label)
    } else {
       set vtkFreeSurferReaders(gGDF,$ID,defaultVariable) $retval
    }


    set vtkFreeSurferReaders(gGDF,$ID,nDefaultVariable)  [vtkFreeSurferReaders(gdfReader) GetDefaultVariableIndex ]

    set vtkFreeSurferReaders(gGDF,$ID,cSubjects) [vtkFreeSurferReaders(gdfReader) GetNumberOfSubjects]
    for { set nSubject 0 } { $nSubject < $vtkFreeSurferReaders(gGDF,$ID,cSubjects) } { incr nSubject } {
        set retval [vtkFreeSurferReaders(gdfReader) GetNthSubjectID $nSubject]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,id) $retval
        } else {
            puts "WARNING: Could not get ${nSubject}th subject ID."
            set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,id) 0
        }

        set retval [vtkFreeSurferReaders(gdfReader) GetNthSubjectClass $nSubject]
        if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
            set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,nClass) $retval
        } else {
            puts "WARNING: Could not get ${nSubject}th subject class."
            set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,nClass) ""
        }

        for { set nVariable 0 } \
            { $nVariable < $vtkFreeSurferReaders(gGDF,$ID,cVariables) } { incr nVariable } {
                set retval [vtkFreeSurferReaders(gdfReader) GetNthSubjectNthValue $nSubject $nVariable]
                if {$retval != [vtkFreeSurferReaders(gdfReader) GetErrVal]} {
                    set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,variables,$nVariable,value) $retval
                    if {$::Module(verbose)} { puts "Plot parse header: set value to \"$vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,variables,$nVariable,value)\""}
                } else {
                    puts "WARNING: Could not value for ${nSubject}th subject ${nVariable}th variable."
                    set vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,variables,$nVariable,value) 0
                }
        }
        # Initialize all subjects as visible.
        set vtkFreeSurferReaders(gPlot,$ID,state,subjects,$nSubject,visible) 1
    }
     
    # This groups the subjects by the class they are in. For each
    # class, for each subject, if the subject is in the class, assign
    # the subject index to that subject-in-class index.
    for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$ID,cClasses) } { incr nClass } {
        set nSubjInClass 0
        for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$ID,cSubjects) } { incr nSubj } {
            if { [vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $ID $vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubj,nClass)] == $nClass } {
                set vtkFreeSurferReaders(gGDF,$ID,classes,$nClass,subjects,$nSubjInClass,index) $nSubj
                incr nSubjInClass
            }
        }
    }           

    # We now have a header.
    set vtkFreeSurferReaders(gGDF,$ID,bReadHeader) 1

    # Start out trying to find the offset/slope for a class/var.
    set vtkFreeSurferReaders(gPlot,0,state,bTryRegressionLine) 1

    # If we have a window, build the dynamic elements.
    if { [info exists vtkFreeSurferReaders(gWidgets,$ID,bWindowBuilt)] && 
         $vtkFreeSurferReaders(gWidgets,$ID,bWindowBuilt) } {
        vtkFreeSurferReadersGDFPlotBuildDynamicWindowElements $ID
    }

    lappend vtkFreeSurferReaders(gGDF,lID) $ID

    if {$::Module(verbose)} {
        puts "PlotParseHeader: finished reading header, got id $ID"
    }
    return $ID
}










#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotPlotData
# This plots the current data on the graph. It is fast enough that it
# can be called any time the data is changed to completely redraw it
# from scratch.
# .ARGS
# int iID the id of the vertex to plot
# int dID the id of the data file to plot, can be found in $::vtkFreeSurferReaders(gGDF,dataID)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotPlotData { iID dID} {
    global vtkFreeSurferReaders

    # Don't plot if the window isn't built or we don't have data.
    if { ![info exists vtkFreeSurferReaders(gWidgets,$dID,bWindowBuilt)] ||
         ![info exists vtkFreeSurferReaders(gGDF,$dID,bReadHeader)] ||
         !$vtkFreeSurferReaders(gWidgets,$dID,bWindowBuilt) || 
         !$vtkFreeSurferReaders(gGDF,$dID,bReadHeader) } {
        if {$::Module(verbose)} {
            puts "vtkFreeSurferReadersPlotPlotData: the window isn't built or we don't have data"
        }
        return
    }

    if {$::Module(verbose)} { puts "\nvtkFreeSurferReadersPlotPlotData iID = $iID, dID = $dID" }

    # update the info label variable
    set vtkFreeSurferReaders(gPlot,$dID,state,info) "Vertex number $iID"

    set gw $vtkFreeSurferReaders(gWidgets,$dID,gwPlot)

    # Set the x axis title to the label of the current variable.
    set labelid [$vtkFreeSurferReaders(gWidgets,$dID,owVar) index select]
    $gw axis configure x \
        -title $vtkFreeSurferReaders(gGDF,$dID,variables,$labelid,label)

    # set up autorange
    $gw axis configure x -autorange 1

    # Remove all the elements and markers from the graph.
    set lElements [$gw element names *]
    foreach element $lElements {
        if {$::Module(verbose)} { puts "deleting element $element" }
        $gw element delete $element
    }
    set lMarkers [$gw marker names *]
    foreach marker $lMarkers {
        if {$::Module(verbose)} { puts "deleting marker $marker" }
        $gw marker delete $marker
    }
    
    # If we have no points, return.
    if { ![info exists vtkFreeSurferReaders(gPlot,$iID,state,lPoints)] || 
         [llength $vtkFreeSurferReaders(gPlot,$iID,state,lPoints)] == 0 } {
        if {$::Module(verbose)} {
            puts "Warning: no points to plot for vertex $iID, data $dID!"
            puts "Trying to read in data..."
        }
        vtkFreeSurferReadersPlotBuildPointList $iID vtkFreeSurferReaders(plot,$dID,scalars)
        # return here, as plot build point list will call plot plot data
        return
    }

    # Depending on our legend mode, we'll draw by class or subject.
    if { $vtkFreeSurferReaders(gPlot,$dID,state,legend) == "class" } {
   
        # For each class, for each subject, if the subject's class is
        # the same as the current class, get its data points and add
        # them to a list. Then draw the entire list of data in the
        # class's color/marker. If the class is hidden, set the color
        # to white (so it shows up white in the legend) and hide the
        # element.
        set xmin 0
        set xmax 0
        set ymin 0
        set ymax 0
        for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$dID,cClasses) } { incr nClass } {
            
            set lData {}
            set nSubjInClass 0

            if {$::Module(verbose)} { puts "Starting loop: nClass = $nClass" }

            for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$dID,cSubjects) } { incr nSubj } {
                set classIndex [vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $dID $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,nClass)]
                if {$::Module(verbose)} {
                    puts "by class: nSubj = $nSubj, classIndex = $classIndex" 
                }
                if { $classIndex == $nClass } {
                    
                    if { $vtkFreeSurferReaders(gPlot,$iID,state,pointsChanged) } {
                        vtkFreeSurferReadersPlotCalculateSubjectMeasurement $iID $nSubj
                    }
                    
                    set vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,variables,$vtkFreeSurferReaders(gPlot,$dID,state,nVariable),value)

                    
                    lappend lData $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                    lappend lData $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                    if {$xmin == $xmax == $ymin == $ymax} {
                        set xmin $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                        set xmax $xmin
                        set ymin $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                        set ymax $ymin
                    } else {
                        if {$vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) < $xmin} {
                            set xmin $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                        } else {
                            if {$vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) > $xmax} {
                                set xmax $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                            }
                        }
                        if {$vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement) < $ymin} {
                            set ymin $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                        } else {
                            if {$vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement) > $ymax} {
                                set ymax $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                            }
                        }
                    }
                }
            }
            
            if { $vtkFreeSurferReaders(gPlot,$dID,state,classes,$nClass,visible) } {
                set bHide 0
                set color $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,color)
            } else {
                set bHide 1
                set color white
            }
            if {$::Module(verbose)} { puts "creating element $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)" }
            $gw element create $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label) \
                -data $lData \
                -symbol $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,marker) \
                -color $color -linewidth 0 -outlinewidth 1 -hide $bHide \
                -activepen activeElement
            
            if {$::Module(verbose)} {
                puts "By classes: data = $lData"
            }
        }
        if {$::Module(verbose)} {
                puts "By classes: xmin $xmin, xmax $xmax, ymin $ymin, ymax $ymax"
        }
        $gw axis configure x -min $xmin -max $xmax
        $gw axis configure y -min $ymin -max $ymax
        
    } else {
        
        # For each subject, if the points have changed, calculate the #
        # measurements. Get the variable value. If the subject is visible,
        # set # the hide flag to 0 and the color to the subject's class
        # color, else # set the hide flag to 1 and set the color to
        # white. Create the # element.
        set xmin 0
        set xmax 0
        set ymin 0
        set ymax 0
        for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$dID,cSubjects) } { incr nSubj } {
            
            if { $vtkFreeSurferReaders(gPlot,$iID,state,pointsChanged) } {
                vtkFreeSurferReadersPlotCalculateSubjectMeasurement $iID $nSubj
            }
            # set this to the index of the variable
            set vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,variable) \
                $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,variables,$vtkFreeSurferReaders(gPlot,$dID,state,nVariable),value)
            set classIndex $vtkFreeSurferReaders(gGDF,$dID,classes,$vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,nClass),index)
            if {  $vtkFreeSurferReaders(gPlot,$dID,state,subjects,$nSubj,visible) } {
                set bHide 0
                set color $vtkFreeSurferReaders(gGDF,$dID,classes,$classIndex,color)
            } else {
                set bHide 1
                set color white
            }
            if {$xmin == $xmax  == $ymin == $ymax} {
                set xmin $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                set xmax $xmin
                set ymin $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                set ymax $ymin
            } else {
                if {$vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) < $xmin} {
                    set xmin $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                } else {
                    if {$vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) > $xmax} {
                        set xmax $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable)
                    }
                }
                if {$vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement) < $ymin} {
                    set ymin $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                } else {
                    if {$vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement) > $ymax} {
                        set ymax $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)
                    }
                }
            }
            if {$::Module(verbose)} { puts "creating element $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,id)" }
            $gw element create $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,id) \
                -data [list $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) \
                           $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)] \
                -symbol $vtkFreeSurferReaders(gGDF,$dID,classes,$classIndex,marker) \
                -color $color -linewidth 0 -outlinewidth 1 -hide $bHide \
                -activepen activeElement
            if {$::Module(verbose)} { puts "Created gw  element for subj $nSubj: $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)" }
        }
        $gw axis configure x -min $xmin -max $xmax
        $gw axis configure y -min $ymin -max $ymax
    }

    # If we're trying to draw the regression line, for each class, if
    # the class is visible, get the offset and slope for that class
    # and the current variable. This depends on the point we're
    # drawing, so get the avg of all the points if necessary. Then
    # make a marker calculating two points on the line. if
    # gdfOffsetSlope() fails, set the bTryRegressionLine flag to
    # false, so we won't try drawing it again.
    if { $vtkFreeSurferReaders(gPlot,$iID,state,bTryRegressionLine) } {

        # init the variables
        for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$dID,cClasses) } { incr nClass } {
            if { $vtkFreeSurferReaders(gPlot,$dID,state,classes,$nClass,visible) } {
            
                set thisClass $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)
                if {$::Module(verbose)} {
                    puts "init vars thisClass = $thisClass"
                }
                set offsetArray($thisClass,N) 0
                set offsetArray($thisClass,sumx) 0
                set offsetArray($thisClass,sumy) 0
                set offsetArray($thisClass,sumxx) 0
                set offsetArray($thisClass,sumxy) 0
                
                set slopeArray($thisClass,N) 0
                set slopeArray($thisClass,sumx) 0
                set slopeArray($thisClass,sumy) 0
                set slopeArray($thisClass,sumxx) 0
                set slopeArray($thisClass,sumxy) 0
            } 
        }
        # go through the subjects, adding the appropriate values on a per class basis
        for {set sid 0} {$sid < $vtkFreeSurferReaders(gGDF,$dID,cSubjects)} {incr sid} {
            set subjClass $vtkFreeSurferReaders(gGDF,$dID,subjects,$sid,nClass)
            if {$vtkFreeSurferReaders(gPlot,$dID,state,classes,[vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $dID $subjClass],visible)} {
                # get the subject's variable value and measurement
                set x $vtkFreeSurferReaders(gGDF,$dID,subjects,$sid,variables,$labelid,value)
                # measurement
                set y [lindex [lindex $vtkFreeSurferReaders(gPlot,$iID,state,lPoints) $sid] 0]

            
                incr offsetArray($subjClass,N)
                set offsetArray($subjClass,sumx) [expr $offsetArray($subjClass,sumx) + $x]
                set offsetArray($subjClass,sumy) [expr $offsetArray($subjClass,sumy) + $y]
                set offsetArray($subjClass,sumxx) [expr $offsetArray($subjClass,sumxx) + ($x * $x)]
                set offsetArray($subjClass,sumxy) [expr $offsetArray($subjClass,sumxy) + ($x * $y)]

                incr slopeArray($subjClass,N) 
                set slopeArray($subjClass,sumx) [expr $slopeArray($subjClass,sumx) + $x]
                set slopeArray($subjClass,sumy) [expr $slopeArray($subjClass,sumy) + $y]
                set slopeArray($subjClass,sumxx) [expr $slopeArray($subjClass,sumxx) + ($x * $x)]
                set slopeArray($subjClass,sumxy) [expr $slopeArray($subjClass,sumxy) + ($x * $y)]
            }
        }

        # if it's the same slope for all lines, calc that first
        if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "doss"} {
            set slopeArray(N) 0
            set slopeArray(sumx) 0
            set slopeArray(sumy) 0
            set slopeArray(sumxx) 0
            set slopeArray(sumxy) 0
            # sum together values from all classes
            for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$dID,cClasses) } { incr nClass } {
                if { $vtkFreeSurferReaders(gPlot,$dID,state,classes,$nClass,visible) }  {
            
                    set thisClass $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)
                    set slopeArray(N) [expr $slopeArray(N) + $slopeArray($thisClass,N)]
                    set slopeArray(sumx) [expr $slopeArray(sumx) + $slopeArray($thisClass,sumx)]
                    set slopeArray(sumy) [expr $slopeArray(sumy) + $slopeArray($thisClass,sumy)]
                    set slopeArray(sumxx) [expr $slopeArray(sumxx) + $slopeArray($thisClass,sumxx)]
                    set slopeArray(sumxy) [expr $slopeArray(sumxy) + $slopeArray($thisClass,sumxy)]
                }
            }
            if {$slopeArray(N) > 0} {
                set slope [expr (($slopeArray(N) * $slopeArray(sumxy)) - ($slopeArray(sumx) * $slopeArray(sumy))) / ( ($slopeArray(N) * $slopeArray(sumxx)) - ($slopeArray(sumx)*$slopeArray(sumx)))]
                if {$::Module(verbose)} {
                    puts "Slope for all classes: N = $slopeArray(N), sumx = $slopeArray(sumx), sumy = $slopeArray(sumy), sumxx = $slopeArray(sumxx), sumxy = $slopeArray(sumxy)"
                    puts "Got slope for all classes : $slope"
                }
            } else {
                set slope 0
            }
        }

        # now calculate the slope and offset for all classes
        for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$dID,cClasses) } { incr nClass } {
            if { $vtkFreeSurferReaders(gPlot,$dID,state,classes,$nClass,visible) }  {
                set thisClass $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)

                # if it's a different slope for each class, calc it, otherwise use the already calcd slope
                if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "dods"} {
                    if {$slopeArray($thisClass,N) > 0} {
                        set slope [expr (($slopeArray(thisClass,N) * $slopeArray($thisClass,sumxy)) - ($slopeArray($thisClass,sumx) * $slopeArray($thisClass,sumy))) / ( ($slopeArray($thisClass,N) * $slopeArray($thisClass,sumxx)) - ($slopeArray($thisClass,sumx)*$slopeArray($thisClass,sumx)))] 
                        if {$::Module(verbose)} {
                            puts "Got slope $slope for class $thisClass"
                        }
                    } else {
                        set slope 0
                    }
                } 

                if {$offsetArray($thisClass,N) > 0} {
                    set offset  [expr (($offsetArray($thisClass,sumxx) * $offsetArray($thisClass,sumy)) - ($offsetArray($thisClass,sumx) * $offsetArray($thisClass,sumxy)))/(($offsetArray($thisClass,N) * $offsetArray($thisClass,sumxx)) - ($offsetArray($thisClass,sumx) * $offsetArray($thisClass,sumx)))]
                    if {$::Module(verbose)} {
                            puts "Got offset $offset for class $thisClass"
                        }
                    
                } else {
                    puts "Warning: offset for $thisClass hasn't got enough good points: $offsetArray($thisClass,N)"
                    set offset 0
                }

                # now calculate the y values at the min and max x of the graph, and make a line between them
                set x1 $xmin
                set y1 [expr ($slope * $x1) + $offset]
                set x2 $xmax
                set y2 [expr ($slope * $x2) + $offset]
                if {$::Module(verbose)} { puts "New way: class $thisClass, offset = $offset, slope = $slope (xmin $xmin, xmax $xmax)"} 
                $gw marker create line \
                    -coords [list $x1 $y1 $x2 $y2] \
                    -outline $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,color) \
                    -dashes {5 5}
            }
        }



        if {0} { 

            puts "\n\n\nTrying the old way"
            if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "doss"} {
                puts "WARNING: will not be calculating regression line for this different offset, same slope case."
            }

            for  { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$dID,cClasses) } { incr nClass } {
        
                if {$::Module(verbose)} { 
                    puts "Regression line: nClass = $nClass"
                }
                if { $vtkFreeSurferReaders(gPlot,$dID,state,classes,$nClass,visible) } {
                    
                    set nVar $vtkFreeSurferReaders(gPlot,$dID,state,nVariable)
                    set nSubjs $vtkFreeSurferReaders(gGDF,$dID,cSubjects)
                    set thisClass $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)
                    
                    set cGood 0
                    if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "dods"} {                    
                        set sumxy 0
                        set sumx 0
                        set sumy 0
                        set sumxx 0
                    } 
                    # for each subject 
                    for {set sid 0} {$sid < $vtkFreeSurferReaders(gGDF,$dID,cSubjects)} {incr sid} {
                        # get the class 
                        if {$vtkFreeSurferReaders(gGDF,$dID,subjects,$sid,nClass) == $thisClass} {
                            # if it matches the one we're looking for
                            # get the subject's variable value and measurement
                            set x $vtkFreeSurferReaders(gGDF,$dID,subjects,$sid,variables,$labelid,value)
                            # measurement
                            set y [lindex [lindex $vtkFreeSurferReaders(gPlot,$iID,state,lPoints) $sid] 0]
                            
                            # add it to the offset/slope calc for this class
                            if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "dods"} {
                                set sumxy [expr $sumxy + ($x * $y)]
                                set sumx [expr $sumx + $x]
                                set sumy [expr $sumy + $y]
                                set sumxx [expr $sumxx + ($x * $x)]
                            }
                            incr cGood
                            # if {$::Module(verbose)} { puts "subject id = $sid x = $x y = $y" }
                        }
                    }
                
                    if { $cGood > 0 } {
                    
                        if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "dods"} {
                            if {$::Module(verbose)} { 
                                puts "class $thisClass, sumx = $sumx, sumy = $sumy, cGood = $cGood, sumxx = $sumxx, sumxy = $sumxy"
                            }
                            set slope [expr (($cGood * $sumxy) - ($sumx * $sumy)) / ( ($cGood * $sumxx) - ($sumx*$sumx))]
                            set offset [expr (($sumxx * $sumy) - ($sumx * $sumxy))/(($cGood * $sumxx) - ($sumx * $sumx))]
                            
                            set x1 $xmin
                            set y1 [expr ($slope * $x1) + $offset]
                            set x2 $xmax
                            set y2 [expr ($slope * $x2) + $offset]
                            if {$::Module(verbose)} { puts "Original: class $thisClass, offset = $offset, slope = $slope (xmin $xmin, xmax $xmax)"} 
                            $gw marker create line \
                                -coords [list $x1 $y1 $x2 $y2] \
                                -outline $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,color) \
                                -dashes {2 2}
                        }
                    } else {
                        if {$::Module(verbose)} { 
                            puts "Not enough good points $cGood"
                        }
                    }
                }

            }
        } 
        # end of trying it the old way
            
        if { $vtkFreeSurferReaders(gPlot,$iID,state,bTryRegressionLine) == 0 } { 
            break 
        }
    }
    set vtkFreeSurferReaders(gPlot,$iID,state,pointsChanged) 0
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotCalculateSubjectMeasurement
# Accesses and calculates the (averaged if necessary) measurment
# values at the current point(s). Stores the values in gPlot.
# .ARGS
# int iID the id of the point
# string inSubject the subject to calculate for
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotCalculateSubjectMeasurement { iID inSubject } {
    global vtkFreeSurferReaders

    # Get the average of the points we've been given.
    set meas 0
    set cGood 0
    set notGood 0

    set meas [lindex [lindex $vtkFreeSurferReaders(gPlot,$iID,state,lPoints) $inSubject] 0]

if {0} {

    foreach lPoint $vtkFreeSurferReaders(gPlot,$iID,state,lPoints) {
        set meas [expr $meas + [lindex $lPoint 0]]
        incr cGood
        if {0} {
        if {$lPoint != ""} {
            if {$::Module(verbose)} { puts "lPoint = $lPoint" }
            scan $lPoint "%d %d %d" x y z
            set lResults [vtkFreeSurferReadersGetNthSubjectMeasurement $vtkFreeSurferReaders(gGDF,$iID,object) \
                              $inSubject $x $y $z]
            set err [lindex $lResults 0]
            if { 0 == $err } {
                set meas [expr $meas + [lindex $lResults 1]]
                incr cGood
            }
        } else {
            incr notGood
        }
        }
    }
    
    if { $cGood > 0 } {
        set meas [expr $meas / $cGood.0]
    }

}

    if {$::Module(verbose)} { 
#        puts "CalculateSubjectMeasurement: meas = $meas"
    }
    # Store the values in gPlot.
    set vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$inSubject,measurement) $meas
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotHilightElement
# Hilight/UnhilightElement works on an element by name (which could be
# a subject or class, depending on viewing mode). It will
# select/unselect the element name in the legend and change the
# drawing pen of the element in the graph, which if activated draws it
# with a red circle around it.
# .ARGS
# int iID the id of the window
# int iElement element to hilight
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotHilightElement { iID iElement } {
    global vtkFreeSurferReaders
    $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) legend activate $iElement
    $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) element activate $iElement
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotUnhilightElement
# Hilight/UnhilightElement works on an element by name (which could be
# a subject or class, depending on viewing mode). It will
# select/unselect the element name in the legend and change the
# drawing pen of the element in the graph, which if activated draws it
# with a red circle around it.
# .ARGS
# int iID the id of the window
# int iElement element to unhilight
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotUnhilightElement { iID iElement } {
    global vtkFreeSurferReaders
    $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) legend deactivate $iElement
    $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) element deactivate $iElement
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotToggleVisibility
# Shows or hide an element by name, in subject or class mode. Changes
# the value of the gPlot visibility flag.
# .ARGS
# int iID the id of the window
# int iElement the element to show or hide
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotToggleVisibility { iID iElement } {
    global vtkFreeSurferReaders

    # If we're in subject legend mode, the legend label is a subject
    # name. Get the subject index and toggle its visibility. If we're in
    # class legend mode, the legend label is a class name, so get the
    # class index and toggle its visibility.
    if { $vtkFreeSurferReaders(gPlot,$iID,state,legend) == "subject" } {
        set nSubj [vtkFreeSurferReadersGDFPlotGetSubjectIndexFromID $iID $iElement]
        if { $vtkFreeSurferReaders(gPlot,$iID,state,subjects,$nSubj,visible) } {
            set vtkFreeSurferReaders(gPlot,$iID,state,subjects,$nSubj,visible) 0
        } else {
            set vtkFreeSurferReaders(gPlot,$iID,state,subjects,$nSubj,visible) 1
        }
    } else {
        set nClass [vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $iID $iElement]
        if { $vtkFreeSurferReaders(gPlot,$iID,state,classes,$nClass,visible) } {
            set vtkFreeSurferReaders(gPlot,$iID,state,classes,$nClass,visible) 0
        } else {
            set vtkFreeSurferReaders(gPlot,$iID,state,classes,$nClass,visible) 1
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotUnfocusElement
# Unfocus is called to 'mouseover' an element. It
# Unhilights an element and puts or removes the subject name
# in a text marker in the graph.
# .ARGS
# int iID the id of the window
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotUnfocusElement { iID } {
    global vtkFreeSurferReaders

    # If we have a focused element, unhighlight it, set the
    # highlighted element name to null, and delete the hover text
    # marker.
    if { [info exists vtkFreeSurferReaders(gPlot,$iID,state,hiElement)] && \
             "$vtkFreeSurferReaders(gPlot,$iID,state,hiElement)" != "" } {
        vtkFreeSurferReadersGDFPlotUnhilightElement $iID $vtkFreeSurferReaders(gPlot,$iID,state,hiElement)
        set vtkFreeSurferReaders(gPlot,$iID,state,hiElement) ""
        $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) marker delete hover
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotFocusElement
# Focus/Unfocus is called to 'mouseover' an element. It
# Hilight/Unhilights an element and puts or removes the subject name
# in a text marker in the graph.
# .ARGS
# int iID the id of the window
# int iElement the element to unfocus
# string inSubjInClass
# int iX
# int iY
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotFocusElement { iID iElement inSubjInClass iX iY } {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts "GDFPlotFocusElement: iID $iID iElement $iElement inSubjInClass $inSubjInClass iX $iX iY $iY"
    }
    # Set the highlighted element name and highlight the element.
    set vtkFreeSurferReaders(gPlot,$iID,state,hiElement) $iElement
    vtkFreeSurferReadersGDFPlotHilightElement $iID $vtkFreeSurferReaders(gPlot,$iID,state,hiElement)

    # Need to get the subject name. If we're in subject mode, this is
    # just the element name, otherwise we're getting the class name in
    # the element name so get the class index, then use that and the
    # parameter we got (index of the data point, also the
    # subject-in-class index) to get the subject index, and then the
    # subject name.
    if { $vtkFreeSurferReaders(gPlot,$iID,state,legend) == "subject" } {
        set sId $iElement
    } else {
        set nClass [vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $iID $iElement]
        set nSubj $vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,subjects,$inSubjInClass,index)
        set sId $vtkFreeSurferReaders(gGDF,$iID,subjects,$nSubj,id)
    }
    $vtkFreeSurferReaders(gWidgets,$iID,gwPlot) marker create text \
        -name hover -text $sId -anchor nw \
        -coords [list $iX $iY]
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotFindMousedElement
# Finds the element under the mouse.
# .ARGS
# int iID the id of the window
# int iX
# int iY
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotFindMousedElement { iID iX iY } {
    global vtkFreeSurferReaders
    set bFound [$vtkFreeSurferReaders(gWidgets,$iID,gwPlot) element closest $iX $iY aFound -halo 10]
    if { $bFound } {
        return [list $aFound(name) $aFound(index) $aFound(x) $aFound(y)]
    }
    return ""
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotGetSubjectIndexFromID
# Converts from subject or class names to indicies.
# .ARGS
# int iID the id of the window
# int iSubjID the subject to convert from
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotGetSubjectIndexFromID { iID iSubjID } {
    global vtkFreeSurferReaders
    for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$iID,cSubjects) } { incr nSubj } {
        if { "$iSubjID" == "$vtkFreeSurferReaders(gGDF,$iID,subjects,$nSubj,id)" } { 
            return $nSubj 
        }
    }
    return -1
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel
# 
# .ARGS
# int iID the id of the window
# string iLabel 
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel { iID iLabel } {
    global vtkFreeSurferReaders
    for { set nClass 0 } { $nClass < $vtkFreeSurferReaders(gGDF,$iID,cClasses) } { incr nClass } {
        if { "$iLabel" == "$vtkFreeSurferReaders(gGDF,$iID,classes,$nClass,label)" } { 
            return $nClass 
        }
    }
    return -1
}

# GDF callbacks.

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBCloseWindow
# 
# .ARGS
# int iID the id of the window
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBCloseWindow { iID } {
    global vtkFreeSurferReaders
    set vtkFreeSurferReaders(gWidgets,$iID,bWindowBuilt) 0
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBLegendEnter
# Highlight the current element
# .ARGS
# int iID window id
# int igw graph widget id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBLegendEnter { iID igw } {
    vtkFreeSurferReadersGDFPlotHilightElement $iID [$igw legend get current]
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBLegendLeave
# Un-highlight the current element.
# .ARGS
# int iID window id
# int igw graph widget id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBLegendLeave { iID igw } {
    vtkFreeSurferReadersGDFPlotUnhilightElement $iID [$igw legend get current]
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBLegendClick
# Toggle the visibiity of the current element, then plot again
# .ARGS
# int iID window id
# int igw graph widget id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBLegendClick { iID igw } {
    vtkFreeSurferReadersGDFPlotToggleVisibility $iID [$igw legend get current]
    vtkFreeSurferReadersPlotPlotData $iID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBGraphMotion
# Update the graph after mouse motion.
# .ARGS
# int iID window id
# int igw graph widget id
# int iX
# int iY
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBGraphMotion { iID igw iX iY } {
    vtkFreeSurferReadersGDFPlotUnfocusElement $iID
    set lResult [vtkFreeSurferReadersGDFPlotFindMousedElement $iID $iX $iY]
    set element [lindex $lResult 0]
    if { "$element" != "" } { 
        set index [lindex $lResult 1]
        set x [lindex $lResult 2]
        set y [lindex $lResult 3]
        vtkFreeSurferReadersGDFPlotFocusElement $iID $element $index $x $y
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotRead
# If the graphing library has been loaded, read the header of a graph. Returns
# the ID of the plot.
# .ARGS
# path ifnHeader the name of the header file
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotRead { ifnHeader } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        return -1
    }
    # read the header
    set vtkFreeSurferReaders(gGDF,dataID) [vtkFreeSurferReadersGDFPlotParseHeader $ifnHeader]


    
    return $vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotPrint
# Print information about the header.
# .ARGS
# int iID the id of the plot to print out
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotPrint { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersGDFPlotPrint: gb lib is not loaded."
        return 
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    gdfPrintStdout $vtkFreeSurferReaders(gGDF,$iID,object)
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotShowWindow
# Show or hide the window. If it hasn't been built, builds the window
# first.
# .ARGS
# int iID the id of the plot window
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotShowWindow { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotShowWindow: gb lib not loaded."
        return 
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    if { ![info exists vtkFreeSurferReaders(gWidgets,$iID,bWindowBuilt)] ||
         !$vtkFreeSurferReaders(gWidgets,$iID,bWindowBuilt) } {
        vtkFreeSurferReadersGDFPlotBuildWindow $iID
    }
    wm deiconify $vtkFreeSurferReaders(gWidgets,$iID,wwTop)
    if { [info exists vtkFreeSurferReaders(gWidgets,$iID,state,window,geometry)] } {
        wm geometry $vtkFreeSurferReaders(gWidgets,$iID,wwTop) $vtkFreeSurferReaders(gWidgets,$iID,state,window,geometry)
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotHideWindow
# Hides the plot window via a wm withdraw.
# .ARGS
# int iID the identifier of the window
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotHideWindow { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { return }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    if { [info exists vtkFreeSurferReaders(gWidgets,$iID,wwTop)] } {
        set vtkFreeSurferReaders(gWidgets,$iID,state,window,geometry) \
        [wm geometry $vtkFreeSurferReaders(gWidgets,$iID,wwTop)]
        wm withdraw $vtkFreeSurferReaders(gWidgets,$iID,wwTop)
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetVariable
# Set the current variable.
#  .ARGS
# int iID window identifier
# int vID the vertex identifier
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetVariable { iID {vID 0} } {
# inVariable
    global vtkFreeSurferReaders 
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetVariable: gb lib not loaded."
        return 
    }
    if {[catch {set inVariable [$vtkFreeSurferReaders(gWidgets,$iID,owVar) index select]} errmsg] == 1} {
        puts "ID $iID not found: $errmsg"
        return
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }

    set vtkFreeSurferReaders(gPlot,$iID,state,nVariable) $inVariable

    # need the vertex id here
    vtkFreeSurferReadersPlotPlotData $vID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetMode
# Set legend mode to subject or class.
#  .ARGS
# int iID window identifier
# string iMode legend mode, valid values are subject and class
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetMode { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetMode: gb lib not loaded."
        return 
    }
    set iMode  [$vtkFreeSurferReaders(gWidgets,$iID,owMode) get select]
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    if { $iMode != "subject" && $iMode != "class" } { 
        return 
    }

    set vtkFreeSurferReaders(gPlot,$iID,state,legend) $iMode

    vtkFreeSurferReadersPlotPlotData $iID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetNthClassMarker
# Set display settings for a class.
#  .ARGS
# int iID window identifier
# string inClass class to see settings for 
# string iMarker value to set, needs to be in the list vtkFreeSurferReaders(kValid,lMarkers)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetNthClassMarker { iID inClass } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetNthClassMarker: gb lib not loaded."
        return 
    }

    set iMarker [$vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig).owMarker$inClass get select]
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    if { $inClass < 0 || $inClass >= $vtkFreeSurferReaders(gGDF,$iID,cClasses) } { 
        return 
    }
    if { [lsearch -exact $vtkFreeSurferReaders(kValid,lMarkers) $iMarker] == -1 } { 
        return 
    }

    set vtkFreeSurferReaders(gGDF,$iID,classes,$inClass,marker) $iMarker

    vtkFreeSurferReadersPlotPlotData $iID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetNthClassColor
# 
# .ARGS
# int iID window id
# string inClass class to set the colour for, must be an id between 0 and vtkFreeSurferReaders(gGDF,$iID,cClasses)
# string iColor colour to set nth class to, must appear in vtkFreeSurferReaders(kValid,lColors) 
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetNthClassColor { iID inClass } {
    global vtkFreeSurferReaders 
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetNthClassColor: gb lib not loaded."
        return 
    }

    set iColor [$vtkFreeSurferReaders(gWidgets,$iID,fwClassConfig).owColor$inClass get select]
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    if { $inClass < 0 || $inClass >= $vtkFreeSurferReaders(gGDF,$iID,cClasses) } { 
        return 
    }
    if { [lsearch -exact $vtkFreeSurferReaders(kValid,lColors) $iColor] == -1 } { 
        return 
    }

    set vtkFreeSurferReaders(gGDF,$iID,classes,$inClass,color) $iColor

    vtkFreeSurferReadersPlotPlotData $iID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetPoint
# Choose a point to be displayed. Either choose one point or make a
# point list to be averaged.
#  .ARGS
# int iID window id
# int iX point location in x
# int iY point location in y
# int iZ point location in z
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetPoint { iID iX iY iZ } {
    global vtkFreeSurferReaders 
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetPoint: gb lib not loaded."
        return 
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    vtkFreeSurferReadersPlotBeginPointList $iID
    vtkFreeSurferReadersPlotAddPoint $iID $iX $iY $iZ
    vtkFreeSurferReadersPlotEndPointList $iID
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotBeginPointList
# Resets the point list to empty.
# .ARGS
# int iID the id of the vertex
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotBeginPointList { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotBeginPointList: gb lib not loaded."
        return 
    }
# don't do this, as the lID list only lists the data file ids
    if {0} {
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    }
    set vtkFreeSurferReaders(gPlot,$iID,state,lPoints) {}
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotAddPoint
# Adds the point to the point list for the given window
# .ARGS
# int iID the id of the vertex
# int iX the x coordinate of the point to add
# int iY the y coordinate of the point to add
# int iZ the z coordinate of the point to add
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotAddPoint { iID iX iY iZ } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotAddPoint: gb lib not loaded."
        return 
    }
# this checks theplot window id, not necessary
    if {0} {
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    }
    lappend vtkFreeSurferReaders(gPlot,$iID,state,lPoints) [list $iX $iY $iZ]
    set vtkFreeSurferReaders(gPlot,$iID,state,pointsChanged) 1
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotEndPointList
# Plots the data after the list is done
# .ARGS
# int iID the id of the vertex to plot
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotEndPointList { iID } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotEndPointList: gb lib not loaded."
        return 
    }
# this is checking the plot window, rather than the vertex
    if {0} {
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    }
    vtkFreeSurferReadersPlotPlotData $iID $::vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSetInfo
# Set the info string displayed under the graph.
# .ARGS
# int iID the id of the window
# string isInfo the input information string
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSetInfo { iID isInfo } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSetInfo: gb lib is not loaded."
        return 
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    set vtkFreeSurferReaders(gPlot,$iID,state,info) $isInfo
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSaveToTable
# Save the currently plotted data to a table.
# .ARGS
# int iID the id of the window
# path ifnTable the name of the table file to save to
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSaveToTable { iID ifnTable } {
    global vtkFreeSurferReaders
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }

    set fp 0
    set err [catch {set fp [open $ifnTable w+]}]
    if { $err || $fp == 0 } {
        puts "vtkFreeSurferReadersPlotSaveToTable: Couldn't write file $ifnTable."
        return
    }
    
    puts $fp "Graph: $vtkFreeSurferReaders(gGDF,$iID,title)"
    puts $fp "Data: $vtkFreeSurferReaders(gGDF,$iID,dataFileName)"
    puts $fp "Variable: $vtkFreeSurferReaders(gGDF,$iID,variables,$vtkFreeSurferReaders(gPlot,$iID,state,nVariable),label)"
    puts $fp "Measurement: $vtkFreeSurferReaders(gGDF,$iID,measurementName)"
    puts $fp "subject id, class id, variable value, measurement value"
    puts $fp "------------"
    for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$iID,cSubjects) } { incr nSubj } {

        set subjLabel $vtkFreeSurferReaders(gGDF,$iID,subjects,$nSubj,id)
        set classLabel $vtkFreeSurferReaders(gGDF,$iID,classes,$vtkFreeSurferReaders(gGDF,$iID,subjects,$nSubj,nClass),label)
        set var $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,variable)
        set meas $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)

        puts $fp "$subjLabel $classLabel $var $meas"
    }
    puts $fp "------------"
    puts ""

    close $fp
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotSaveToPostscript
# Save the current plot graphic to a postscript file.
# .ARGS
# int iID the id of the window
# path ifnPS the name of the postscript file to write into
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotSaveToPostscript { iID ifnPS } {
    global vtkFreeSurferReaders
    if { !$vtkFreeSurferReaders(gbLibLoaded) } { 
        puts "vtkFreeSurferReadersPlotSaveToPostscript: gb lib not loaded."
        return 
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "ID $iID not found"
        return 
    }
    set err [catch {$vtkFreeSurferReaders(gWidgets,$iID,gwPlot) postscript output $ifnPS} sResult]
    if { $err } {
        puts "vtkFreeSurferReadersPlotSaveToPostscript: Could not save postscript file: $sResult"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetPlotFileName
# The filename is set elsehwere, in variable vtkFreeSurferReaders(PlotFileName)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetPlotFileName {} {
    global vtkFreeSurferReaders Module

    if {$Module(verbose) == 1} {
        puts "FreeSurferReaders Plot filename: $vtkFreeSurferReaders(PlotFileName)"
    }
   # vtkFreeSurferReaders(gdfReader) SetHeaderFileName $vtkFreeSurferReaders(PlotFileName)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotApply
# Read in the plot specified. 
# .ARGS
# str mid model id
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotApply { mid } {
    global vtkFreeSurferReaders Volume

    puts "About to read $vtkFreeSurferReaders(PlotFileName)..."

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersPlotApply: starting"
    }
    vtkFreeSurferReadersGDFInit
    set vtkFreeSurferReaders(gGDF,dataID) [vtkFreeSurferReadersPlotParseHeader $vtkFreeSurferReaders(PlotFileName)]

    # now read the data
    set datafilename [vtkFreeSurferReaders(gdfReader) GetDataFileName]
    # if it's a bfloat or a bshort, pass in the .bhdr version to the reader apply
    if {[file extension $datafilename] == ".bfloat" || 
        [file extension $datafilename] == ".bshort"} {
        # take off the first slice number if it's present
        set datafilename [string trimright [file rootname $datafilename] "_0"].bhdr
    }
    if {[file pathtype $datafilename] == "relative"} {
         set datafilename [file normalize [file join [file dirname $vtkFreeSurferReaders(PlotFileName)] $datafilename]]
    }
    set vtkFreeSurferReaders(VolumeFileName) $datafilename
    set stem [file rootname $vtkFreeSurferReaders(VolumeFileName)]
    set Volume(name) [vtkFreeSurferReaders(gdfReader) GetTitle]

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersGDFPlotRead: read the header $vtkFreeSurferReaders(PlotFileName), now about to try reading the data file $vtkFreeSurferReaders(VolumeFileName)"
    }

    # this isn't working so well, so save the scalars as a vtkDataArray    
    # set dataID [vtkFreeSurferReadersApply]
    if {[file extension $datafilename] == ".bhdr"} {
    catch "mybreader Delete"
    vtkBVolumeReader mybreader
    set ::Gui(progressText) "Reading b data volume"
    mybreader AddObserver StartEvent MainStartProgress
    mybreader AddObserver ProgressEvent "MainShowProgress mybreader"
    mybreader AddObserver EndEvent MainEndProgress
    
    mybreader SetFileName $vtkFreeSurferReaders(VolumeFileName)
    mybreader SetFilePrefix $stem
    mybreader SetStem $stem
    set retval [mybreader ReadVolumeHeader]
    if {$retval == 0} {
        puts "ERROR reading  $vtkFreeSurferReaders(VolumeFileName)"
        mybreader Delete
        return -1
    }
    set vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) [mybreader ReadVolumeData]
    set scalarsVar vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars)
    mybreader Delete
    MainEndProgress
    } else {
       if {[file extension $datafilename] == ".mgz" ||
        [file extension $datafilename] == ".mgh"} {
        catch "reader Delete"
        vtkITKArchetypeImageSeriesVectorReader reader
        reader SetArchetype $datafilename
        reader SetOutputScalarTypeToNative
        reader SetDesiredCoordinateOrientationToNative
        reader Update
        if {$::Module(verbose)} { puts "Read the data file $datafilename" }
        set imageData [reader GetOutput]
        set numPoints [$imageData GetNumberOfPoints]
        if {$::Module(verbose)} { puts "Got number of points $numPoints" }
        set vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) [[$imageData GetPointData] GetScalars]
        # need to register it to hold onto it over tcl calls
        $vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) Register $::slicer3::Application
        set scalarsVar vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars)
        reader Delete
       }
    }

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersGDFPlotRead: read data file, got id $scalarsVar"
    }

    # link it in with the active model
    # set modelname [$Module(vtkFreeSurferReaders,fPlot).fModel.mbActive cget -text]
    # get the model id of the active model (assume that the linking still works), otherwise
    # need to figure out which of $Model(idList) is $modelname
    # set mid $::Model(activeID)
    set vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),modelID) $mid

    # set it to be pickable
    #::Model($mid,actor,viewRen) SetPickable 1

    #bind $::Gui(fViewWin) <ButtonRelease-1> {vtkFreeSurferReadersPickPlot %W %x %y}


    # now add the points to the lPoint list for vertex 0
    vtkFreeSurferReadersPlotBuildPointList 0 $scalarsVar 

    # now build the plot window
    if {$::Module(verbose)} {
        puts "Building plot window for $vtkFreeSurferReaders(gGDF,dataID), data id $scalarsVar"
    }
    vtkFreeSurferReadersGDFPlotBuildWindow $vtkFreeSurferReaders(gGDF,dataID)

    # and plot stuff by setting the mode to the default
    set vtkFreeSurferReaders(gPlot,$vtkFreeSurferReaders(gGDF,dataID),state,nVariable) 0
    vtkFreeSurferReadersPlotSetMode $vtkFreeSurferReaders(gGDF,dataID)

    return $vtkFreeSurferReaders(gGDF,dataID)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotCancel
# Cancel reading in the plot
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotCancel {} {
    global vtkFreeSurferReaders
    if {$::Module(verbose)} {
       puts "vtkFreeSurferReadersPlotCancel: does nothing"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersUncompressMGH 
# Uncompresses gzipped mgh files, which can be named .mgz or .mgh.gz.
# Will reset the vtkFreeSurferReaders(VolumeFileName), creating a new file (if possible) that's the uncompressed mgh volume.
# Returns -1 on failure.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersUncompressMGH {} {
    global vtkFreeSurferReaders Module

    # new changes, mgh reader will deal with this so return 0
    if {$::Module(verbose)} { puts "vtkFreeSurferReadersUncompressMGH: letting the mgh reader uncompress it" }
    return 0


    # if this is the first time we've hit an zipped mgh file, set up the uncompression program (and the temp dir?)
    if {$vtkFreeSurferReaders(MGHDecompressorExec) == "" ||
        ![file executable $vtkFreeSurferReaders(MGHDecompressorExec)]} {
        set vtkFreeSurferReaders(MGHDecompressorExec) [tk_getOpenFile \
                                                           -initialdir $::env(SLICER_HOME) \
                                                           -parent .tMain \
                                                           -title "A file that can gunzip:"]
    }
    # if it's a .mgz file, copy it to .mgh.gz so that gunzip can process it, otherwise just make a copy to uncompress
    if {[string match *.mgz $vtkFreeSurferReaders(VolumeFileName)]} {
        set longFileName [file rootname $vtkFreeSurferReaders(VolumeFileName)].mgh.gz
        set retval [catch {file copy -force $vtkFreeSurferReaders(VolumeFileName) $longFileName} errmsg]
        if {$retval != 0} {
            puts "Error: Cannot copy $vtkFreeSurferReaders(VolumeFileName) to $longFileName for decompression:\n$errmsg"
            return -1
        } else {
            set vtkFreeSurferReaders(VolumeFileName) $longFileName
        }
    } else {
        # if it's just an mgh.gz file:
        # make a copy to work from, as when unzip it, it's going to replace itself
        set fileCopyName COPY${vtkFreeSurferReaders(VolumeFileName)}
        set reteval [catch {file copy -force $vtkFreeSurferReaders(VolumeFileName) $fileCopyName} errmsg]
        if {$retval != 0} {
            puts "Error: Cannot copy $vtkFreeSurferReaders(VolumeFileName) to $fileCopyName for decompression:\n$errmsg"
            return -1
        } else {
            set vtkFreeSurferReaders(VolumeFileName) $fileCopyName
        }
    }

    if {[file exist [file rootname $vtkFreeSurferReaders(VolumeFileName)]]} {
        puts "Warning: file [file rootname $vtkFreeSurferReaders(VolumeFileName)] already exists, using it instead of unzipping $vtkFreeSurferReaders(VolumeFileName)"

    } else {
        puts " -- calling $vtkFreeSurferReaders(MGHDecompressorExec) $vtkFreeSurferReaders(VolumeFileName)"
        exec $vtkFreeSurferReaders(MGHDecompressorExec) $vtkFreeSurferReaders(VolumeFileName)
    }

    # take the .gz off the volume file name
    set vtkFreeSurferReaders(VolumeFileName) [file rootname $vtkFreeSurferReaders(VolumeFileName)]
    puts " -- reset the volume file name to the uncompressed version"

    return 0
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetQADirName
# Set the directory name that contains a series of subjects, then populates a frame with the subject names for selection.
# .ARGS
# path startdir a default directory to start looking from
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetQADirName { { startdir $::env(SLICER_HOME) } } {
    global vtkFreeSurferReaders Module

    set vtkFreeSurferReaders(QADirName) [tk_chooseDirectory \
                                             -initialdir $startdir \
                                             -mustexist true \
                                             -title "Select Directory Containing Subject Files" \
                                             -parent .tMain ]
    if {$Module(verbose) == 1} {
        puts "FreeSurferReaders QA directory name: $vtkFreeSurferReaders(QADirName)"
    }

    # got a valid directory, so let's not use the subjects file
    set vtkFreeSurferReaders(QAUseSubjectsFile) 0

    # pick up subject dirs from this directory and put them in the list box
    vtkFreeSurferReadersSetQASubjects
    vtkFreeSurferReadersQAResetSubjectsListBox
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetQASubjectsFileName
# The filename is set elsewhere, in variable vtkFreeSurferReaders(QASubjectsFileName)
# Set the flag to use it or not, vtkFreeSurferReaders(QAUseSubjectsFile) 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetQASubjectsFileName {} {
    global vtkFreeSurferReaders Module

    if {$Module(verbose) == 1} {
        puts "FreeSurferReaders QA Subjects filename: $vtkFreeSurferReaders(QASubjectsFileName)"
    }
    if {$vtkFreeSurferReaders(QASubjectsFileName) != ""} {
        set vtkFreeSurferReaders(QAUseSubjectsFile) 1
    } else {
        set vtkFreeSurferReaders(QAUseSubjectsFile) 0
    } 
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetQASubjects
# Add subdirectories of vtkFreeSurferReaders(QADirName), which should be the names of subjects, 
# to vtkFreeSurferReaders(QASubjectNames), or else read them from a subjects.csh file if the 
# $vtkFreeSurferReaders(QAUseSubjectsFile) flag is set.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetQASubjects {} {
    global vtkFreeSurferReaders Module

    set dirs ""
    set subjectnames ""

    # do we want to read from the subjects file or from the directory?
    if {!$vtkFreeSurferReaders(QAUseSubjectsFile)} {
        set dir $vtkFreeSurferReaders(QADirName)
        if { $dir != "" } {
            set retval "yes"
            if {!$vtkFreeSurferReaders(QAAlwaysGlob)} {
                set retval [tk_messageBox -parent .tMain -type yesnocancel -title "Subject directory check" -message "About to search in $dir for FreeSurfer subjects. Continue? \nPress Cancel to not ask again and always search.\n\nOperation may hang if greater than 1000 subject dirs are present, can reset dir in vtkFreeSurferReaders QA tab."]
            }
            if {$retval == "cancel"} {
                set vtkFreeSurferReaders(QAAlwaysGlob) 0
            }
            if {$retval == "yes" || $retval == "cancel"} {
                set files [glob -nocomplain $dir/*]
            
                foreach f $files {
                    if { [file isdirectory $f] &&
                         [file exists [file join $f mri]]} {
                        lappend dirs $f
                        lappend subjectnames [file tail $f]
                    } 
                    if {$::Module(verbose)} {
                        if {[file isdirectory $f] && ![file exists [file join $f mri]]} {
                            puts "Skipping subject dir $f, no mri subdirectory"
                        }
                    }
                }
            }
            
        }
    } else {
        # parse the names from the subject file
        
        # open the file
        if {[catch {set qasubfid [open $vtkFreeSurferReaders(QASubjectsFileName) r]} errmsg] == 1} {
            puts "ERROR opening $vtkFreeSurferReaders(QASubjectsFileName):\n$errmsg"
            puts "ERROR opening $vtkFreeSurferReaders(QASubjectsFileName):\n$errmsg"
            return
        }

        set subjectsdirstr ""
        # run through it looking for the setenv SUBJECT_DIR and set SUBJECTS lines
        while {![eof $qasubfid]} {
            set line [gets $qasubfid]
            # look at the start of the line so that any commented out ones don't get caught
            if {[regexp "^setenv SUBJECTS_DIR (.*)" $line matchVar subjectsdirstr] == 1} {
                if {$::Module(verbose)} { puts "Got subjects directory $subjectsdirstr"}
            }
            if {[regexp "^set SUBJECTS = (.*)" $line matchVar subjectstr] == 1} {
                if {$::Module(verbose)} { puts "Got subjects $subjectstr"}
                # take the brackets off and convert to a list, the subject names are space separated
                set subjectstrtrim [string trim $subjectstr {( )}]
                set subjectnames [split $subjectstrtrim]
            }
        }
        # close the file
        close $qasubfid
        # do a sanity check that the subjects dir contains the subjects
        # if yes, reset the global var
        # if not, put up a warning to reset the var or something... set from script location?
        if {$subjectsdirstr != ""} {
            set missingdirs ""
            foreach s $subjectnames {
                if {![file isdirectory [file join $subjectsdirstr $s]]} {
                    lappend missingdirs $s
                }
            }
            if {$missingdirs != ""} {
                puts "Subjects file $vtkFreeSurferReaders(QASubjectsFileName) specifies invalid subjects directory $subjectsdirstr : missing subjects:\n $missingdirs\nTherefore, not resetting the subjects directory, fix the .csh file or set the subject dir explicity and select subjects interactively"
            } else {
                # use the subjects dir from the file
                if {$::Module(verbose)} {
                    DevInfoWindow "Using the subjects dir from the subjects file:\n$subjectsdirstr"
                }
                set vtkFreeSurferReaders(QADirName) $subjectsdirstr
            }
        }
    }
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetQASubjects: dirs =\n$dirs\nnames = \n$subjectnames"
    }
    # sort them alpha
    set vtkFreeSurferReaders(QASubjectNames) [lsort $subjectnames]
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadQA
# Called when reading in a subjects.csh file from the command line
# .ARGS
# path fname the name of the subjects.csh file, full path to it
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadQA { fname } {
     global vtkFreeSurferReaders

    if {[file exists $fname] == 0} {
        puts "Load FreeSurfer QA: $fname does not exist!"
        return
    }

    # set the file name
    set vtkFreeSurferReaders(QASubjectsFileName) $fname
    vtkFreeSurferReadersSetQASubjectsFileName

    # read the subject
    vtkFreeSurferReadersSetQASubjects

    # set up the list box
    vtkFreeSurferReadersQAResetSubjectsListBox

    # tab to the Freesurfers window, QA tab
    Tab vtkFreeSurferReaders row1 QA
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQASetLoad
# Add this kind of volume to the list of volumes to load when doing a QA process.
# .ARGS
# string voltype - the kind of volume file to read in, added to vtkFreeSurferReaders(QAVolFiles)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQASetLoad {voltype} {
    global vtkFreeSurferReaders
 
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQASetLoad: start: loading: $vtkFreeSurferReaders(QAVolFiles)"
    }
    set ind [lsearch $vtkFreeSurferReaders(QAVolFiles) $voltype]
    if {$vtkFreeSurferReaders(QAVolFiles,$voltype) == 1} {
        if {$ind == -1} {
            # add it to the list
            # if it's aseg, add it to the front
            if {$voltype == "aseg"} {
                set vtkFreeSurferReaders(QAVolFiles) [linsert $vtkFreeSurferReaders(QAVolFiles) 0 $voltype]
                if {$::Module(verbose)} { puts "added aseg to the beginning $vtkFreeSurferReaders(QAVolFiles)"}
            } else {
                lappend vtkFreeSurferReaders(QAVolFiles) $voltype
            }
        }
    } else {
        if {$ind != -1} {
            # remove it from the list
            set vtkFreeSurferReaders(QAVolFiles) [lreplace $vtkFreeSurferReaders(QAVolFiles) $ind $ind]
        }
    }


    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQASetLoad: new: loading: $vtkFreeSurferReaders(QAVolFiles)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQASetLoadAddNew
# If the vtkFreeSurferReaders(QAVolTypeNew) isn't empty, and is not on the list 
# of types to check for, add it to the list kept in vtkFreeSurferReaders(QAVolFiles).
# Makes sure that label map volumes (asegs) are always at the front of the list.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQASetLoadAddNew {} {
    global vtkFreeSurferReaders

    # if the QAVolTypeNew isn't empty, and not on the list of types to check for, add it
    if {$vtkFreeSurferReaders(QAVolTypeNew) != ""} {
        # has more than one been added?
        foreach newvol $vtkFreeSurferReaders(QAVolTypeNew) {
            if {[lsearch $vtkFreeSurferReaders(QAVolFiles) $newvol] == -1} {
                # if it's a new aseg, add it to the front
                if {[regexp "^aseg.*" $newvol matchVar] == 1} {
                    set vtkFreeSurferReaders(QAVolFiles) [linsert $vtkFreeSurferReaders(QAVolFiles) 0 $newvol]
                } else {
                    # put it at the end
                    lappend vtkFreeSurferReaders(QAVolFiles) $newvol
                }
            }
        }
    }
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQASetLoadAddNew: now loading $vtkFreeSurferReaders(QAVolFiles)"
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersStartQA
# This will set up the interface for the first subject. Once it's done, the pop up
# panel button push will launch the next subject
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersStartQA {} {
    global vtkFreeSurferReaders Module

    # get the selected subjects from the list box
    set vtkFreeSurferReaders(QASubjects) ""
    foreach ind [$vtkFreeSurferReaders(qaSubjectsListbox) curselection] {
        lappend vtkFreeSurferReaders(QASubjects) [$vtkFreeSurferReaders(qaSubjectsListbox) get $ind]
    }
    
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersStartQA"
        puts "\t subjects dir =  $vtkFreeSurferReaders(QADirName)"
        puts "\t subjects to QA = $vtkFreeSurferReaders(QASubjects)"
        puts "\t vols to load = $vtkFreeSurferReaders(QAVolFiles)"
    }

    if {$vtkFreeSurferReaders(QASubjects) == ""} {
        DevInfoWindow "No subjects selected."
        return
    }


    # close out anything that's open
    puts "Closing all volumes to start QA"
    MainFileClose

    # turn off casting to short, as we're just viewing
    set vtkFreeSurferReaders(QAcast) $vtkFreeSurferReaders(castToShort)
    if {$vtkFreeSurferReaders(QAEdit)} {
         set vtkFreeSurferReaders(castToShort) 1
    } else {
        set vtkFreeSurferReaders(castToShort) 0
    }

    # set the foreground opacity level 
    set vtkFreeSurferReaders(sliceOpacity) $::Slice(opacity)
    set ::Slice(opacity) $vtkFreeSurferReaders(QAOpacity)
    MainSlicesSetOpacityAll

    # turn it into a better viewing set up
    set vtkFreeSurferReaders(QAviewmode) $::View(mode)
    MainViewerSetMode $vtkFreeSurferReaders(QADefaultView)
    RenderAll

    set islabelmap 0

    # clear out the global message
    set vtkFreeSurferReaders(QAmsg) ""

    # reset the scanning time?

    # this call will set up with the next subject
    vtkFreeSurferReadersQAReviewSubject [lindex  $vtkFreeSurferReaders(QASubjects) 0]    
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQAResetSubjectsListBox
# Called when vtkFreeSurferReaders(QASubjectNames) has been changed (not automatic),
# updates the list box that displays the names for selection. Will empty the box if the
# list of subjects is empty.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQAResetSubjectsListBox {} {
    global vtkFreeSurferReaders
    
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQAResetSubjectsListBox"
    }

    if {[info exist vtkFreeSurferReaders(QASubjectNames)] && $vtkFreeSurferReaders(QASubjectNames) != "" && [info exist vtkFreeSurferReaders(qaSubjectsListbox)]} {
        if {$::Module(verbose)} { 
            puts "vtkFreeSurferReadersQAResetSubjectsListBox: Using already set QASubjectNames, clearing out list box and repopulating it" 
        }
        # clear out the old stuff
        $vtkFreeSurferReaders(qaSubjectsListbox) delete 0 end
        # use for instead of foreach to preserve ordering
        for {set i 0} {$i < [llength $vtkFreeSurferReaders(QASubjectNames)]} {incr i} {
            set sub [lindex $vtkFreeSurferReaders(QASubjectNames) $i]
            $vtkFreeSurferReaders(qaSubjectsListbox) insert end $sub
            # for now make them all active as add them
            $vtkFreeSurferReaders(qaSubjectsListbox) selection set end end
        }
    } else {
        if {$::Module(verbose)} { 
            puts "Clearing the box if there's an empty list" 
        }
        if {[info exist vtkFreeSurferReaders(QASubjectNames)] && $vtkFreeSurferReaders(QASubjectNames) == "" && [info exist vtkFreeSurferReaders(qaSubjectsListbox)]} {
            # have an empty subjects list, clear the box 
            $vtkFreeSurferReaders(qaSubjectsListbox) delete 0 end
        } 
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetQAEdit
# The value of vtkFreeSurferReaders(QAEdit) is set elsewhere, it's a flag that
# determines if volumes read in for QA purposes are cast to short or not
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetQAEdit {} {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts $vtkFreeSurferReaders(QAEdit)
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildQAInteractor
# Build a new top level window, named .top$subject$vol and populate it with a comment
# entry box and a series of buttons to enter the qa result. When the button is pressed,
# it will call a procedure to record this volume's qa result.
# .ARGS
# string subject the subject we are qa-ing
# string vol the volume we're qa-ing
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildQAInteractor { subject vol } {
    global vtkFreeSurferReaders Gui

    if {$subject == "" || $vol == ""} {
        puts "No subject or volume..."
        return
    }
    if {$::Module(verbose)} {
        puts "Building interactor window for subject $subject and vol $vol"
    }

    if {[info command .top${subject}${vol}] !=  ""} {
        if {$::Module(verbose)} {
            puts "Already have a .top${subject}${vol}"
        }
        wm deiconify .top${subject}${vol}
        return
    }

    toplevel .top${subject}${vol}
    wm geometry .top${subject}${vol} +[winfo x .tMain]+10

    frame .top${subject}${vol}.f1 -bg $Gui(activeWorkspace)
    pack .top${subject}${vol}.f1  -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    set f .top${subject}${vol}.f1

    eval {label $f.lSubject -text "Subject = $subject"} $Gui(WLA)
    pack $f.lSubject

    eval {label $f.lVol -text "Volume = $vol"} $Gui(WLA)
    pack $f.lVol

    frame $f.fNotes -bg $Gui(activeWorkspace)
    frame $f.fEval -bg $Gui(activeWorkspace)

    pack $f.fNotes -side top -padx $Gui(pad) -pady $Gui(pad) -fill x
    pack $f.fEval -side top -padx $Gui(pad) -pady $Gui(pad) -fill x


    set f .top${subject}${vol}.f1.fNotes
    eval {label $f.lNotes -text "Notes:"} $Gui(WLA)
    eval {entry $f.eNotes -textvariable vtkFreeSurferReaders($subject,$vol,Notes) -width 50} $Gui(WEA)
    pack $f.lNotes -side left -padx 0
    pack $f.eNotes -side left -padx $Gui(pad) -expand 1 -fill x

    set f .top${subject}${vol}.f1.fEval
    foreach eval $vtkFreeSurferReaders(QAResultsList) \
        tip $vtkFreeSurferReaders(QAResultsListTooltips) {
        DevAddButton $f.b$eval $eval "vtkFreeSurferReadersRecordSubjectQA $subject $vol $eval"
        TooltipAdd $f.b$eval "$tip"
        pack $f.b$eval -side left -padx $Gui(pad) -expand 1
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildQAInteractorNextSubject
# This provides the button that will go onto the next subject.
# Calls the proc to review the next subject when the button is pressed.
#.ARGS
# string subject the next subject we are going to QA
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildQAInteractorNextSubject { subject } {
    global vtkFreeSurferReaders Gui

    if {$::Module(verbose)} {
        puts "Building next subject interactor window for subject $subject"
    }

    if {[info command .top${subject}] !=  ""} {
        puts "Already have a .top${subject}"
        wm deiconify .top${subject}
        return
    }
    toplevel .top${subject}
    wm geometry .top${subject} +[winfo x .tMain]+50

    frame .top${subject}.f1 -bg $Gui(activeWorkspace)
    pack .top${subject}.f1  -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    set f .top${subject}.f1

    eval {label $f.lSubject -text "Subject = $subject"} $Gui(WLA)
    pack $f.lSubject

    eval {label $f.lVol -text "Volumes = $vtkFreeSurferReaders(QAVolFiles)"} $Gui(WLA)
    pack $f.lVol

    frame $f.fNext -bg $Gui(activeWorkspace)
    pack $f.fNext -side top -padx $Gui(pad) -pady $Gui(pad) -fill x


    set f .top${subject}.f1.fNext
    # when you press this, it gets the next subject in the list, and starts the review process for them
    DevAddButton $f.bNext "Click for Next Subject" "vtkFreeSurferReadersReviewNextSubject $subject"
    pack $f.bNext -side left -padx $Gui(pad) -expand 1
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersRecordSubjectQA
# Appends to the QA log for this subject the evaluation and some data provenance information.
# .ARGS
# string subject the subject name
# string vol  the name of the volume 
# string eval the evaluation of the volume
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersRecordSubjectQA { subject vol eval } {
    global vtkFreeSurferReaders 

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersRecordSubjectQA subject = $subject, eval = $eval, vol = $vol"
    }

    # open the file for appending
    set fname [file join $vtkFreeSurferReaders(QADirName) $subject $vtkFreeSurferReaders(QASubjectFileName)]
    if {$::Module(verbose)} { puts "vtkFreeSurferReadersRecordSubjectQA fname = $fname" }

    # env(USER) may not be defined
    if {[info exists ::env(USER)] == 1} {
        set username "$::env(USER)"
    } else {
        if {[info exists ::env(USERNAME)] == 1} {
            set username "$::env(USERNAME)"
        } else {
            puts "WARNING: USER and USERNAME environment variables are not defined, using default"
            set username "default"
        }
    }
    set timemsg "[clock format [clock seconds] -format "%D-%T-%Z"]"
    # take out any spaces from the time zone
    set timemsg [join [split $timemsg] "-"]
    # make up the message with single quotes between each one for easy parsing later, 
    # leave out ones on the end as will get empty strings there
    set msg "$timemsg\"$username\"Slicer-$::SLICER(version)\"[ParseCVSInfo FreeSurferQA {$Revision: 1.56 $}]\"$::tcl_platform(machine)\"$::tcl_platform(os)\"$::tcl_platform(osVersion)\"$vol\"$eval\"$vtkFreeSurferReaders($subject,$vol,Notes)"
    
    if {[catch {set fid [open $fname "a"]} errmsg] == 1} {
        puts "Can't write to subject file $fname.\nCopy and paste this if you want to save it:\n$msg"
        puts "Cannot open subject's file for appending this QA run:\nfilename = $fname\n$errmsg"
    } else {
        # write it out
        puts $fid $msg
        close $fid
    }

    # now close down the window that called me
    if {$::Module(verbose)} {
        puts "Closing .top${subject}${vol}"
    }
    wm withdraw .top${subject}${vol}

    # and add to the global qa run's message
    append vtkFreeSurferReaders(QAmsg) "\n$msg"
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersResetTimeScale
#
#  When the starting slice or the slice step change, reset the timescale slider.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersResetTimeScale { } {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersResetTimeScale: startCOR: $vtkFreeSurferReaders(scanStartCOR), startSAG: $vtkFreeSurferReaders(scanStartSAG), step: $vtkFreeSurferReaders(scanStep), slice cor range high =  [Slicer GetOffsetRangeHigh 2], slice sag range high = [Slicer GetOffsetRangeHigh 1]"
    }
    # break up the scanning into cor scanning and sag scanning
    # cor goes from the high offset on slice 2 to the start value
    set corsteps [expr ( [Slicer GetOffsetRangeHigh 2] -  $vtkFreeSurferReaders(scanStartCOR) ) / $vtkFreeSurferReaders(scanStep)]
    # sag goes from the high offset on slice 1 to the start value
    set sagsteps [expr ( [Slicer GetOffsetRangeHigh 1] - $vtkFreeSurferReaders(scanStartSAG) ) / $vtkFreeSurferReaders(scanStep)]

    # then reset the slider to go from 0 to (corsteps + sagsteps) - 1
    if {[$::Module(vtkFreeSurferReaders,fQA).fBtns.sTime cget -to] != [expr $corsteps + $sagsteps - 1]} {
        $::Module(vtkFreeSurferReaders,fQA).fBtns.sTime configure -to [expr $corsteps + $sagsteps - 1]
        if {$::Module(verbose)} {
            puts "\n\n\treset time slider's to value to [expr $corsteps + $sagsteps - 1]"
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQAResetTaskDelay
#
#  When the scan pause changes, reset the task delay on the Start button.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQAResetTaskDelay { } {
    global vtkFreeSurferReaders Module

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQAResetTaskDelay: new delay: $vtkFreeSurferReaders(scanMs) ms"
    }
    if {[info command istask] != ""} {
        $::Module(vtkFreeSurferReaders,fQA).fBtns.play configure -taskdelay $vtkFreeSurferReaders(scanMs)
    }
}


#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQASetTime
#
#  Show the slice and view for the current time
# .ARGS
# int t the time step
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQASetTime { {t ""} } {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} { puts "vtkFreeSurferReadersQASetTime t = $t, QAtime = $::vtkFreeSurferReaders(QAtime)" }

    # if time hasn't been changed, return
    if { $t != "" } {
        if { $t == $::vtkFreeSurferReaders(QAtime) } {
            return
        }
    }

    if {$t == "" } {
        set t $vtkFreeSurferReaders(QAtime)
    } else {
        set vtkFreeSurferReaders(QAtime) $t
    }

    # recalibrate the slider if necessary
    vtkFreeSurferReadersResetTimeScale

    # set the slider
    $vtkFreeSurferReaders(timescale) set $t

    # now display the right slice
    # break up the scanning into cor scanning and sag scanning
    # cor goes from the high offset on slice 2 to the start value
    set corsteps [expr ( [Slicer GetOffsetRangeHigh 2] -  $vtkFreeSurferReaders(scanStartCOR) ) / $vtkFreeSurferReaders(scanStep)]
    # sag goes from the high offset on slice 1 to the start value
    set sagsteps [expr ( [Slicer GetOffsetRangeHigh 1] - $vtkFreeSurferReaders(scanStartSAG) ) / $vtkFreeSurferReaders(scanStep)]

    # if the time is less than the corsteps, reset the COR slice
    if {$t < $corsteps} {
        if {$::Module(verbose)} { 
            puts "$t < $corsteps, doing cor" 
        }
        # check the view mode
        if {$::View(mode) != "Single512COR"} {
            MainViewerSetMode "Single512COR"
        }
        # take the starting slice and add the time mult by the scan step
        set corslice [expr $vtkFreeSurferReaders(scanStartCOR) + [expr $t * $vtkFreeSurferReaders(scanStep)]]

        # puts -nonewline "\t C${corslice}" 

        MainSlicesSetOffset 2 $corslice
        RenderSlice 2
        # the 3d window is small, hopefully rendering it to show the updated 2d slice location won't take too long
        Render3D
    } else {
        if {$t == $corsteps} {
            # reset the slices on the cor and axi to be 0
            MainSlicesSetOffset 0 0
            MainSlicesSetOffset 2 0
            RenderSlice 0
            RenderSlice 2
        }
        # reset the SAG slice
        if {$::Module(verbose)} { 
            puts "$t >= $corsteps, doing sag" 
        }
        if {$::View(mode) != "Single512SAG"} {
            MainViewerSetMode "Single512SAG"
        }
        # take the starting slice and add t then take way the cor steps that have already been done
        set sagslice [expr $vtkFreeSurferReaders(scanStartSAG) + [expr ($t - $corsteps) * $vtkFreeSurferReaders(scanStep)]]
        # puts -nonewline "\tS${sagslice}" 

        MainSlicesSetOffset 1 $sagslice
        RenderSlice 1
        # the 3d window is small, hopefully rendering it to show the updated 2d slice location won't take too long
        Render3D
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersStepFrame
#
#  adjust the frame according to current increment and handle boundaries
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersStepFrame {} {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersStepFrame"
    }
    set first [$::Module(vtkFreeSurferReaders,fQA).fBtns.sTime cget -from]
    set last [$::Module(vtkFreeSurferReaders,fQA).fBtns.sTime cget -to]
    # set inc $::vtkFreeSurferReaders(scanStep)
    # incrementing is handled in the qa set time function
    set inc 1
    set t $::vtkFreeSurferReaders(QAtime)

    set t [expr $t + $inc]

    if {$t > $last} {
        set t $last
        # reset the view mode to normal if not there already
        if {$::View(mode) != $vtkFreeSurferReaders(QADefaultView)} {
            MainViewerSetMode $vtkFreeSurferReaders(QADefaultView)
        }
    } 
    if {$t < $first} {
        set t $first 
    } 
    vtkFreeSurferReadersQASetTime $t
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReviewNextSubject
# Figures out if there is a subject to do next, and calls vtkFreeSurferReadersQAReviewSubject
# for it if so. Stops the slice scanning playback for this subject as well.
# .ARGS
# string subject the current subject
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReviewNextSubject { subject } {
    global vtkFreeSurferReaders

    # turn off any istask
    $::Module(vtkFreeSurferReaders,fQA).fBtns.play off

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQAReviewNextSubject"
    }

    # if I'm the last one, just quit
    if {$subject == [lindex  $vtkFreeSurferReaders(QASubjects) end]} {
        puts "vtkFreeSurferReadersReviewNextSubject: No more subjects."
        return
    }

    # close the last ones
    if {$vtkFreeSurferReaders(QAClose)} {
        MainFileClose
    }

    # close the window that called me
    wm withdraw .top${subject}

    # find out where I am in the subjects list
    set myindex [lsearch  $vtkFreeSurferReaders(QASubjects)  $subject]

    # get the next one
    set nextindex [incr myindex]
    set nextsubject [lindex $vtkFreeSurferReaders(QASubjects) $nextindex]

    # review it
    vtkFreeSurferReadersQAReviewSubject $nextsubject
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQAReviewSubject
# Loads all the volumes selected for review that exist in this subject's mri directory,
# then sets up the scanning through the slices and kicks it off.
# Will preferentially load the mgh volumes over COR ones.
# Will load the free surfer colours if this is a label map, and load the label map into
# both the label map and the foreground volume, all other volumes go in the background.
# .ARGS
# string subject the subject we are reviewing
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQAReviewSubject { subject } {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQAReviewSubject"
    }

    set numLoaded 0
    
    foreach vol $vtkFreeSurferReaders(QAVolFiles) {

        vtkFreeSurferReadersBuildQAInteractor $subject $vol

        set subfilename [file join $vtkFreeSurferReaders(QADirName) $subject mri $vol]
        set filetoload ""
        # check to see if there's an mgh file there
        set mghfilenames [glob -nocomplain ${subfilename}*.*]
        if {$mghfilenames != ""} {
            # figure out if have more than one
            if {$::Module(verbose)} { puts $mghfilenames }
            set filetoload [lindex $mghfilenames 0]
        } else {
            # otherwise check for a COR file in a subdir of that name
            if {[file isdirectory $subfilename]} {
                # check to see if there's a COR-.info file there
                set corfilename [file join $subfilename COR-.info]
                if {[file exist $corfilename]} {
                    set filetoload $corfilename
                }
            }
        }
        if {$filetoload != ""} {
            # load it up
            puts "Loading $filetoload"
            
            # if it's an aseg volume (could be an added one) it's a label map, so keep it 
            # loaded in the foreground as well
            if {$vol == "aseg" || [regexp "^aseg.*" $vol matchVar] == 1} {
                set islabelmap 1

                # load the colours
                if {$vtkFreeSurferReaders(coloursLoaded) != 1} {
                    vtkFreeSurferReadersLoadColour
                }

                # vtkFreeSurferReadersLoadVolume $filetoload $islabelmap ${subject}-${vol}-lb
                # but, load it as a regular volume too
                # just load it as a label map and set both fore and label to it
                set volID [vtkFreeSurferReadersLoadVolume $filetoload $islabelmap ${subject}-${vol}]
                # and put it in the background instead so can see the colours
                if {$::Module(verbose)} { puts "Loaded aseg, got volume id $volID" }
                MainSlicesSetVolumeAll Fore $volID
                MainSlicesSetVolumeAll Label $volID
                RenderAll
            } else {
                set islabelmap 0
                set volID [vtkFreeSurferReadersLoadVolume $filetoload $islabelmap ${subject}-${vol}]
                # and put it in the foreground
                MainSlicesSetVolumeAll Fore $volID
            }
            # make all slices visible
            foreach s {0 1 2} {
                set ::Slice($s,visibility) 1
                MainSlicesSetVisibility $s
            }
            # then update the viewer
            RenderAll
            
            incr numLoaded

            # don't pause if this is the aseg and there are more to load
            if {$vol != "aseg" || [llength $vtkFreeSurferReaders(QAVolFiles)] == 1} {
                # reset the time scale

                # kick off the istask to cycle through the volumes
            }
        } else {
            puts "Can't find a file to load for $subject $vol, skipping"
            # remove the interactor for this volume
            wm withdraw .top${subject}${vol}
        }
    }
    # done the volumes for this subject

   
    # scan through slices now
    set vtkFreeSurferReaders(QAtime) 0
    $::Module(vtkFreeSurferReaders,fQA).fBtns.play on
    vtkFreeSurferReadersStepFrame

    if {$numLoaded == 0} {
        DevWarningWindow "No volumes loaded for subject $subject, skipping.\nTried: $vtkFreeSurferReaders(QAVolFiles)"
    } else {
        # scan through slices now
        set vtkFreeSurferReaders(QAtime) 0
        $::Module(vtkFreeSurferReaders,fQA).fBtns.play on
        vtkFreeSurferReadersStepFrame
    }

    # is this the last subject?
    if {$subject == [lindex $vtkFreeSurferReaders(QASubjects) end]} {
        vtkFreeSurferReadersBuildQAInteractorStop
    } else {
        # make the interactor that allows you to go onto the next subject
        vtkFreeSurferReadersBuildQAInteractorNextSubject $subject
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildQAInteractorStop
# Builds a button that stops everything by calling vtkFreeSurferReadersBuildQAStop 
# when the button is pressed.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildQAInteractorStop {} {
    global vtkFreeSurferReaders Gui

    if {$::Module(verbose)} {
        puts "Building done QA interactor window"
    }
    if {[info command .topStopQA] !=  ""} {
        if {$::Module(verbose)} {
            puts "Already have a .topStopQA"
        }
        wm deiconify .topStopQA
        return
    }
    toplevel .topStopQA
    wm geometry .topStopQA +[winfo x .tMain]+0

    frame .topStopQA.f1 -bg $Gui(activeWorkspace)
    pack .topStopQA.f1  -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    set f .topStopQA.f1

    frame $f.fStop -bg $Gui(activeWorkspace)
    pack $f.fStop -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

    set f .topStopQA.f1.fStop
    DevAddButton $f.bStop "Click to Stop QA" "vtkFreeSurferReadersQAStop"
    TooltipAdd $f.bStop "Writes results to a file. Turns off slice scanning, resets editing, view options.\nCloses all volumes."
    pack $f.bStop -side left -padx $Gui(pad) -expand 1

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQAStop
# Writes results to a file. Turns off slice scanning, resets editing, view options. 
# Closes all volumes.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQAStop {} {
    global vtkFreeSurferReaders 

    # puts stuff back the way it was
    puts "Resetting everything to the way it was before we changed things for qa"

    # turn off istask that scans through slices
    $::Module(vtkFreeSurferReaders,fQA).fBtns.play off
    
    # reset the time
    set vtkFreeSurferReaders(QAtime) 0

    set vtkFreeSurferReaders(castToShort) $vtkFreeSurferReaders(QAcast)
    MainViewerSetMode $vtkFreeSurferReaders(QAviewmode)
    
    set ::Slice(opacity) $vtkFreeSurferReaders(sliceOpacity)
    MainSlicesSetOpacityAll
    
    # reselect the subjects in case the user selected something elsewhere
    foreach s $vtkFreeSurferReaders(QASubjects) {
        # get the index of this subject
        for {set listIndex 0} {$listIndex < [$vtkFreeSurferReaders(qaSubjectsListbox) size]} {incr listIndex} {
            set thisSubj [$vtkFreeSurferReaders(qaSubjectsListbox) get $listIndex]
            if {$thisSubj == $s} {
                # select it
                if {$::Module(verbose)} {
                    puts "Reselecting $listIndex as $thisSubj == $s"
                }
                $vtkFreeSurferReaders(qaSubjectsListbox) selection set $listIndex $listIndex
            }
        }
    }

    # also write out the overall QA message to a file
    set fname [file join $vtkFreeSurferReaders(QADirName) QA-[clock format [clock seconds] -format "%Y-%m-%d-%H-%M-%S-%Z"].log]
    # make sure there are no spaces in the file name
    set fname [join [split $fname] "-"]
    if {[catch {set fid [open $fname "w"]} errmsg] == 1} {
        puts "Can't write to QA file $fname.\nCopy and paste this if you want to save it:\n $vtkFreeSurferReaders(QAmsg)"
        puts "Cannot open file for writing about this QA run:\nfilename = $fname\n$errmsg"
    } else {
        puts $fid  $vtkFreeSurferReaders(QAmsg) 
        close $fid
    }
    
    #        set closeup [tk_messageBox -type yesno -message "Do you want to close all subject volumes?"]
    #        if {$closeup == "yes"} {
    #            MainFileClose
    #        }
    if {$vtkFreeSurferReaders(QAClose)} {
        # close the last subject
        MainFileClose
    }

    # close the window that called us
    wm withdraw .topStopQA
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQASummary
# Builds a window that will display a series of html tables, one for each subject
# in the vtkFreeSurferReaders(QADirName) directory, showing all the QA results
# done so far. Buttons at the bottom will call vtkFreeSurferReadersQAMakeNewSubjectsCsh
# that will let you run a new qa session on a subset of the subjects, picked by the 
# evoluation. It resorts the fields from the QA logs to a more useful order for display.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQASummary {} {
    global vtkFreeSurferReaders Module Gui

    if {$Module(verbose)} {
        puts "vtkFreeSurferReadersQASummary"
    }

    # check to see that the subjects directory is set
    set subjectsDir  $vtkFreeSurferReaders(QADirName)
    if {$subjectsDir == ""} {
        DevWarningWindow "No subjects directory set, can't summarise results"
        return
    }

    set summaryName [file tail [file dirname $subjectsDir]]

    # check to see if we already built a summary window
    if {[info command .top${summaryName}] != ""} {
        if {$::Module(verbose)} {
            puts "Already have a .top${summaryName}, showing and returning"
        }
        wm withdraw .top${summaryName}
        wm deiconify .top${summaryName}
        
    } else {
        # make the window
        toplevel .top${summaryName}
    
        foreach fname {Summary Display NewQA Cancel} {
            frame .top${summaryName}.f${fname} -bg $Gui(activeWorkspace)
            pack .top${summaryName}.f${fname}  -side top -padx $Gui(pad) -pady 0 -fill x
        }


        set f .top${summaryName}.fSummary

        eval {label $f.lDirname -text "Subjects directory = $subjectsDir"} $Gui(WLA)
        pack $f.lDirname

        # make a scrolled pane to display things in
        set f .top${summaryName}.fDisplay

        iwidgets::scrolledhtml $f.shDisplay \
            -height 200 -width 700 \
            -background $Gui(activeWorkspace)
        pack $f.shDisplay -side top -fill x 

        # make some buttons to create new subjects.csh files
        set f .top${summaryName}.fNewQA
        DevAddLabel $f.lnewqa "Make a new subjects file, using the most recent addition to the log file for each volume:"
        pack $f.lnewqa -side top
        foreach newqa $vtkFreeSurferReaders(QAResultsList) {
            DevAddButton $f.b$newqa "QA $newqa" "vtkFreeSurferReadersQAMakeNewSubjectsCsh $subjectsDir $newqa"
            TooltipAdd $f.b$newqa "Create a new subjects.csh file to review subjects with volumes marked as $newqa"
            pack $f.b$newqa -side left -padx $Gui(pad) -expand 1
        }

        # cancel, close the window
        set f .top${summaryName}.fCancel
        DevAddButton $f.bCancel "Cancel" "wm withdraw .top${summaryName}"
        TooltipAdd $f.bCancel "Close the window without writing any new subject .csh files"
        pack $f.bCancel -side top -pady $Gui(pad) -expand 1
    }

    set summaryHTMLString "<h2>QA Summary for $subjectsDir</h2>"

    # find the information in the QA logs in the subjects directories
    foreach subject $vtkFreeSurferReaders(QASubjectNames) {
        set topdir [file join $subjectsDir $subject]

        if {$::Module(verbose)} { 
            puts "QA logs for $subject, dir $topdir"
        }
        # find the file named QA.log
        set qaFileName [file join $topdir $vtkFreeSurferReaders(QASubjectFileName)]
        if {[file exists $qaFileName] == 1 } {
            if {$::Module(verbose)} { 
                puts "Found a QA log file for subject $subject: $qaFileName"
            }
            append summaryHTMLString "<h3>$subject</h3>"
            # read the qa log file
            if {[catch {set fid [open $qaFileName "r"]} errmsg] == 1} {
                puts "Can't open file $qaFileName for reading"
            } else {
                # make the table header
                append summaryHTMLString "<table>"
                append summaryHTMLString "<tr><td><b>Volume</b></td> <td><b>Evaluation</b></td> <td><b>Notes</b></td> <td><b>User</b></td> <td><b>Time</b></td> <td><b>Slicer Version</b></td> <td><b>Script Version</b></td> <td><b>Machine</b></td> <td><b>OS</b></td> <td><b>OS Version</b></td>  </tr>"
                # read in line by line
                while {![eof $fid]} {
                    set line [gets $fid]
                                        
                    # break up the line
                    set newline [split [string trim $line] \"]
                    # Pre October 2005:
                    # get bits of the line that are in quotes and reassemble them
                    # they're the second and fourth tokens in the line, leave them alone
                    # and split the first and third tokens by spaces
                    set dateuserver [split [string trim [lindex $newline 0]]]

                    if {[regexp {^(\d\d)/(\d\d)/(\d\d)-.*} $dateuserver matchVar mo day yr] == 1} {
                        if {($yr <= 5 && $mo <= 10)} {
                            # if the regexp was sucessful, check the year and mo to make sure we're earlier than oct 2005 so that
                            # we're using the old style string where not everything is in quotes
                            if {$::Module(verbose)} {
                                puts "QA: have an old style string earlier than Oct 05 with yr = $yr, mo = $mo: $dateuserver"
                            }
                            
                            set filerev [lindex $newline 1]
                            set machosvervoleval [split [string trim [lindex $newline 2]]]
                            set notes [lindex $newline 3]
                            
                            # save the tokens for this user in an array, used later for building new review files
                            foreach token [split $dateuserver] fld {time user slicerver} { 
                                set $fld $token
                            }
                            foreach token [split $machosvervoleval] fld {machine os osver vol eval} {
                                set $fld $token
                            }
                        } else {
                            # it's the new version
                            if {$::Module(verbose)} {
                                puts "Parsed out the date for this line, it's more recent than oct 05: $dateuserver"
                            }
                            foreach token $newline fld {time user slicerver filerev machine os osver vol eval notes} {
                                set $fld $token
                            }
                        }
                    } else {
                        # assume that it's using the new version with quotes
                        if {$::Module(verbose)} {
                            puts "New version qa parsing.\nTokenized line = '$newline'"
                        }
                        foreach token $newline fld {time user slicerver filerev machine os osver vol eval notes} {
                            set $fld $token
                            if {$::Module(verbose)} { puts "QA parsing: fld $fld set to token $token" }
                        }
                    }
                    # save the information - the file may contain multiple evals of a volume,
                    # just interested in the last one, so over writing is fine (only overwrites in the variable, prints out all of them)
                    # will put the specific volumes in the comments of the new subjects.csh 
                    append summaryHTMLString "<tr>"
                    append summaryHTMLString "<td>${vol}</td>"
                    foreach fld {eval notes user time slicerver filerev machine os osver} {
                        set val [subst $$fld]
                        set ::vtkFreeSurferReaders(NewQA,$subject,$vol,$fld) $val
                        append summaryHTMLString "<td>${val}</td>"
                    }
                    append summaryHTMLString "</tr>"
                }
                close $fid
                append summaryHTMLString "</table>"
            }
        } else {
            puts "Warning: no $vtkFreeSurferReaders(QASubjectFileName) file found in $topdir, skipping sujbect $subject" 
        }

    }

    # display the information
    .top${summaryName}.fDisplay.shDisplay render $summaryHTMLString
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersQAMakeNewSubjectsCsh
# Writes out a new .csh file to the subjectsDir/scripts directory. Will try to create
# the directory if it doesn't exist. Adds each subject once, with notes in the comments
# about which volumes got this evaluation
# .ARGS
# string subjectsDir the directory above where to write the new file
# string subset the evaluation that defines the subset of subjects to write into the new csh file, valid values are defined in  vtkFreeSurferReaders(QAResultsList)
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersQAMakeNewSubjectsCsh { subjectsDir { subset  "Review" } } {
    global vtkFreeSurferReaders 

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersQAMakeNewSubjectsCsh  subjectsDir = $subjectsDir ; subset = $subset"
    }

    set scriptsDirName  [file join $subjectsDir scripts]
    if {![file isdirectory $scriptsDirName]} {
        # create the scripts directory
        if {[catch {file mkdir $scriptsDirName} errmsg] == 1} {
            puts "Can't create directory $scriptsDirName: $errmsg\nWill write to standard error."
            set fid stderr
        }
    }

    # write a time stamped subset csh
    set fname [file join $scriptsDirName subjects${subset}-[clock format [clock seconds] -format "%Y-%m-%d-%H-%M-%S-%Z"].csh]
    # take out any spaces
    set fname [join [split $fname] "-"]
    if {$::Module(verbose)} { puts "fname = $fname" }
    if {[catch {set fid [open $fname "w"]} errmsg] == 1} {
        puts "Can't open $fname for writing, file contents will be on stderr"
        set fid stderr
    }

    puts $fid "\# These subjects have already been QA'd, with a most recent result of $subset"
    puts $fid "setenv SUBJECTS_DIR $subjectsDir"

    puts -nonewline $fid "set SUBJECTS = ("
    set subjlineList ""
    set comments "\# Volumes rated $subset for each subject: "
    foreach subject $vtkFreeSurferReaders(QASubjectNames) {
        set subjAdded 0
        foreach vol $vtkFreeSurferReaders(QAVolTypes) {
            if {[info exist vtkFreeSurferReaders(NewQA,$subject,$vol,eval)] &&
                $vtkFreeSurferReaders(NewQA,$subject,$vol,eval) == $subset} {
                if {!$subjAdded} {
                    if {$::Module(verbose)} { puts "need to add subject $subject vol $vol to file" }
                    lappend subjlineList $subject
                    set subjAdded 1
                }
                append comments "$subject:$vol "
            }
        }
    }
    foreach sub $subjlineList {
        puts -nonewline $fid $sub
        # only add a space if there's another one
        if {[lsearch $subjlineList $sub] < [expr [llength $subjlineList] - 1]} {
            puts -nonewline $fid " "
        }
    }
    # close the braces
    puts $fid ")"
    # put in the comments 
    puts $fid $comments
    close $fid

    # close the window that called me
    set summaryName [file tail [file dirname $subjectsDir]]
    set resp [tk_messageBox -type yesno -message "File written: $fname.\nDo you want to close the summary window?"]
    if {$resp == "yes"} {
        wm withdraw .top${summaryName}
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPlotBuildPointList 
# Adds data points to the plotter's point list
# .ARGS
# int pointID the vertex id of the window to plot
# int scalarVar the variable that points to the scalar volume data
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotBuildPointList {pointID scalarVar} {
    global vtkFreeSurferReaders

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersPlotBuildPointList pointID $pointID, scalarVar $scalarVar"
    }

    # clear out the list
    vtkFreeSurferReadersPlotBeginPointList $pointID

    if {$::Module(verbose)} { puts "vtkFreeSurferReadersPlotBuildPointList: getting the scalar vars number of tuples and components: $scalarVar" }

    # add to it, just the subject data for this vertex
    if {[info command [subst $$scalarVar]] != ""} {
    set numPoints [[subst $$scalarVar] GetNumberOfTuples]
    set numSubjects [[subst $$scalarVar] GetNumberOfComponents]
    } else {
    set numPoints 0
    set numSubjects 0
    }
    if {$::Module(verbose)} { puts "BuildPointList: got numPoints = $numPoints, numSubjects = $numSubjects" }


    if {$pointID >= $numPoints || $pointID < 0} { 
        puts "ERROR: cannot build a graph data list for point $pointID, valid range is 0 to [expr $numPoints-1]"
        return
    }

    for {set sid 0} {$sid < $numSubjects} {incr sid} {
        set pointList [[subst $$scalarVar] GetComponent $pointID $sid]
        vtkFreeSurferReadersPlotAddPoint $pointID [lindex $pointList 0] [lindex $pointList 1] [lindex $pointList 2]
    }

    # try using regression on this vertex to get the line
    set vtkFreeSurferReaders(gPlot,$pointID,state,bTryRegressionLine) 1

    # and finish (plot)
    vtkFreeSurferReadersPlotEndPointList $pointID
    
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPickPlot
#
# .ARGS
# windowpath widget the window in which a point was picked
# int x x coord of the pick
# int y y coord of the pick
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPickPlot {widget x y} {
    global vtkFreeSurferReaders Point Module Model viewRen Select

    set testPickers 0
    set verb $::Module(verbose) 
    set ::Module(verbose) 0

    if {$::Module(verbose)} { 
        puts "\nvtkFreeSurferReadersPickPlot: widget = $widget, x = $x, y = $y"
        puts "Getting picked point for plotting" 
    }

    
    if {$testPickers} {
        # vtkFastCellPicker
        set fastPickMs [time {set retval [SelectPick Select(picker) $widget $x $y]}]
        if {$::Module(verbose)} {
            puts "vtkFastCellPicker took $fastPickMs, retval = $retval, tolerance = [Select(picker) GetTolerance]"
            puts "\tvtkFastCellPicker cell id = [Select(picker) GetCellId]"
        }
        

        # vtkPickFilter
        catch "wldPicker Delete"
        vtkWorldPointPicker wldPicker
        set wldPickMS [time {set retval [wldPicker Pick $x $y 0.0 viewRen]}]
        if {$::Module(verbose)} {
            puts "vtkWorldPointPicker time = $wldPickMS, retval = $retval"
        }
        set wldPt [wldPicker GetPickPosition]
        
        catch "pickFilter Delete"
        vtkPickFilter pickFilter
        # assume model 0 for now, need to pass in polydata GetOutput
        pickFilter AddInput $Model(0,polyData)
        pickFilter SetWorldPoint [lindex $wldPt 0] [lindex $wldPt 1] [lindex $wldPt 2] 
        pickFilter PickCellOn
        pickFilter Update
        set cellpickMs [time {set cellID [pickFilter GetCellId]}]
        if {$::Module(verbose)} {          
            puts "\tvtkPickFilter get cell id time = $cellpickMs"
            puts "\tvtkPickFilter using model 0"
            puts "\tvtkPickFilter cell id = $cellID"
        }


        if {0} {
            # this is way too slow
            # vtkCellPicker
            set pickMs [time {set retval [SelectPick Point(picker) $widget $x $y]}]
            if {$::Module(verbose)} {
                puts "vtkCellPicker took $pickMs"
                puts "\tvtkCellPicker cell id = [Point(picker) GetCellId]"
            }
            if { $retval != 0} {
                # check for a model
                set actor [Point(picker) GetActor]
                set Point(model) ""
                set modelID -1
                foreach id $Model(idList) {
                    foreach r $Module(Renderers) {
                        if { $actor == "Model($id,actor,$r)" } {
                            set Point(model) Model($id,actor,$r)
                            set modelID $id
                        }
                    }
                }
                if {$vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),modelID) != $modelID} {
                    puts "Can't plot a point on model $modelID, only on $vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),modelID)"
                    return
                }
                
                set cellId [Point(picker) GetCellId]
                if {$::Module(verbose)} {
                    puts "\tPoint(model) = $Point(model)"
                    # cellId = $cellId
                }
            }
        }
    }
    # use a point picker to get the point the cursor was over, then pass that in to the plotter
    set pointpickMs [time {set retval [SelectPick Select(ptPicker) $widget $x $y]}]
    if {$::Module(verbose)} { 
        puts "vtkPointPicker took $pointpickMs" 
        puts "\ttolerance = [Select(ptPicker) GetTolerance]"
    }
    if {$retval != 0} {
        set pid [Select(ptPicker) GetPointId]
        if {$::Module(verbose)} {
            puts -nonewline "\tPoint id = $pid\n\tIn cells:"
            # find the cells that it's part of
            catch "cellIds Delete"
            vtkIdList cellIds
            $Model(0,polyData) GetPointCells $pid cellIds
            for {set i 0} {$i < [cellIds GetNumberOfIds]} {incr i} {
                puts -nonewline " [cellIds GetId $i]"
            }
            puts "\n"
        }
        # check against the scalars array
        if {[$vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) GetNumberOfComponents] < $pid} {
            if {$::Module(verbose)} {
                puts "\tPlotting point $pid, data id = $vtkFreeSurferReaders(gGDF,dataID)"
            }
            set ::Module(verbose) $verb
            vtkFreeSurferReadersPlotPlotData $pid $vtkFreeSurferReaders(gGDF,dataID) 
        }
    } else {
        if {$::Module(verbose)} {
            puts "\tSelect(ptPicker) didn't find anything at $x $y"
        }
    }
    set ::Module(verbose) $verb
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPickScalar
# Get the scalar value at the picked point, on the active model
# .ARGS
# windowpath widget the window in which a point was picked
# int x x coord of the pick
# int y y coord of the pick
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPickScalar {widget x y} {
    global vtkFreeSurferReaders Point Module Model viewRen Select

    if {$::Module(verbose)} { puts "vtkFreeSurferReadersPickScalar: widget = $widget, x = $x, y = $y"
    }

    # use a point picker to get the point the cursor was over, 
    # then pass that in to the display widget
    set pointpickMs [time {set retval [SelectPick Select(ptPicker) $widget $x $y]}]
    if {$::Module(verbose)} { 
        puts "vtkPointPicker took $pointpickMs" 
    }
    if {$retval != 0} {
        set pid [Select(ptPicker) GetPointId]
#        set mid $Model(activeID)
        # get the model
        set actors [Select(ptPicker) GetActors]
        if {$::Module(verbose)} { puts "Actors = $actors" }
        if {[$actors GetNumberOfItems] > 0} {
            $actors InitTraversal
            set a [$actors GetNextActor]
            if {[regexp {^Model[(](.*),actor,viewRen[)]} $a matchVar mid] == 1} {
                if {$::Module(verbose)} { puts "got model id $mid" }
            } else {
                if {$::Module(verbose)} { puts "found no model id, using 0" }
                set mid 0
            }
        }

        # check against the scalar array' size
        if {[info exists Model($mid,polyData)] == 1} {
            set ptData [$Model($mid,polyData) GetPointData]
            set scalars [$ptData GetScalars]
            if {$scalars != ""} {
                set numTuples [$scalars GetNumberOfTuples]
                if {$::Module(verbose)} { puts "pid = $pid, numTuples = $numTuples for model $mid" }
                if {$pid >= 0 && $pid < $numTuples} {
                    # get the scalar value
                    set val [[[$Model($mid,polyData) GetPointData] GetScalars] GetValue $pid]
                    # get the colour that it's mapped to
                    set col [[Model($mid,mapper,viewRen) GetLookupTable] GetColor $val]
                    
                    # get the name of the colour if it's a label map
                    set colourName ""
                    set scalarsName  [[[$Model($mid,polyData) GetPointData] GetScalars] GetName]
                    if {$scalarsName == "labels"} {
                        if {[array exists ::${mid}vtkFreeSurferReadersLabels]} {
                            global ::${mid}vtkFreeSurferReadersLabels
                            set colourName [subst $${mid}vtkFreeSurferReadersLabels($val)]
                        } else {
                            if {$::Module(verbose)} {
                                puts "can't find array ${mid}vtkFreeSurferReadersLabels"
                            }
                        }
                    }
                    
                    if {$::Module(verbose)} { puts "pid $pid val = $val, colour = $col, colourName = $colourName (scalars name = $scalarsName)" }
                    vtkFreeSurferReadersShowScalarValue $mid $pid $val $col $colourName
                } else {
                    if {$::Module(verbose)} { puts "pid $pid out of range of $numTuples for model $mid" }
                }
            } else {
                if {$::Module(verbose)} { puts "No scalars in model $mid"}
            }
        } else {
            if {$::Module(verbose)} {
                puts "no poly data for model $mid"
            }
        }
    } else {
        if {$::Module(verbose)} {
            puts "\tSelect(ptPicker) didn't find anything at $x $y"
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersShowScalarValue
# Build or redisplay a pop up window with the picked scalar value
# .ARGS
# int mid model id
# int pid point id (vertex number)
# float val scalar value
# string col the colour that the scalar value has mapped to
# string colourName the name of the colour that the scalar has been mapped to
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersShowScalarValue {mid pid val col {colourName ""}} {
    global vtkFreeSurferReaders Gui

    if {$::Module(verbose)} {  puts "mid = $mid, pid = $pid, val = $val, colour name = $colourName" }

    set w .topScalars${mid}
    if {[info command $w] != ""} {
        if {$::Module(verbose)} { puts "Already have a $w" }
        wm deiconify $w
        $w.f.lScalar configure -text "Scalar Value = $val"
        $w.f.lColour configure -text "Display Colour = $col"
        $w.f.lColourName configure -text "$colourName"
        $w.f.lPoint configure -text "Point id = $pid"
    } else {
        # build it
        toplevel $w
        wm geometry $w +[winfo x .tViewer]+10
        wm title $w "Model $mid [Model($mid,node) GetName]"

        frame $w.f -bg $Gui(activeWorkspace)
        pack $w.f -side top -padx $Gui(pad) -pady $Gui(pad) -fill x

        eval {label $w.f.lScalar -text "Scalar Value = $val" -width 40} $Gui(WLA)
        eval {label $w.f.lColour -text "Display Colour = $col" -width 40} $Gui(WLA)
        eval {label $w.f.lColourName -text "$colourName" -width 40} $Gui(WLA)
        eval {label $w.f.lModel -text "Model id = $mid" -width 40} $Gui(WLA)
        eval {label $w.f.lPoint -text "Point id = $pid" -width 40} $Gui(WLA)
        pack $w.f.lScalar $w.f.lColour $w.f.lColourName $w.f.lModel $w.f.lPoint -side top 

        DevAddButton $w.f.bClose "Close" "wm withdraw $w"
        pack $w.f.bClose -side top -pady $Gui(pad) -expand 1
    }

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersBuildModelScalarsGui
# Will build or rebuild the $Module(vtkFreeSurferReaders,fModels).fScalar frame 
# so that it holds radio buttons for all of the scalars to load
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersBuildModelScalarsGui { {f ""} } {
    global vtkFreeSurferReaders Module Gui

    if {$f == ""} {
        set f $Module(vtkFreeSurferReaders,fModels).fScalar
    }

    if {[info command $f.lTitle] == ""} {
        DevAddLabel $f.lTitle "Load Associated Scalar files:"
        pack $f.lTitle -side top -padx $Gui(pad) -pady 0
    }
    foreach scalar $vtkFreeSurferReaders(scalars) {
        if {[info command $f.r$scalar] == ""} {
            if {$::Module(verbose)} {
                puts "vtkFreeSurferReadersBuildModelScalarsGui: adding button for $scalar"
            }
            eval {checkbutton $f.r$scalar \
                      -text "$scalar" -command "vtkFreeSurferReadersSetLoad $scalar" \
                      -variable vtkFreeSurferReaders(assocFiles,$scalar) \
                      -width 9 \
                      -indicatoron 0} $Gui(WCA)
            pack $f.r$scalar -side top -padx 0
        } else {
            if {$::Module(verbose)} {
                puts "vtkFreeSurferReadersBuildModelScalarsGui: $scalar already has a button"
            }
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersScalarSetLoadAddNew
# If the vtkFreeSurferReaders(scalarsNew) isn't empty, and is not on the list 
# of scalars to load with a surface, vtkFreeSurferReaders(assocFiles), add it to 
# the list kept in scalars and redraw the radio buttons.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersScalarSetLoadAddNew {} {
    global vtkFreeSurferReaders

    set rebuildGui 0
    # if the scalarsNew isn't empty, and not on the list of types to check for, add it
    if {$vtkFreeSurferReaders(scalarsNew) != ""} {
        # has more than one been added?
        foreach newscalar $vtkFreeSurferReaders(scalarsNew) {
            if {[lsearch $vtkFreeSurferReaders(scalars) $newscalar] == -1} {
                # put it at the end
                lappend vtkFreeSurferReaders(scalars) $newscalar
                incr rebuildGui
            }
        }
    }
    if {$rebuildGui > 0} {
        # add a radio button for new ones
        if {$::Module(verbose)} { 
            puts "vtkFreeSurferReadersScalarsSetLoadAddNew: rebuilding the gui"
        }
        vtkFreeSurferReadersBuildModelScalarsGui
    }

    # now reset the list to empty
    set vtkFreeSurferReaders(scalarsNew) ""

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersScalarsSetLoadAddNew: now loading $vtkFreeSurferReaders(scalars)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersAddLuts
# Add FreeSurfer look up tables to the Lut(idLabel) list, and build vtk obj for them.
# Adds a table for each name listed in vtkFreeSurferReaders(lutNames).
# Will create a table with a unique id. Will overwrite the last lut in the id list
# who's name matches if it has already been created.
# Saves the IDs in vtkFreeSurferReaders(lutIDs)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersAddLuts {} {
    global vtkFreeSurferReaders Lut 

    # clear out the id list
    set vtkFreeSurferReaders(lutIDs) ""

    set useCXXTable 1

    foreach newLut $vtkFreeSurferReaders(lutNames) {
        set nextId -1

        # has this lut name already been added?
        foreach lid $Lut(idList) {
            if {$Lut($lid,name) == $newLut} {
                set nextId $lid
            }
        }
        if {$nextId == -1} {
            # find the next id
            set nextId [expr [lindex [lsort -integer $Lut(idList)] end] + 1]
            if {$::Module(verbose)} {
                puts "vtkFreeSurferReadersAddLuts: next id = $nextId"
                puts "vtkFreeSurferReadersAddLuts adding $newLut lut"
            }
            lappend Lut(idList) $nextId
            set Lut($nextId,name) "$newLut"
            set Lut($nextId,fileName) ""
        } else {
            if {$::Module(verbose)} {
                puts "$newLut already there (id = $nextId), redefining it"
            }
        }
        lappend vtkFreeSurferReaders(lutIDs) $nextId
        set Lut($nextId,numberOfColors) 256

        

        if {$useCXXTable == 1} {
            # build vtk if not already allocated
            if {[info command Lut($nextId,lut)] == "" || [Lut($nextId,lut) GetClassName] != "vtkFSLookupTable"} {
                catch "Lut($nextId,lut) Delete"
                vtkFSLookupTable Lut($nextId,lut)
                if {$::Module(verbose)} {
                    Lut($nextId,lut) DebugOn
                }
            }
            
            Lut($nextId,lut) SetLutTypeTo${newLut}
            # hack to get some output from the ones not defined yet in vtkFSLookupTable
            switch $newLut {
                "BlueRed" {
                    Lut($nextId,lut) SetLutTypeToHeat
                }
                "RedBlue" {
                    Lut($nextId,lut) SetLutTypeToHeat
                    Lut($nextId,lut) ReverseOn
                }
            }
        } else {
            # the original way of definining the luts, using vtkLookupTable
            if {[info command Lut($nextId,lut)] == "" || [Lut($nextId,lut) GetClassName] != "vtkLookupTable"}  {
                catch "Lut($nextId,lut) Delete"
                vtkLookupTable Lut($nextId,lut)
            }
        
            switch $newLut {
                "RedGreen" {
                    set Lut($nextId,hueRange) "0 .8"
                    set Lut($nextId,saturationRange) "1 1"
                    set Lut($nextId,valueRange) "1 0"
                    set Lut($nextId,annoColor) "1 1 0"
                    foreach param "NumberOfColors HueRange SaturationRange ValueRange" {
                        eval Lut($nextId,lut) Set${param} $Lut($nextId,[Uncap ${param}])
                    }
                    Lut($nextId,lut) SetRampToLinear
                    Lut($nextId,lut) Build
                }
                "GreenRed" {
                    # brightening value
                    set offset 0.1
                    # insert the min/max values
                    set b  [expr 1.0 * ($offset  + 0.95*(1.0-$offset))]
                    Lut($nextId,lut) SetTableValue 0 0.0 0.0 $b 1.0
                    Lut($nextId,lut) SetTableValue 255 $b $b 0.0 1.0
                    # then build the rest of them
                    for {set i 1} {$i < 255} {incr i} {
                        # the r and g are flipped half way through the scale
                        set curv [expr ($i - 127.0) / 254.0]
                        set f [expr tanh($curv)]
                        if {$f > 0} {
                            set r [expr 1.0 * ($offset + 0.95*(1.0-$offset)*abs($f))]
                            set g [expr 1.0 * ($offset * (1.0 - abs($f)))]
                        } else {
                            set r [expr 1.0 * ($offset * (1.0 - abs($f)))]
                            set g [expr 1.0 * ($offset + 0.95*(1.0-$offset)*abs($f))]
                        }
                        set b [expr 1.0 * ($offset*(1 - abs($f)))]
                        Lut($nextId,lut) SetTableValue $i $r $b $g 1.0
                    }
                    # set the global vars
                    set Lut($nextId,hueRange) [Lut($nextId,lut) GetHueRange]
                    set Lut($nextId,saturationRange) [Lut($nextId,lut) GetSaturationRange]
                    set Lut($nextId,valueRange) [Lut($nextId,lut) GetValueRange]
                    set Lut($nextId,annoColor) "1 1 1"
                }
                "Heat" -
                "BlueRed" {
                    set useNew 1
                    if {$newLut == "Heat"} {
                        set invphaseflag 1
                    }
                    set truncphaseflag 1
                    
                    set fthresh .5
                    set fmid -.5
                    set tmpoffset 0.23
                    set fslope .85
                    set r 0.0
                    set g 0.0
                    set b 0.0
                    
                    # zero out the table
                    for {set c 0} {$c < $Lut($nextId,numberOfColors)} {incr c} {
                        Lut($nextId,lut) SetTableValue $c 0.0 0.0 0.0 0.5
                    }
                    
                    # this part calculates the blue part of the LUT properly
                    set halfcols [expr $Lut($nextId,numberOfColors) / 2]
                    # set c -$halfcols
                    for {set c 0} {$c < $halfcols} {incr c} {
                        set f [expr 1.0 * $c / $Lut($nextId,numberOfColors)]
                        set fcurv $f
                        if {$invphaseflag == 1} {
                            set f [expr 0.0 - $f]
                        } 
                        if {$truncphaseflag && $f < 0} {
                            set f 0.0
                        }
                        if {[expr abs($f)] > $fthresh && [expr abs($f)] < $fmid} {
                            set ftmp [expr abs($f)]
                            set c1 [expr 1.0 / ($fmid - $fthresh)]
                            if {$fcurv != 1.0} {
                                set c2tmp [expr ($fmid - $fthresh) * ($fmid - $fthresh)]
                                set c2 [expr ($fmid - $fthresh - $fcurv * $c1 * $c2tmp) / ((1.0 - $fcurv) * ($fmid - $fthresh))]
                            } else {
                                set c2 0.0
                            }
                            set ftmp [expr $fcurv * $c1 * ($ftmp - $fthresh) * ($ftmp - $fthresh) + $c2*(1.0 - $fcurv)*($ftmp - $fthresh) + $fthresh]
                            if {$f < 0.0} {
                                set f [expr 0.0 - $ftmp]
                            } else {
                                set f $ftmp
                            }
                        }
                        if {$f >= 0.0} {
                            set r [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid-$fthresh):0.0) + (($f < $fthresh)?0.0:($f < $fmid)?($f - $fthresh)/($fmid - $fthresh):1.0)]
                            set g [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fmid)?0.0:($f < $fmid + 1.0/$fslope)?1.0*($f-$fmid)*$fslope:1)]
                            #                            set b [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0)]
                            set b [expr (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0)]
                        } else {
                            set f [expr 0.0 - $f]
                            set b [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fthresh)?0.0:($f < $fmid)?($f - $fthresh)/($fmid - $fthresh):1.0)]
                            set g [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fmid)?0.0:($f < $fmid + 1.0/$fslope)?1.0*($f-$fmid)*$fslope:1)]
                            set r [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0 - ($f - $fthresh) / ($fmid - $fthresh):0)]
                        }
                        
                        if {$c >= 0} {
                            if {$::Module(verbose) && $newLut == "Heat"} {
                                # set b 1.0
                                puts [format "%4d: f=%3.5f r=%3.5f g=%3.5f b=%3.5f" $c $f $r $g $b]
                                
                            }
                            Lut($nextId,lut) SetTableValue $c $r $g $b 1.0
                        }
                    }
                    
                    
                    # now do the orange/red part of the LUT
                    if {$::Module(verbose) && $newLut == "Heat"} { puts "Red part\n" }
                    
                    set fthresh 0.7
                    set fmid .8
                    set tmpoffset 0.25
                    set fslope 4.0
                    for {set c $halfcols} {$c < $Lut($nextId,numberOfColors)} {incr c} {
                        set f [expr 1.0 * $c / $Lut($nextId,numberOfColors)]
                        set fcurv $f
                        if {$invphaseflag == 1} {
                            set f [expr 0.0 - $f]
                        } 
                        if {$truncphaseflag && $f < 0} {
                            set f 0.0
                        }
                        if {[expr abs($f)] > $fthresh && [expr abs($f)] < $fmid} {
                            set ftmp [expr abs($f)]
                            set c1 [expr 1.0 / ($fmid - $fthresh)]
                            if {$fcurv != 1.0} {
                                set c2tmp [expr ($fmid - $fthresh) * ($fmid - $fthresh)]
                                set c2 [expr ($fmid - $fthresh - $fcurv * $c1 * $c2tmp) / ((1.0 - $fcurv) * ($fmid - $fthresh))]
                            } else {
                                set c2 0.0
                            }
                            set ftmp [expr $fcurv * $c1 * ($ftmp - $fthresh) * ($ftmp - $fthresh) + $c2*(1.0 - $fcurv)*($ftmp - $fthresh) + $fthresh]
                            if {$f < 0.0} {
                                set f [expr 0.0 - $ftmp]
                            } else {
                                set f $ftmp
                            }
                        }
                        if {$f >= 0.0} {
                            set r [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid-$fthresh):0.0) + (($f < $fthresh)?0.0:($f < $fmid)?($f - $fthresh)/($fmid - $fthresh):1.0)]
                            set g [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fmid)?0.0:($f < $fmid + 1.0/$fslope)?1.0*($f-$fmid)*$fslope:1)]
                            set b [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0)]
                            #                            set b [expr (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0)]
                        } else {
                            set f [expr 0.0 - $f]
                            set b [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fthresh)?0.0:($f < $fmid)?($f - $fthresh)/($fmid - $fthresh):1.0)]
                            set g [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0-($f - $fthresh)/($fmid - $fthresh):0) + (($f < $fmid)?0.0:($f < $fmid + 1.0/$fslope)?1.0*($f-$fmid)*$fslope:1)]
                            set r [expr $tmpoffset * (($f < $fthresh)?1.0:($f < $fmid)?1.0 - ($f - $fthresh) / ($fmid - $fthresh):0)]
                        }
                        if {$c >= 0} {
                            if {$::Module(verbose) && $newLut == "Heat"} {
                                # set b 1.0
                                puts [format "%4d: f=%3.5f r=%3.5f g=%3.5f b=%3.5f" $c $f $r $g $b]
                                
                            }
                            Lut($nextId,lut) SetTableValue $c $r $g $b 1.0
                        }
                    }
                }
                default {
                    puts "Unknown look up table name $newLut, using Gray's values"
                    set Lut($nextId,hueRange) $Lut(0,hueRange)
                    set Lut($nextId,saturationRange) $Lut(0,saturationRange)
                    set Lut($nextId,valueRange) $Lut(0,valueRange)
                    set Lut($nextId,annoColor) $Lut(0,annoColor)
                    foreach param "NumberOfColors HueRange SaturationRange ValueRange" {
                        eval Lut($nextId,lut) Set${param} $Lut($nextId,[Uncap ${param}])
                    }
                    Lut($nextId,lut) SetRampToLinear
                    Lut($nextId,lut) Build
                }
            }
            # end of bypassing old look up tables
        }
        
    }

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersPickScalarsLut
# Pick which FreeSurfer color lookup table to use with active model.
# Names of the look up tables are held in $vtkFreeSurferReaders(lutNames)
# .ARGS
# widget parentButton the button to add a drop down menu to
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPickScalarsLut { parentButton } {
    global vtkFreeSurferReaders Gui

    set m $::Model(activeID)
    if {$m == ""} { 
        if {$::Module(verbose)} {
            puts "WARNING: vtkFreeSurferReadersPickScalarsLut Model(activeID) is empty! Returning..."
        }
        return 
    }

    catch "destroy .mFSpickscalarslut"
    eval menu .mFSpickscalarslut $Gui(WMA)

    set ren [lindex $::Module(Renderers) 0]
    set currlut [Model($m,mapper,$ren) GetLookupTable]

    set numcmds 0
    foreach l $::Lut(idList) {
        # is this a FS one?
        if {[lsearch $vtkFreeSurferReaders(lutNames) $::Lut($l,name)] != -1} {
            if {$::Module(verbose)} {
                puts "got an fs one, l = $l"
            }
            if { "Lut($l,lut)" == $currlut } {
                set labeltext "* $::Lut($l,name) *"
            } else {
                set labeltext "$::Lut($l,name)"
            }
            .mFSpickscalarslut insert end command -label $labeltext \
                -command "ModelsSetScalarsLut $m $l \; Render3D \; if {\"[info command .fsEditLut]\" == \".fsEditLut\"} { vtkFreeSurferReadersEditScalarsLut }"
            incr numcmds
        } else {
            if {$::Module(verbose)} {
                puts "skipping $l, name = $::Lut($l,name)"
            }
        }
    }
    # add any annotation luts
    set fssarLUTs [info commands fssarlut_$m]
    foreach l $fssarLUTs {
        if {$l == $currlut} {
            set labelText "* Model $m Annotation *"
        } else {
            set labelText "Model $m Annotation"
        }
        .mFSpickscalarslut insert end command -label $labelText \
                -command "Model($m,mapper,viewRen) SetLookupTable $l \; Render3D"
        incr numcmds
    }
    set x [expr [winfo rootx $parentButton] + 10]
    set y [expr [winfo rooty $parentButton] + 10]
    
    if {$numcmds > 0} {
        .mFSpickscalarslut post $x $y
    } else {
        DevWarningWindow "WARNING: no FS colour look up tables, call vtkFreeSurferReadersAddLuts"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetScalarFileName
# vtkFreeSurferReaders(scalarFileName) is set elsewhere.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetScalarFileName {} {
    global vtkFreeSurferReaders Volume
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersSetScalarFileName: scalar file name set to $vtkFreeSurferReaders(scalarFileName)"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersEditScalarsLut
# Builds and or pops up a frame that allows editing of the freesurfer look up tables.
# Starts from the lut used by the active model, reset it via the Pick Palette button on 
# the Display tab.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersEditScalarsLut {} {
    global vtkFreeSurferReaders Gui Model

    set m $Model(activeID)
    set ren [lindex $::Module(Renderers) 0]
    set currlut [Model($m,mapper,$ren) GetLookupTable]
    set w .fsEditLut

    # is it not build already?
    if {[info command $w] == ""} {
        toplevel $w 
        wm title $w "Edit FreeSurfer Color Lookup Tables"

        frame $w.f -bg $Gui(activeWorkspace)
        pack $w.f -side top -fill x

        DevAddLabel $w.f.lLut "[$currlut GetLutTypeString]"
        pack $w.f.lLut -side top

        foreach c {LowThresh HiThresh Reverse Truncate Offset Slope Blufact FMid} {
            frame $w.f.f$c -bg $Gui(activeWorkspace) -relief groove 
            pack $w.f.f$c -side top -fill x -expand 1

            set f $w.f.f$c
            DevAddLabel $f.l$c $c
            DevAddEntry vtkFreeSurferReaders LUT$c $f.e$c
            bind $f.e$c <Return> "vtkFreeSurferReadersSetLutParam $c"
            pack $f.l$c $f.e$c -side left -expand 1
        }

        # this relies on the fact that the set lut type to functions sets all the values, call 
        # something to trigger a remapping of all the scalars
        DevAddButton $w.f.bReset "Reset" "$currlut SetLutTypeTo[$currlut GetLutTypeString] ; vtkFreeSurferReadersEditScalarsLut ; Render3D"
        # pack $w.f.bReset -side top -pady $Gui(pad) -expand 1

        DevAddButton $w.f.bClose "Close" "wm withdraw $w"
        pack $w.f.bClose -side top -pady $Gui(pad) -expand 1
    } else {
        wm deiconify $w
    }
    # now set all the vars
    $w.f.lLut configure -text "[$currlut GetLutTypeString]"
    foreach c {LowThresh HiThresh Reverse Truncate Offset Slope Blufact FMid} {
        set vtkFreeSurferReaders(LUT${c}) [$currlut Get${c}]
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersSetLutParam
# Set a vtkFSLookupTable class parmeter, for the active model's look up table
# .ARGS
# string param the parameter to change, read from the vtkFreeSurferReaders areay as 
# LUTparam
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersSetLutParam {param} {
    global vtkFreeSurferReaders Model

    set m $Model(activeID)
    set ren [lindex $::Module(Renderers) 0]
    set l [Model($m,mapper,$ren) GetLookupTable]
    
    $l Set${param} $vtkFreeSurferReaders(LUT${param})

    Render3D

}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersReadScalars
# read in the scalar overlays associated with a model id
# .ARGS
# int m the model id 
# path fileName optional, can be built from the ModelFileName and the assocFiles
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersReadScalars { m {fileName ""} } {
   
    global vtkFreeSurferReaders Model Module

    # a list of files to load
    set scalarFileList ""

    if {$m == "" || $m == -1} {
        puts "ReadScalars: invalid model id $m. Load or set an active model"
        return
    }

    if {$fileName == ""} {
        # build up the list of files to load

        # check if there's more than one
        set numScalars 0
        foreach s $vtkFreeSurferReaders(scalars) {
            if {[lsearch $vtkFreeSurferReaders(assocFiles) $s] != -1} {
                incr numScalars
            }
        }
        if {$::Module(verbose)} {
            puts "Have $numScalars scalar arrays associated with this model, call:\nvtkFreeSurferReadersSetModelScalar $m scalarName"
        }
        foreach s $vtkFreeSurferReaders(scalars) {
            if {[lsearch $vtkFreeSurferReaders(assocFiles) $s] != -1} {
                lappend scalarFileList [file rootname $vtkFreeSurferReaders(ModelFileName)].$s
            }
        }
    } else {
        # check to see how many scalars are there already
        # set numScalars [expr [[$Model($m,polyData) GetPointData] GetNumberOfArrays] + 1]
        if {$::Module(verbose)} {
            # puts "Reading in from file(s), starting with scalar number $numScalars"
        }
        lappend scalarFileList $fileName
        set numScalars [llength $scalarFileList]
        if {$::Module(verbose)} {
            puts "Adding $numScalars for this model"
        }
    }

    set numScalarsAdded 0
    foreach scalarFileName $scalarFileList {
        if [file exists $scalarFileName] {
            set s [string trimleft [file extension $scalarFileName] "."]
            puts "Model $m: Reading in file $s associated with this surface: $scalarFileName"
            # need to delete these so that if close the scene and reopen a surface file, these won't still exist
            if {$::Module(verbose)} {
                puts "Deleting Model($m,floatArray$s)..."
            }
            catch "Model($m,floatArray$s) Delete"
            vtkFloatArray Model($m,floatArray$s)
            Model($m,floatArray$s) SetName $s

            # should check here that using the right kind of surface scalar reader
            if {$s == "w"} {
                catch "Model($m,swr$s) Delete"
                vtkFSSurfaceWFileReader Model($m,swr$s)

                Model($m,swr$s) SetFileName $scalarFileName
                Model($m,swr$s) SetOutput Model($m,floatArray$s)

                # set the total number of vertices in the associated model
                Model($m,swr$s) SetNumberOfVertices [[$Model($m,polyData) GetPointData] GetNumberOfTuples]

                # set up a progress observer
                Model($m,swr$s) AddObserver StartEvent MainStartProgress
                Model($m,swr$s) AddObserver ProgressEvent "MainShowProgress Model($m,swr$s)"
                Model($m,swr$s) AddObserver EndEvent       MainEndProgress
                set ::Gui(progressText) "Reading $s"
                
                set retval [Model($m,swr$s) ReadWFile]
                if {[vtkFreeSurferReadersCheckWError $retval] != 0} {
                    puts "vtkFreeSurferReadersReadScalars: Error reading $fileName, returning..."
                    return
                }
            } else {
                catch "Model($m,ssr$s) Delete"
                vtkFSSurfaceScalarReader Model($m,ssr$s)

                # set up progress reporting
                Model($m,ssr$s) AddObserver StartEvent MainStartProgress
                Model($m,ssr$s) AddObserver ProgressEvent "MainShowProgress Model($m,ssr$s)"
                Model($m,ssr$s) AddObserver EndEvent       MainEndProgress
                set ::Gui(progressText) "Reading $s"

                Model($m,ssr$s) SetFileName $scalarFileName
                # this doesn't work on solaris, can't cast float array to vtkdataobject
                Model($m,ssr$s) SetOutput Model($m,floatArray$s)
                
                Model($m,ssr$s) ReadFSScalars
            }
            MainEndProgress

            # if there's going to be more than one, use add array, otherwise just set it
            if {$numScalars == 1} {
                [$Model($m,polyData) GetPointData] SetScalars Model($m,floatArray$s)
                [$Model($m,polyData) GetPointData] SetActiveScalars $s
                # increment it, as otherwise the number added will still be zero, 
                # and this won't be visible
                incr numScalarsAdded 
            } else {
                if {$::Module(verbose)} {
                    puts "Adding scalar named $s to model id $m"
                }
                [$Model($m,polyData) GetPointData] AddArray Model($m,floatArray$s)
                # may have some missing files
                incr numScalarsAdded 
                if {$numScalarsAdded == 1} {
                    # set the first one active
                    [$Model($m,polyData) GetPointData] SetActiveScalars $s                        
                }
            }
                
            # set the lookup table for the scalar field
            if {$::Module(verbose)} {
                puts "vtkFreeSurferReadersReadScalars: checking scalar file extension $s to decide on LUT to use"
            }
            set lutIndex -1
            if {$s == "fs"} {
                # field sign
                set lutIndex [lsearch $vtkFreeSurferReaders(lutNames) "BlueRed"]
               
            }
            if {$s == "curv" || $s == "avg_curv" || $s == "sulc" || $s == "thickness"} {
                set lutIndex [lsearch $vtkFreeSurferReaders(lutNames) "GreenRed"]
            }
            if {$s == "retinotopy"} {
                # use colour wheel 
                set lutIndex [lsearch $vtkFreeSurferReaders(lutNames) "ColorWheel"]
            }
            if {$lutIndex == -1} {
                # use default 
                puts "Warning: scalar extension $s doesn't match known extensions, using Heat colour scale"
                set lutIndex [lsearch $vtkFreeSurferReaders(lutNames) "Heat"]
            }
            if {$lutIndex != -1} {
                set lutID [lindex $vtkFreeSurferReaders(lutIDs) $lutIndex]
                if {$::Module(verbose)} {
                    puts "vtkFreeSurferReadersReadScalars: calling ModelsSetScalarsLut $m $lutID, for [lindex $vtkFreeSurferReaders(lutNames) $lutIndex]"
                }
                ModelsSetScalarsLut $m $lutID
            }

            # add the filename to the model node
            if {$::Module(verbose)} {
                puts "Adding scalar file name $scalarFileName to model node $m"
            }
            Model($m,node) AddScalarFileName $scalarFileName

            # may need to set reader output to "" and delete here
        } else {
            DevWarningWindow "Scalar file does not exist: $scalarFileName"
        }
    }

    if {$numScalarsAdded >= 1} {
        set Model(scalarVisibility) 1
        Model($m,node) SetScalarVisibility 1
        Model($m,node) SetVisibility 1
        # auto range on the scalar
        set Model(scalarVisibilityAuto) 1
    
        set range [$Model($m,polyData) GetScalarRange]
        set Model(scalarLo) [lindex $range 0]
        set Model(scalarHi) [lindex $range 1]
        if {$::Module(verbose)} {
            puts "Model $m has scalars, range = $range"
        }
    }
    
    if {$fileName != ""} {
        # if this wasn't called from vtkFreeSurferReadersBuildSurface, need to set a few things
        MainModelsSetScalarVisibility $m $Model(scalarVisibility)
        MainModelsSetScalarRange $m $Model(scalarLo) $Model(scalarHi)
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadScalarFile
# Sets up and reads in a scalar file for the active model, using vtkFreeSurferReaders(scalarFileName). Can be
# called to load a file from the command line, if fileName is not an empty string, it will over ride 
# vtkFreeSurferReaders(scalarFileName), otherwise load scalarFileName
# .ARGS
# path fileName path to a scalar file, defaults to empty string.
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadScalarFile { {fileName ""} } {
    global vtkFreeSurferReaders Model

    if {$fileName != ""} {
        set vtkFreeSurferReaders(scalarFileName) $fileName 
    }

    if {[file exists $vtkFreeSurferReaders(scalarFileName)] == 0} {
        puts "Load FreeSurfer Scalar: file \"$vtkFreeSurferReaders(scalarFileName)\" does not exist!"
        return
    }

    # check that there's an active model
    set m $Model(activeID)
    if {$m == ""} { 
        if {$::Module(verbose)} {
            puts "WARNING: vtkFreeSurferReadersLoadScalarFile Model(activeID) is empty! Returning..."
        }
        return 
    }

    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersLoadScalar: for model id $m, loading scalar file name = $vtkFreeSurferReaders(scalarFileName)"
    }

    # read in the scalar for this model 
    vtkFreeSurferReadersReadScalars $m $vtkFreeSurferReaders(scalarFileName)
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersLoadAnnotationFile
# Sets up and reads in an annotation file for the active model, using vtkFreeSurferReaders(annotFileName). Can be
# called to load a file from the command line, if fileName is not an empty string, it will over ride 
# vtkFreeSurferReaders(annotFileName), otherwise load annotFileName
# .ARGS
# path fileName path to a scalar file, defaults to empty string.
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersLoadAnnotationFile { {fileName ""} } {
    global vtkFreeSurferReaders Model

    if {$fileName != ""} {
        set vtkFreeSurferReaders(annotFileName) $fileName 
    }

    if {[file exists $vtkFreeSurferReaders(annotFileName)] == 0} {
        puts "Load FreeSurfer Annotation: file \"$vtkFreeSurferReaders(annotFileName)\" does not exist!"
        return
    }

    # check that there's an active model
    set m $Model(activeID)
    
    if {$::Module(verbose)} {
        puts "vtkFreeSurferReadersLoadAnnotation: for model id $m, loading annot file name = $vtkFreeSurferReaders(annotFileName)"
    }

    # read in the annotation for this model 
    set a [string trimleft [file extension [file rootname $vtkFreeSurferReaders(annotFileName)]] "."]
    vtkFreeSurferReadersReadAnnotation $a $m $vtkFreeSurferReaders(annotFileName)
}
