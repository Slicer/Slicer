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
#   vtkFreeSurferReadersPlotBuildPointList  pointID scalarVar
#   vtkFreeSurferReadersPickPlot widget x y
#   vtkFreeSurferReadersPickScalar widget x y
#   vtkFreeSurferReadersShowScalarValue mid pid val col
#==========================================================================auto=



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
# .PROC vtkFreeSurferReadersGDFInit
# Initializes the list of valid markers and colours for GDF
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFInit { } {
    global vtkFreeSurferReaders 

    # check to see if Init was called already
    if {[info exists vtkFreeSurferReaders(gbLibLoaded)] == 1 && $vtkFreeSurferReaders(gbLibLoaded) == 1} {
        # clear up the windows
        # puts "Clearing up windows, id list = $vtkFreeSurferReaders(gGDF,lID)"
        foreach winID $vtkFreeSurferReaders(gGDF,lID) {
            # puts "Closing window for id $winID"
            vtkFreeSurferReadersGDFPlotCBCloseWindow $winID
        }
    }
    set vtkFreeSurferReaders(verbose) 0
    set vtkFreeSurferReaders(kValid,lMarkers)  {square circle diamond plus cross splus scross triangle}
    set vtkFreeSurferReaders(kValid,lColors) {red blue green yellow black purple orange pink brown}
    # clear the list
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
        puts "Declared a vtkGDFReader"
    }
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersExit
# Cleans up any vtk variables that were declared globally, called on exit from the C++ module that sourced it.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersExit {} {
    global vtkFreeSurferReaders
    catch "vtkFreeSurferReaders(gdfReader) Delete"
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
        if {$::vtkFreeSurferReaders(verbose)} {
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
                    if {$::vtkFreeSurferReaders(verbose)} { puts "Plot parse header: set value to \"$vtkFreeSurferReaders(gGDF,$ID,subjects,$nSubject,variables,$nVariable,value)\""}
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

    if {$::vtkFreeSurferReaders(verbose)} {
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
# int dID the id of the data file to plot, can be found in $::vtkFreeSurferReaders(gGDF,dataID). If a string, is treated as the model name, found in vtkFreeSurferReaders(plot,modelID) linked with it's data id, and converted to an int
# float x the x of the vertex, optional
# float y the y of the vertex, optional
# float z the z of the vertex, optional
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotPlotData { iID dID {x 0.0} {y 0.0} {z 0.0} } {
    global vtkFreeSurferReaders

    if {[info exists vtkFreeSurferReaders(verbose)] == 0} {
        set vtkFreeSurferReaders(verbose) 0
    }
    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersPlotPlotData: iID = $iID, dID = $dID"
    }
    # figure out if the dID is an int or a text string
    if { [string is integer $dID] == 0} {
        # find the model id 
        if {[info exists vtkFreeSurferReaders(plot,modelID)]} {
            if {$::vtkFreeSurferReaders(verbose)} {
                puts "vtkFreeSurferReadersPlotPlotData: trying to link data id $dID with an integer value from $vtkFreeSurferReaders(plot,modelID)"
            }
            foreach {dataID modelID} $vtkFreeSurferReaders(plot,modelID) {
                if {$::vtkFreeSurferReaders(verbose)} {
                    puts "Checking data id $dataID and modelID $modelID"
                }
                if {$modelID == $dID} {
                    set dID $dataID
                    if {$::vtkFreeSurferReaders(verbose)} {
                        puts "Found data id $dID for model $modelID"
                    }
                }
            }
        } else {
            puts "vtkFreeSurferReadersPlotPlotData: data not initialised for model $dID yet."
            return
        }
    }
    # Don't plot if the window isn't built or we don't have data.
    if { ![info exists vtkFreeSurferReaders(gWidgets,$dID,bWindowBuilt)] ||
         !$vtkFreeSurferReaders(gWidgets,$dID,bWindowBuilt) } {
        if {$::vtkFreeSurferReaders(verbose)} {
            puts "vtkFreeSurferReadersPlotPlotData: the plot window isn't built yet for $dID"
        }
        return "Plot window wasn't built yet for id $dID"
    }
    if { ![info exists vtkFreeSurferReaders(gGDF,$dID,bReadHeader)] ||
         !$vtkFreeSurferReaders(gGDF,$dID,bReadHeader) } {
        if {$::vtkFreeSurferReaders(verbose)} {
            puts "vtkFreeSurferReadersPlotPlotData: no data for data id $dID"
        }
        return "No data for id $dID"
    }

    if {$::vtkFreeSurferReaders(verbose)} { puts "\nvtkFreeSurferReadersPlotPlotData iID = $iID, dID = $dID" }

    # update the info label variable
    if {$x == 0.0 && $y == 0.0 && $z == 0.0} {
        # nothing was passed in, do we have a saved xyz?
        if { ![info exists vtkFreeSurferReaders(gPlot,$iID,state,XYZ)]} {
            vtkFreeSurferReadersPlotSetInfo $dID "Vertex number $iID"
        } else {
            # use the saved xyz for this vertex
            vtkFreeSurferReadersPlotSetInfo $dID "$vtkFreeSurferReaders(gPlot,$iID,state,XYZ) Vertex number $iID"
        }
    } else {       
        vtkFreeSurferReadersPlotSetInfo $dID "($x, $y, $z) Vertex number $iID"
        if { ![info exists vtkFreeSurferReaders(gPlot,$iID,state,XYZ)] ||
             $vtkFreeSurferReaders(gPlot,$iID,state,XYZ) != "($x, $y, $z)"} {
            # save the RAS 
            set vtkFreeSurferReaders(gPlot,$iID,state,XYZ) "($x, $y, $z)"
        }
    }

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
        if {$::vtkFreeSurferReaders(verbose)} { puts "deleting element $element" }
        $gw element delete $element
    }
    set lMarkers [$gw marker names *]
    foreach marker $lMarkers {
        if {$::vtkFreeSurferReaders(verbose)} { puts "deleting marker $marker" }
        $gw marker delete $marker
    }
    
    # If we have no points, return.
    if { ![info exists vtkFreeSurferReaders(gPlot,$iID,state,lPoints)] || 
         [llength $vtkFreeSurferReaders(gPlot,$iID,state,lPoints)] == 0 } {
        if {$::vtkFreeSurferReaders(verbose)} {
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

            if {$::vtkFreeSurferReaders(verbose)} { puts "Starting loop: nClass = $nClass" }

            for { set nSubj 0 } { $nSubj < $vtkFreeSurferReaders(gGDF,$dID,cSubjects) } { incr nSubj } {
                set classIndex [vtkFreeSurferReadersGDFPlotGetClassIndexFromLabel $dID $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,nClass)]
                if {$::vtkFreeSurferReaders(verbose)} {
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
            if {$::vtkFreeSurferReaders(verbose)} { puts "creating element $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label)" }
            $gw element create $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,label) \
                -data $lData \
                -symbol $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,marker) \
                -color $color -linewidth 0 -outlinewidth 1 -hide $bHide \
                -activepen activeElement
            
            if {$::vtkFreeSurferReaders(verbose)} {
                puts "By classes: data = $lData"
            }
        }
        if {$::vtkFreeSurferReaders(verbose)} {
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
            if {$::vtkFreeSurferReaders(verbose)} { puts "creating element $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,id)" }
            $gw element create $vtkFreeSurferReaders(gGDF,$dID,subjects,$nSubj,id) \
                -data [list $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) \
                           $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)] \
                -symbol $vtkFreeSurferReaders(gGDF,$dID,classes,$classIndex,marker) \
                -color $color -linewidth 0 -outlinewidth 1 -hide $bHide \
                -activepen activeElement
            if {$::vtkFreeSurferReaders(verbose)} { puts "Created gw  element for subj $nSubj: $vtkFreeSurferReaders(gPlot,$dID,state,data,subjects,$nSubj,variable) $vtkFreeSurferReaders(gPlot,$iID,state,data,subjects,$nSubj,measurement)" }
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
                if {$::vtkFreeSurferReaders(verbose)} {
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
                if {$::vtkFreeSurferReaders(verbose)} {
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
                        set slope [expr (($slopeArray($thisClass,N) * $slopeArray($thisClass,sumxy)) - ($slopeArray($thisClass,sumx) * $slopeArray($thisClass,sumy))) / ( ($slopeArray($thisClass,N) * $slopeArray($thisClass,sumxx)) - ($slopeArray($thisClass,sumx)*$slopeArray($thisClass,sumx)))] 
                        if {$::vtkFreeSurferReaders(verbose)} {
                            puts "Got slope $slope for class $thisClass"
                        }
                    } else {
                        set slope 0
                    }
                } 

                if {$offsetArray($thisClass,N) > 0} {
                    set offset  [expr (($offsetArray($thisClass,sumxx) * $offsetArray($thisClass,sumy)) - ($offsetArray($thisClass,sumx) * $offsetArray($thisClass,sumxy)))/(($offsetArray($thisClass,N) * $offsetArray($thisClass,sumxx)) - ($offsetArray($thisClass,sumx) * $offsetArray($thisClass,sumx)))]
                    if {$::vtkFreeSurferReaders(verbose)} {
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
                if {$::vtkFreeSurferReaders(verbose)} { puts "New way: class $thisClass, offset = $offset, slope = $slope (xmin $xmin, xmax $xmax)"} 
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
        
                if {$::vtkFreeSurferReaders(verbose)} { 
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
                            # if {$::vtkFreeSurferReaders(verbose)} { puts "subject id = $sid x = $x y = $y" }
                        }
                    }
                
                    if { $cGood > 0 } {
                    
                        if {$vtkFreeSurferReaders(gGDF,$dID,gd2mtx) == "dods"} {
                            if {$::vtkFreeSurferReaders(verbose)} { 
                                puts "class $thisClass, sumx = $sumx, sumy = $sumy, cGood = $cGood, sumxx = $sumxx, sumxy = $sumxy"
                            }
                            set slope [expr (($cGood * $sumxy) - ($sumx * $sumy)) / ( ($cGood * $sumxx) - ($sumx*$sumx))]
                            set offset [expr (($sumxx * $sumy) - ($sumx * $sumxy))/(($cGood * $sumxx) - ($sumx * $sumx))]
                            
                            set x1 $xmin
                            set y1 [expr ($slope * $x1) + $offset]
                            set x2 $xmax
                            set y2 [expr ($slope * $x2) + $offset]
                            if {$::vtkFreeSurferReaders(verbose)} { puts "Original: class $thisClass, offset = $offset, slope = $slope (xmin $xmin, xmax $xmax)"} 
                            $gw marker create line \
                                -coords [list $x1 $y1 $x2 $y2] \
                                -outline $vtkFreeSurferReaders(gGDF,$dID,classes,$nClass,color) \
                                -dashes {2 2}
                        }
                    } else {
                        if {$::vtkFreeSurferReaders(verbose)} { 
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
            if {$::vtkFreeSurferReaders(verbose)} { puts "lPoint = $lPoint" }
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

    if {$::vtkFreeSurferReaders(verbose)} { 
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

    if {$::vtkFreeSurferReaders(verbose)} {
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

    destroy .fsgdf-$iID

    set vtkFreeSurferReaders(gWidgets,$iID,bWindowBuilt) 0
}

#-------------------------------------------------------------------------------
# .PROC vtkFreeSurferReadersGDFPlotCBCloseAllWindows
# Find top level .fsgdf-* windows numbered 0-99 and destroy them
# TODO: check the window id list in vtkFreeSurferReaders(gGDF,lID)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersGDFPlotCBCloseAllWindows {} {
    set windowList [info commands {.fsgdf-[0-9]}]
    lappend windowList [info commands {.fsgdf-[0-9][0-9]}]
    foreach w $windowList {
        # puts "Destroying $w"
        destroy $w
        # get the id
        set retval [regexp {.fsgdf-([0-9]+)} $w matchVar id]
        if {$retval == 1} {
            # puts "Setting windowbuilt flag to 0 for id = $id"
            set vtkFreeSurferReaders(gWidgets,$id,bWindowBuilt) 0
        }
    }
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
        puts "vtkFreeSurferReadersGDFPlotPrint: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotShowWindow: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotHideWindow: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetVariable: ID $iID not found: $errmsg"
        return
    }
    if { [lsearch $vtkFreeSurferReaders(gGDF,lID) $iID] == -1 } { 
        puts "vtkFreeSurferReadersPlotSetVariable: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetMode: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetNthClassMarker: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetNthClassColor: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetPoint: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotBeginPointList: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotAddPoint: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotEndPointList: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSetInfo: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSaveToTable: ID $iID not found"
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
        puts "vtkFreeSurferReadersPlotSaveToPostscript: ID $iID not found"
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

    if {$vtkFreeSurferReaders(verbose) == 1} {
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

    if {[info vars ::vtkFreeSurferReaders(verbose)] == ""} {
      set ::vtkFreeSurferReaders(verbose) 1
    }

    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersPlotApply: starting, calling GDFInit"
    }
    vtkFreeSurferReadersGDFInit
    set vtkFreeSurferReaders(gGDF,dataID) [vtkFreeSurferReadersPlotParseHeader $vtkFreeSurferReaders(PlotFileName)]
    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersPlotApply: have model id $mid and data id $vtkFreeSurferReaders(gGDF,dataID)"
    }

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

    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersGDFPlotApply: read the header $vtkFreeSurferReaders(PlotFileName), now about to try reading the data file $vtkFreeSurferReaders(VolumeFileName)"
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
            # make sure it's readable
            if {[file exists $datafilename] == 0} {
                # try concatenating the path from the GDF file and the data file name 
                set datafilename "[file dirname $vtkFreeSurferReaders(PlotFileName)]/[file tail $datafilename]"
                if {[file exists $datafilename] == 0} {
                    puts "Cannot find data file, even tried $datafilename"
                    return -1
                }
            }
            vtkITKArchetypeImageSeriesVectorReaderFile reader
            reader SetArchetype $datafilename
            reader SetOutputScalarTypeToNative
            reader SetDesiredCoordinateOrientationToNative
            reader Update
            if {$::vtkFreeSurferReaders(verbose)} { puts "Read the data file $datafilename" }
            set imageData [reader GetOutput]
            set numPoints [$imageData GetNumberOfPoints]
            if {$::vtkFreeSurferReaders(verbose)} { puts "Got number of points $numPoints" }
            set vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) [[$imageData GetPointData] GetScalars]
            # need to register it to hold onto it over tcl calls
            $vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars) Register $::slicer3::Application
            set scalarsVar vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),scalars)
            if {$::vtkFreeSurferReaders(verbose)} { puts "Set scalarsVar to $scalarsVar, data id = $vtkFreeSurferReaders(gGDF,dataID)" }
            reader Delete
        }
    }
    
    if {$::vtkFreeSurferReaders(verbose)} {
        if {[info var scalarsVar] != ""} {
            puts "vtkFreeSurferReadersGDFPlotApply: read data file, got id $scalarsVar"
        } else {
            puts "vtkFreeSurferReadersGDFPlotApply: read data file, but scalars var is not valid"
        }
    }

    # link it in with the active model
    # set modelname [$Module(vtkFreeSurferReaders,fPlot).fModel.mbActive cget -text]
    # get the model id of the active model (assume that the linking still works), otherwise
    # need to figure out which of $Model(idList) is $modelname
    # set mid $::Model(activeID)
    # set vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),modelID) $mid
    lappend vtkFreeSurferReaders(plot,modelID) $vtkFreeSurferReaders(gGDF,dataID) $mid
    if {$::vtkFreeSurferReaders(verbose)} {
        puts "Linking data id $vtkFreeSurferReaders(gGDF,dataID) to model id: $mid"
    }

    # set it to be pickable
    #::Model($mid,actor,viewRen) SetPickable 1

    #bind $::Gui(fViewWin) <ButtonRelease-1> {vtkFreeSurferReadersPickPlot %W %x %y}


    # now add the points to the lPoint list for vertex 0
    if {$::vtkFreeSurferReaders(verbose)} {
        puts "Building point list with 0, scalarsVar = $scalarsVar"
    }
    vtkFreeSurferReadersPlotBuildPointList 0 $scalarsVar 

    # now build the plot window
    if {$::vtkFreeSurferReaders(verbose)} {
        puts "Building plot window for $vtkFreeSurferReaders(gGDF,dataID), data id $scalarsVar"
    }
    vtkFreeSurferReadersGDFPlotBuildWindow $vtkFreeSurferReaders(gGDF,dataID)

    # and plot stuff by setting the mode to the default
    set vtkFreeSurferReaders(gPlot,$vtkFreeSurferReaders(gGDF,dataID),state,nVariable) 0
    vtkFreeSurferReadersPlotSetMode $vtkFreeSurferReaders(gGDF,dataID)

    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersPlotApply: returning data id $vtkFreeSurferReaders(gGDF,dataID)"
    }
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
    if {$::vtkFreeSurferReaders(verbose)} {
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
    if {$::vtkFreeSurferReaders(verbose)} { puts "vtkFreeSurferReadersUncompressMGH: letting the mgh reader uncompress it" }
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
# .PROC vtkFreeSurferReadersPlotBuildPointList 
# Adds data points to the plotter's point list
# .ARGS
# int pointID the vertex id of the window to plot
# int scalarVar the variable that points to the scalar volume data
# .END
#-------------------------------------------------------------------------------
proc vtkFreeSurferReadersPlotBuildPointList {pointID scalarVar} {
    global vtkFreeSurferReaders

    if {$::vtkFreeSurferReaders(verbose)} {
        puts "vtkFreeSurferReadersPlotBuildPointList pointID $pointID, scalarVar $scalarVar"
    }

    # clear out the list
    vtkFreeSurferReadersPlotBeginPointList $pointID

    if {$::vtkFreeSurferReaders(verbose)} { puts "vtkFreeSurferReadersPlotBuildPointList: getting the scalar vars number of tuples and components: $scalarVar" }

    # add to it, just the subject data for this vertex
    if {[info command [subst $$scalarVar]] != ""} {
        set numPoints [[subst $$scalarVar] GetNumberOfTuples]
        set numSubjects [[subst $$scalarVar] GetNumberOfComponents]
    } else {
        set numPoints 0
        set numSubjects 0
    }
    if {$::vtkFreeSurferReaders(verbose)} { puts "BuildPointList: got numPoints = $numPoints, numSubjects = $numSubjects" }


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
    set verb $::vtkFreeSurferReaders(verbose) 
    set ::vtkFreeSurferReaders(verbose) 0

    if {$::vtkFreeSurferReaders(verbose)} { 
        puts "\nvtkFreeSurferReadersPickPlot: widget = $widget, x = $x, y = $y"
        puts "Getting picked point for plotting" 
    }

    
    if {$testPickers} {
        # vtkFastCellPicker
        set fastPickMs [time {set retval [SelectPick Select(picker) $widget $x $y]}]
        if {$::vtkFreeSurferReaders(verbose)} {
            puts "vtkFastCellPicker took $fastPickMs, retval = $retval, tolerance = [Select(picker) GetTolerance]"
            puts "\tvtkFastCellPicker cell id = [Select(picker) GetCellId]"
        }
        

        # vtkPickFilter
        catch "wldPicker Delete"
        vtkWorldPointPicker wldPicker
        set wldPickMS [time {set retval [wldPicker Pick $x $y 0.0 viewRen]}]
        if {$::vtkFreeSurferReaders(verbose)} {
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
        if {$::vtkFreeSurferReaders(verbose)} {          
            puts "\tvtkPickFilter get cell id time = $cellpickMs"
            puts "\tvtkPickFilter using model 0"
            puts "\tvtkPickFilter cell id = $cellID"
        }


        if {0} {
            # this is way too slow
            # vtkCellPicker
            set pickMs [time {set retval [SelectPick Point(picker) $widget $x $y]}]
            if {$::vtkFreeSurferReaders(verbose)} {
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
                if {$::vtkFreeSurferReaders(verbose)} {
                    puts "\tPoint(model) = $Point(model)"
                    # cellId = $cellId
                }
            }
        }
    }
    # use a point picker to get the point the cursor was over, then pass that in to the plotter
    set pointpickMs [time {set retval [SelectPick Select(ptPicker) $widget $x $y]}]
    if {$::vtkFreeSurferReaders(verbose)} { 
        puts "vtkPointPicker took $pointpickMs" 
        puts "\ttolerance = [Select(ptPicker) GetTolerance]"
    }
    if {$retval != 0} {
        set pid [Select(ptPicker) GetPointId]
        if {$::vtkFreeSurferReaders(verbose)} {
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
            if {$::vtkFreeSurferReaders(verbose)} {
                puts "\tPlotting point $pid, data id = $vtkFreeSurferReaders(gGDF,dataID)"
            }
            set ::vtkFreeSurferReaders(verbose) $verb
            vtkFreeSurferReadersPlotPlotData $pid $vtkFreeSurferReaders(plot,$vtkFreeSurferReaders(gGDF,dataID),modelID) 
        }
    } else {
        if {$::vtkFreeSurferReaders(verbose)} {
            puts "\tSelect(ptPicker) didn't find anything at $x $y"
        }
    }
    set ::vtkFreeSurferReaders(verbose) $verb
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

    if {$::vtkFreeSurferReaders(verbose)} { puts "vtkFreeSurferReadersPickScalar: widget = $widget, x = $x, y = $y"
    }

    # use a point picker to get the point the cursor was over, 
    # then pass that in to the display widget
    set pointpickMs [time {set retval [SelectPick Select(ptPicker) $widget $x $y]}]
    if {$::vtkFreeSurferReaders(verbose)} { 
        puts "vtkPointPicker took $pointpickMs" 
    }
    if {$retval != 0} {
        set pid [Select(ptPicker) GetPointId]
#        set mid $Model(activeID)
        # get the model
        set actors [Select(ptPicker) GetActors]
        if {$::vtkFreeSurferReaders(verbose)} { puts "Actors = $actors" }
        if {[$actors GetNumberOfItems] > 0} {
            $actors InitTraversal
            set a [$actors GetNextActor]
            if {[regexp {^Model[(](.*),actor,viewRen[)]} $a matchVar mid] == 1} {
                if {$::vtkFreeSurferReaders(verbose)} { puts "got model id $mid" }
            } else {
                if {$::vtkFreeSurferReaders(verbose)} { puts "found no model id, using 0" }
                set mid 0
            }
        }

        # check against the scalar array' size
        if {[info exists Model($mid,polyData)] == 1} {
            set ptData [$Model($mid,polyData) GetPointData]
            set scalars [$ptData GetScalars]
            if {$scalars != ""} {
                set numTuples [$scalars GetNumberOfTuples]
                if {$::vtkFreeSurferReaders(verbose)} { puts "pid = $pid, numTuples = $numTuples for model $mid" }
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
                            if {$::vtkFreeSurferReaders(verbose)} {
                                puts "can't find array ${mid}vtkFreeSurferReadersLabels"
                            }
                        }
                    }
                    
                    if {$::vtkFreeSurferReaders(verbose)} { puts "pid $pid val = $val, colour = $col, colourName = $colourName (scalars name = $scalarsName)" }
                    vtkFreeSurferReadersShowScalarValue $mid $pid $val $col $colourName
                } else {
                    if {$::vtkFreeSurferReaders(verbose)} { puts "pid $pid out of range of $numTuples for model $mid" }
                }
            } else {
                if {$::vtkFreeSurferReaders(verbose)} { puts "No scalars in model $mid"}
            }
        } else {
            if {$::vtkFreeSurferReaders(verbose)} {
                puts "no poly data for model $mid"
            }
        }
    } else {
        if {$::vtkFreeSurferReaders(verbose)} {
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

    if {$::vtkFreeSurferReaders(verbose)} {  puts "mid = $mid, pid = $pid, val = $val, colour name = $colourName" }

    set w .topScalars${mid}
    if {[info command $w] != ""} {
        if {$::vtkFreeSurferReaders(verbose)} { puts "Already have a $w" }
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





