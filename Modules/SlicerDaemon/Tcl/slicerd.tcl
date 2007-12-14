#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: slicerd.tcl,v $
#   Date:      $Date: 2006/01/30 20:47:46 $
#   Version:   $Revision: 1.10 $
# 
#===============================================================================
# FILE:        slicerd.tcl
# PROCEDURES:  
#==========================================================================auto=

#
# experimental slicer daemon - sp 2006-06-09
# - ported to slicer3
#
# experimental slicer daemon - sp 2005-09-23 
# - meants to be as simple as possible
# - only one server socket listening at a time
# - checks with the user on first connection to see if it's okay to accept connections
#

set __comment {

    # a noop -- just copy image onto itself
    c:/pieper/bwh/slicer2/latest/slicer2/Modules/vtkQueryAtlas/tcl/slicerget.tcl 1 \
        | c:/pieper/bwh/slicer2/latest/slicer2/Modules/vtkQueryAtlas/tcl/slicerput.tcl noop

    # put external data into slicer
    unu 1op abs -i d:/data/bunny-small.nrrd | c:/pieper/bwh/slicer2/latest/slicer2/Modules/vtkQueryAtlas/tcl/slicerput.tcl 

    # run an external command and put the data back into slicer
    c:/pieper/bwh/slicer2/latest/slicer2/Modules/vtkQueryAtlas/tcl/slicerget.tcl 1 \
        | unu 1op abs -i - \
        | c:/pieper/bwh/slicer2/latest/slicer2/Modules/vtkQueryAtlas/tcl/slicerput.tcl abs
    
}

#
# returns a listening socket on given port or uses default
# - local only for security, use ssh tunnels for remote access
#
proc slicerd_start { {port 18943} } {

    set ::SLICERD(port) $port
    set ret [ catch {set ::SLICERD(serversock) [socket -server slicerd_sock_cb $port]} res]

    if { $ret } {
        puts "Warning: could not start slicer daemon at default port (probably another slicer daemon already running on this machine)."
    }
}

#
# shuts down the socket
# - frees the tcl helper if it exists
#
proc slicerd_stop { } {

    if { ![info exists SLICERD(serversock)] } {
        return
    }
    set _tcl ::tcl_$SLICERD(serversock)
    catch "$_tcl Delete"
    catch "unset ::SLICERD(approved)"

    close $sock
}

#
# accepts new connections
#
proc slicerd_sock_cb { sock addr port } {

    if { ![info exists ::SLICERD(approved)] } {
        set ::SLICERD(approved) [tk_messageBox \
                                     -type yesno \
                                     -title "Slicer Daemon" \
                                     -message "Connection Attemped from $addr.\n\nAllow external connections?"]
    }
    if { $::SLICERD(approved)  == "no" } {
        close $sock
        return
    }

    #
    # create a tcl helper for this socket
    # then set up a callback for when the socket becomes readable
    #
    set _tcl ::tcl_$sock
    catch "$_tcl Delete"
    vtkTclHelper $_tcl
    
    # special trick to let the tcl helper know what interp to use
    set tag [$_tcl AddObserver ModifiedEvent ""]
    $_tcl SetInterpFromCommand $tag

    fileevent $sock readable "slicerd_sock_fileevent $sock"
}

