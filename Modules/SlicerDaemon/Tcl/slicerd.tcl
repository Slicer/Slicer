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

#=auto==========================================================================
# Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
# 
#  Program:   3D Slicer
#  Module:    $RCSfile: slicerd.tcl,v $
#  Date:      $Date: 2006/01/30 20:47:46 $
#  Version:   $Revision: 1.10 $
#===============================================================================

#
# experimental slicer daemon - sp 2005-09-23 
# - meant to be as simple as possible
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

# this package contains the vtkTclHelper
package require vtkQueryAtlas

#
# returns a listening socket on given port or uses default
# - local only for security, use ssh tunnels for remote access
#
proc slicerd_start { {port 18943} } {

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
            foreach volid $::Volume(idList) {
                puts -nonewline $sock "$volid \"[Volume($volid,node) GetName]\" "
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
            if { [info command Volume($volid,vol)] == "" } {
                # if the id isn't a number, try looking by name as a pattern
                set ids [MainVolumesGetVolumesByNamePattern $volid]
                if { [llength $ids] != 1 } {
                    puts $sock "get error: bad id"
                    flush $sock
                    return
                } 
                set volid $ids ;# there's only one and it's our target
            }

            # calculate the space directions and origin
            catch "export_matrix Delete"
            vtkMatrix4x4 export_matrix
            eval export_matrix DeepCopy [Volume($volid,node) GetRasToIjkMatrix]
            export_matrix Invert
            set space_origin [format "(%g, %g, %g)" \
                [export_matrix GetElement 0 3]\
                [export_matrix GetElement 1 3]\
                [export_matrix GetElement 2 3] ]
            set space_directions [format "(%g, %g, %g) (%g, %g, %g) (%g, %g, %g)" \
                [export_matrix GetElement 0 0]\
                [export_matrix GetElement 1 0]\
                [export_matrix GetElement 2 0]\
                [export_matrix GetElement 0 1]\
                [export_matrix GetElement 1 1]\
                [export_matrix GetElement 2 1]\
                [export_matrix GetElement 0 2]\
                [export_matrix GetElement 1 2]\
                [export_matrix GetElement 2 2] ]
            export_matrix Delete

            # TODO: should add direction cosines and label_map status
            set im [Volume($volid,vol) GetOutput]

            puts stderr "image $volid" 
            puts stderr "scalar_type [$im GetScalarType]" 
            puts stderr "dimensions [$im GetDimensions]" 
            puts stderr "space_origin $space_origin"
            puts stderr "space_directions $space_directions"

            fconfigure $sock -translation auto
            puts $sock "image $volid" 
            puts $sock "scalar_type [$im GetScalarType]" 
            puts $sock "dimensions [$im GetDimensions]" 
            puts $sock "space_origin $space_origin"
            puts $sock "space_directions $space_directions"
            flush $sock

            catch "export_flip Delete"
            vtkImageFlip export_flip
            export_flip SetInput $im
            export_flip SetFilteredAxis 1
            [export_flip GetOutput] Update

            ::tcl_$sock SetImageData [export_flip GetOutput]
            fconfigure $sock -translation binary
            ::tcl_$sock SendImageDataScalars $sock
            fconfigure $sock -translation auto
            flush $sock
            catch "export_flip Delete"
        }
        "put" {
            gets $sock line
            if { ![string match "image *" $line] } {
                puts $sock "put error: bad protocol"
                flush $sock
                return
            }
            set name [lindex $line 1]
            if { $name == "" } {
                set name "slicerd"
            }

            gets $sock dimensions
            set dimensions [lrange $dimensions 1 3]
            gets $sock space_origin
            set space_origin [lrange $space_origin 1 end]
            gets $sock space_directions
            set space_directions [lrange $space_directions 1 end]
            gets $sock components
            set components [lindex $components 1]
            gets $sock scalar_type
            set scalar_type [lindex $scalar_type 1]


            # add a mrml node
            set n [MainMrmlAddNode Volume]
            set i [$n GetID]
            MainVolumesCreate $i
    
            $n SetName $name
            $n SetDescription "Imported via slicerd"
    
            set idata ::imagedata-[info cmdcount]
            vtkImageData $idata
            eval $idata SetDimensions $dimensions
            $idata SetNumberOfScalarComponents $components
            $idata SetScalarType $scalar_type
            $idata AllocateScalars

            ::tcl_$sock SetImageData $idata
            fconfigure $sock -translation binary -encoding binary
            ::tcl_$sock ReceiveImageDataScalars $sock
            fconfigure $sock -translation auto

            ::Volume($i,node) SetNumScalars $components
            ::Volume($i,node) SetScalarType $scalar_type

    
            ::Volume($i,node) SetDimensions [lindex $dimensions 0] [lindex $dimensions 1]
            ::Volume($i,node) SetImageRange 1 [lindex $dimensions 2]
            catch "flip Delete"
            vtkImageFlip flip
            flip SetFilteredAxis 1
            flip SetInput $idata
            flip Update
            Volume($i,vol) SetImageData [flip GetOutput]
            Volume($i,vol) Update
            slicerd_parse_space_directions $i $space_origin $space_directions
            $idata Delete
            flip Delete
            MainUpdateMRML

            Slicer SetOffset 0 0
            MainSlicesSetVolumeAll Back $i
            RenderAll
        }
        "eval*" {
            puts $sock [eval $line]
            flush $sock
        }
        default {
            puts $sock "unknown command $line"
            flush $sock
        }
    }
}

#
# convert nrrd-style space directions line into vtk/slicer info
# - unfortunately, this is some nasty math to do in tcl
#
proc slicerd_parse_space_directions {volid space_origin space_directions} {

    #
    # parse the 'space directions' and 'space origin' information into
    # a slicer RasToIjk and related matrices by telling the mrml node
    # the RAS corners of the volume
    #

    regsub -all "\\(" $space_origin " " space_origin
    regsub -all "\\)" $space_origin " " space_origin
    regsub -all "\\," $space_origin " " space_origin
    regsub -all "\\(" $space_directions " " space_directions
    regsub -all "\\)" $space_directions " " space_directions
    regsub -all "\\," $space_directions " " space_directions


puts "space_origin $space_origin"
puts "space_directions $space_directions"

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
    
    ::Volume($volid,node) SetSpacing $spacei $spacej $spacek
    [::Volume($volid,vol) GetOutput] SetSpacing $spacei $spacej $spacek


    #
    # fill the ijk to ras matrix
    # - use it to calculate the slicer internal matrices (RasToIjk etc)
    #
    catch "Ijk_matrix Delete"
    vtkMatrix4x4 Ijk_matrix
    Ijk_matrix Identity
    for {set i 0} {$i < 3} {incr i} {
        for {set j 0} {$j < 3} {incr j} {
            set val [lindex $space_directions [expr 3 * $i + $j]]
            Ijk_matrix SetElement $j $i $val
        }
        set val [lindex $space_origin $i]
        Ijk_matrix SetElement $i 3 $val
    }

    set dims [[::Volume($volid,vol) GetOutput] GetDimensions]

    VolumesComputeNodeMatricesFromIjkToRasMatrix2 $volid Ijk_matrix $dims

puts [Ijk_matrix Print]

    Ijk_matrix Delete
}