#
# handles input on the connection
#
proc slicerd_sock_fileevent {sock} {
    
    if { [eof $sock] } {
        close $sock
        set _tcl ::tcl_$sock
        catch "$_tcl Delete"
        return
    }
    
    gets $sock line
    
    switch -glob $line {
        "ls*" {
            puts "listing of scene"
            set n [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLVolumeNode]
            for {set i 0} {$i < $n} {incr i} {
                set node [$::slicer3::MRMLScene GetNthNodeByClass $i vtkMRMLVolumeNode]
                puts -nonewline $sock "$i \"[$node GetID] [$node GetName]\" "
            }
            puts $sock ""
            flush $sock
        }
        "get*" {
            if { [llength $line] < 2 } {
                puts $sock "get error: missing id"
                flush $sock
                return
            }
            
            set volid [lindex $line 1]
            if { [string is digit $volid] } {
                set node [$::slicer3::MRMLScene GetNthNodeByClass $volid vtkMRMLVolumeNode]
            } else {
                set node [$::slicer3::MRMLScene GetNodeByID $volid]
            }
            
            if { $node == "" } {
                # try to get volume by name
                set numNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLVolumeNode]
                for {set n 0} {$n < $numNodes} {incr n} {
                    set node [$::slicer3::MRMLScene GetNthNodeByClass $n vtkMRMLVolumeNode]
                    if { [$node GetName] == $volid } {
                        break;
                    }
                    set node ""
                }
            }
            
            if { $node == "" } {
                puts $sock "get error: bad id"
                flush $sock
                return
            } 
            
            set isTensor 0

            # find out if we are dealing with a tensor or a scalar volume
            if { [$node GetClassName] eq "vtkMRMLDiffusionTensorVolumeNode" } {
                set isTensor 1
            } else {
                # If we are not dealing with tensor, assuming scalar data
                set isTensor 0
            }
            
            # calculate the space directions and origin
            catch "::slicerd::export_matrix Delete"
            vtkMatrix4x4 ::slicerd::export_matrix
            $node GetRASToIJKMatrix ::slicerd::export_matrix
            ::slicerd::export_matrix Invert
            set space_origin [format "(%.15g, %.15g, %.15g)" \
                                  [::slicerd::export_matrix GetElement 0 3]\
                                  [::slicerd::export_matrix GetElement 1 3]\
                                  [::slicerd::export_matrix GetElement 2 3] ]
            set space_directions [format "(%.15g, %.15g, %.15g) (%.15g, %.15g, %.15g) (%.15g, %.15g, %.15g)" \
                                      [::slicerd::export_matrix GetElement 0 0]\
                                      [::slicerd::export_matrix GetElement 1 0]\
                                      [::slicerd::export_matrix GetElement 2 0]\
                                      [::slicerd::export_matrix GetElement 0 1]\
                                      [::slicerd::export_matrix GetElement 1 1]\
                                      [::slicerd::export_matrix GetElement 2 1]\
                                      [::slicerd::export_matrix GetElement 0 2]\
                                      [::slicerd::export_matrix GetElement 1 2]\
                                      [::slicerd::export_matrix GetElement 2 2] ]
            ::slicerd::export_matrix Delete
            
            # TODO: should add direction cosines and label_map status
            set im [$node GetImageData]
            
            if {$isTensor} {
                # get the measurement frame
                catch "::slicerd::measurement_frame_matrix Delete"
                
                vtkMatrix4x4 ::slicerd::measurement_frame_matrix
                $node GetMeasurementFrameMatrix ::slicerd::measurement_frame_matrix
             
                # measurement frame in the nrrd header is columnwise 
                set measurement_frame [format "(%.15g, %.15g, %.15g) (%.15g, %.15g, %.15g) (%.15g, %.15g, %.15g)" \
                                           [::slicerd::measurement_frame_matrix GetElement 0 0]\
                                           [::slicerd::measurement_frame_matrix GetElement 1 0]\
                                           [::slicerd::measurement_frame_matrix GetElement 2 0]\
                                           [::slicerd::measurement_frame_matrix GetElement 0 1]\
                                           [::slicerd::measurement_frame_matrix GetElement 1 1]\
                                           [::slicerd::measurement_frame_matrix GetElement 2 1]\
                                           [::slicerd::measurement_frame_matrix GetElement 0 2]\
                                           [::slicerd::measurement_frame_matrix GetElement 1 2]\
                                           [::slicerd::measurement_frame_matrix GetElement 2 2] ]
                
                set tensors [[$im GetPointData] GetTensors]
            }
            
            puts stderr "image $volid" 
            puts stderr "name [$node GetName]"
            puts stderr "scalar_type [$im GetScalarType]" 
            puts stderr "space_origin $space_origin"
            
            if {$isTensor} {
                puts stderr "dimensions [$tensors GetNumberOfComponents] [$im GetDimensions]"
                puts stderr "space_directions none $space_directions"
                puts stderr "kinds 3D-masked-symmetric-matrix space space space"
                puts stderr "measurement_frame $measurement_frame"
            } else {
                puts stderr "dimensions [$im GetDimensions]"
                puts stderr "space_directions $space_directions"
                puts stderr "kinds space space space"
            }
            
            fconfigure $sock -translation auto
            puts $sock "image $volid" 
            puts $sock "name [$node GetName]" 
            puts $sock "scalar_type [$im GetScalarType]" 
            
            if {$isTensor} {
                # for now a tensor that is exported always has 7 values
                puts $sock "dimensions 7 [$im GetDimensions]"
            } else {
                puts $sock "dimensions [$im GetDimensions]" 
            }

            puts $sock "space_origin $space_origin"

            if {$isTensor} {
                puts $sock "space_directions none $space_directions"
                puts $sock "kinds 3D-masked-symmetric-matrix space space space"
                puts $sock "measurement_frame $measurement_frame"
            } else {
                puts $sock "space_directions $space_directions"
                puts $sock "kinds space space space"
            }
            flush $sock
            
            ::tcl_$sock SetImageData $im
            ::tcl_$sock SetVolumeNode $node

            fconfigure $sock -translation binary
            
            if {$isTensor} {
                ::tcl_$sock SetMeasurementFrame ::slicerd::measurement_frame_matrix
                ::tcl_$sock SendImageDataTensors $sock
               
            } else {
                ::tcl_$sock SendImageDataScalars $sock
            }
            fconfigure $sock -translation auto
            flush $sock
            if {$isTensor} {
                ::slicerd::measurement_frame_matrix Delete
            }
        } 
        "put*" {
            #
            # read a volume from the socket (e.g. from slicerput)
            # - read the protocol tag ("image")
            # - create a unique name if needed
            # - parse the header data
            # - create the image data and mrml node
            # - make the new volume the default background image
            #
            
            gets $sock line
            if { ![string match "image *" $line] } {
                puts $sock "put error: bad protocol"
                flush $sock
                return
            }
            set name [lindex $line 1]
            if { $name == "" } {
                for {set n 0} {1} {incr n} {
                    set name "slicerd_$n"
                    if { [[$::slicer3::MRMLScene GetNodesByName $name] GetNumberOfItems] == 0 } {
                        break;
                    }
                }
            }
            gets $sock space
            set space [lindex $space 1]
            gets $sock dimensions
            set dimensions [lrange $dimensions 1 end]
            gets $sock space_origin
            set space_origin [lrange $space_origin 1 end]
            gets $sock space_directions
            set space_directions [lrange $space_directions 1 end]
            gets $sock kinds
            set kinds [lrange $kinds 1 end]
            gets $sock components
            set components [lindex $components 1]
            gets $sock scalar_type
            set scalar_type [lindex $scalar_type 1]

            set measurement_frame "none"

            # add a mrml node according to volume class
            set node ""
            if { [lindex $kinds 0] == "3D-masked-symmetric-matrix"} {
                gets $sock measurement_frame
                set  measurement_frame [lrange $measurement_frame 1 end]
                set dimensions [lrange $dimensions 1 end]
                set node [vtkMRMLDiffusionTensorVolumeNode New]
                puts "SlicerDaemon: Assume you want to put a tensor volume."
            } else {
                puts "SlicerDaemon: Assume you want to put a scalar volume."
                set node [vtkMRMLScalarVolumeNode New]
            }

            $node SetName $name
            $node SetDescription "Imported via slicerd"
            
            set idata ::imagedata-[info cmdcount]
            set tensordata ::tensordata-[info cmdcount]

            vtkImageData $idata
            eval $idata SetDimensions $dimensions
            $idata SetNumberOfScalarComponents $components
            $idata SetScalarType $scalar_type
            $idata AllocateScalars

            ::tcl_$sock SetImageData $idata
            #slicerd_parse_space_directions $node $space_origin $space_directions $space

            fconfigure $sock -translation binary -encoding binary

            if {  [lindex $kinds 0] == "3D-masked-symmetric-matrix" } {
                # space directions contains "none", cut it out
                slicerd_parse_space_directions $node $space_origin [lrange $space_directions 1 end] $space
                ::tcl_$sock SetVolumeNode $node
                slicerd_parse_space_measurement_frame_and_setMF ::tcl_$sock \
                    $measurement_frame $space
                ::tcl_$sock ReceiveImageDataTensors $sock
            } else { 
                slicerd_parse_space_directions $node $space_origin $space_directions $space        
                ::tcl_$sock ReceiveImageDataScalars $sock
            }
            fconfigure $sock -translation auto
            $node SetAndObserveImageData $idata
            $idata Delete

            $::slicer3::MRMLScene AddNode $node
            [$::slicer3::ApplicationLogic GetSelectionNode] SetReferenceActiveVolumeID [$node GetID]
            $::slicer3::ApplicationLogic PropagateVolumeSelection
            $node Delete
           
        }
        "eval*" {
            set oldMode [fconfigure $sock -buffering]
            fconfigure $sock -buffering none
            puts $sock [eval $line]
            flush $sock
            fconfigure $sock -buffering $oldMode
        }
        default {
            puts $sock "unknown command $line"
            flush $sock
        }
    }
}

#
# convert nrrd-style measurement frame line into a vtkMatrix4x4
#
proc slicerd_parse_space_measurement_frame_and_setMF {tcl_sock mf_line space} {

    regsub -all "\\(" $mf_line " " mf_line
    regsub -all "\\)" $mf_line " " mf_line
    regsub -all "\\," $mf_line " " mf_line
    
    catch "measurement_frame_matrix Delete"
    vtkMatrix4x4 measurement_frame_matrix
    measurement_frame_matrix Identity
    
    set elements [split $mf_line " "]
    set elements_cleaned ""
    
    foreach e $elements {
        if {$e != ""} {
            lappend elements_cleaned $e
        }
    }
    measurement_frame_matrix SetElement 0 0 [lindex $elements_cleaned 0]
    measurement_frame_matrix SetElement 1 0 [lindex $elements_cleaned 1]
    measurement_frame_matrix SetElement 2 0 [lindex $elements_cleaned 2]
    measurement_frame_matrix SetElement 0 1 [lindex $elements_cleaned 3]
    measurement_frame_matrix SetElement 1 1 [lindex $elements_cleaned 4]
    measurement_frame_matrix SetElement 2 1 [lindex $elements_cleaned 5]
    measurement_frame_matrix SetElement 0 2 [lindex $elements_cleaned 6]
    measurement_frame_matrix SetElement 1 2 [lindex $elements_cleaned 7]
    measurement_frame_matrix SetElement 2 2 [lindex $elements_cleaned 8]
   

    if {$space eq "left-posterior-superior"} {
        # in slicer everything is in RAS, so we need to transform
        # the measurement frame (measurement_frame_matrix).
        # This can be done by multiplication with matrix LPSToRAS.
        # Slicerput.tcl has to deal with the case where the space is neither
        # RAS nor LPS. 
        
        catch "::slicerd::LPSToRAS Delete"
        vtkMatrix4x4 ::slicerd::LPSToRAS
        ::slicerd::LPSToRAS Identity
        ::slicerd::LPSToRAS SetElement 0 0 -1 
        ::slicerd::LPSToRAS SetElement 1 1 -1
        ::slicerd::LPSToRAS Multiply4x4 ::slicerd::LPSToRAS measurement_frame_matrix measurement_frame_matrix
        puts "Found Tensor in LPS space. Converting measurment frame into from LPS into RAS space.\n"
        puts "This is the new mesurement frame:\n"
        puts [measurement_frame_matrix Print]
    }
    
    $tcl_sock SetMeasurementFrame measurement_frame_matrix

    [$tcl_sock GetVolumeNode] SetMeasurementFrameMatrix measurement_frame_matrix
}

#
# convert nrrd-style space directions line into vtk/slicer info
# - unfortunately, this is some nasty math to do in tcl
#
proc slicerd_parse_space_directions {node space_origin space_directions space} {
    #
    # parse the 'space directions' and 'space origin' information into
    # a slicer RasToIjk and related matrices by telling the mrml node
    # the RAS corners of the volume
    #
    # if "space" is right-anterior-superior: nothing else to do.
    # else if "space" is left-posterior-superior: adapt space directions and 
    # origin.
    # else: other space sirections are not supported!

    if {$space_origin eq "none"} {
        set space_origin "0 0 0"
    }  

    regsub -all "\\(" $space_origin " " space_origin
    regsub -all "\\)" $space_origin " " space_origin
    regsub -all "\\," $space_origin " " space_origin
    regsub -all "\\(" $space_directions " " space_directions
    regsub -all "\\)" $space_directions " " space_directions
    regsub -all "\\," $space_directions " " space_directions

    #
    # normalize and save length for each space direction vector
    #
    set spacei 0
    foreach dir [lrange $space_directions 0 2] {
        set spacei [expr $spacei + $dir * $dir]
    }
    set spacei [expr sqrt($spacei)]
    set unit_space_directions ""
    foreach dir [lrange $space_directions 0 2] {
        lappend unit_space_directions [expr $dir / $spacei]
    }

    set spacej 0
    foreach dir [lrange $space_directions 3 5] {
        set spacej [expr $spacej + $dir * $dir]
    }
    set spacej [expr sqrt($spacej)]
    foreach dir [lrange $space_directions 3 5] {
        lappend unit_space_directions [expr $dir / $spacej]
    }

    set spacek 0
    foreach dir [lrange $space_directions 6 8] {
        set spacek [expr $spacek + $dir * $dir]
    }
    set spacek [expr sqrt($spacek)]
    foreach dir [lrange $space_directions 6 8] {
        lappend unit_space_directions [expr $dir / $spacek]
    }
    
    # In slicer3, all image data has 1 1 1 spacing -- only the RASToIJK matrix matters
    # [$node GetImageData] SetSpacing $spacei $spacej $spacek

    #
    # fill the ijk to ras matrix
    # - use it to calculate the slicer internal matrices (RASToIJK etc)
    #3
    catch "::slicerd::Ijk_matrix Delete"
    vtkMatrix4x4 ::slicerd::Ijk_matrix
    ::slicerd::Ijk_matrix Identity
    for {set i 0} {$i < 3} {incr i} {
        for {set j 0} {$j < 3} {incr j} {
            set val [lindex $space_directions [expr 3 * $i + $j]]
            ::slicerd::Ijk_matrix SetElement $j $i $val
        }
        set val [lindex $space_origin $i]
        ::slicerd::Ijk_matrix SetElement $i 3 $val
    }    
    #puts "::slicerd::Ijk_matrix before:"
    #puts [::slicerd::Ijk_matrix Print]
    
    if {$space eq "left-posterior-superior"} {
        # in slicer everything is in RAS, so we need to transform
        # space directions and origin (::slicerd::Ijk_matrix).
        # This can be done by multiplication with matrix LPSToRAS.
        # Slicerput.tcl has to deal with the case where the space is neither
        # RAS nor LPS.
        
        catch "::slicerd::LPSToRAS Delete"
        vtkMatrix4x4 ::slicerd::LPSToRAS
        ::slicerd::LPSToRAS Identity
        ::slicerd::LPSToRAS SetElement 0 0 -1 
        ::slicerd::LPSToRAS SetElement 1 1 -1
        ::slicerd::LPSToRAS Multiply4x4 ::slicerd::LPSToRAS ::slicerd::Ijk_matrix ::slicerd::Ijk_matrix
    }     
    ::slicerd::Ijk_matrix Invert
    
    $node SetRASToIJKMatrix ::slicerd::Ijk_matrix
    #puts "Here the RASToIJK matrix:\n"
    #puts [::slicerd::Ijk_matrix Print]

    ::slicerd::Ijk_matrix Delete
}
