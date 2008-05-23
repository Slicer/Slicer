
#
# win32-icons.tcl 
#
# - helper script to modify windows inon resources in
#   a starpack launcher for slicer3
#
# - uses slightly modified version of tklib0.4's ico package
#



package require Tcl 8.4


# Instantiate vars we need for this package
namespace eval ::ico {
    namespace export icons iconMembers getIcon getIconByName writeIcon copyIcon transparentColor clearCache EXEtoICO
    # stores cached indices of icons found
    variable  RES
    array set RES {}

    # used for 4bpp number conversion
    variable BITS
    array set BITS [list {} 0 0000 0 0001 1 0010 2 0011 3 0100 4 \
      0101 5 0110 6 0111 7 1000 8 1001 9 \
      1010 10 1011 11 1100 12 1101 13 1110 14 1111 15 \
      \
      00000 00 00001 0F 00010 17 00011 1F \
      00100 27 00101 2F 00110 37 00111 3F \
      01000 47 01001 4F 01010 57 01011 5F \
      01100 67 01101 6F 01110 77 01111 7F \
      10000 87 10001 8F 10010 97 10011 9F \
      10100 A7 10101 AF 10110 B7 10111 BF \
      11000 C7 11001 CF 11010 D7 11011 DF \
      11100 E7 11101 EF 11110 F7 11111 FF]
}




package require Tk
package require Img

proc modifyLauncherIcons { logoDir exe {ico ""} } {
  set iconSpecs {
    {1 32 4 16}
    {2 16 4 16}
    {3 32 8 256}
    {4 48 8 256}
    {5 48 4 16}
    {6 16 8 256}
  }

  foreach spec $iconSpecs {


    foreach {index dim bpp numColors} $spec {}

    set im [image create photo]
    $im read $logoDir/3DSlicerLogo-DesktopIcon-${dim}x${dim}x${numColors}.png
    puts "::ico::writeIcon $exe TK $bpp $im"
    ::ico::writeIconIndex $exe TK $bpp $im $index
  }

  if { $ico != "" } {
    set im [image create photo]
    $im read $logoDir/3DSlicerLogo-DesktopIcon-32x32x256.png
    ::ico::writeIcon $ico 0 $bpp $im -type ICO
  }
}


proc ::ico::writeIconIndex {file name bpp data index} {
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands writeIcon${type}Index]]} {
#        return -code error "unsupported file format $type"
    }
    if {[llength $data] == 1} {
        set data [getColorListFromImage $data]
    } elseif {[lsearch -glob [join $data] #*] > -1} {
        set data [translateColors $data]
    }
    if {$bpp != 1 && $bpp != 4 && $bpp != 8 && $bpp != 24 && $bpp != 32} {
        return -code error "invalid color depth"
    }
    set palette {}
    if {$bpp <= 8} {
        set palette [getPaletteFromColors $data]
        if {[lindex $palette 0] > (1 << $bpp)} {
            return -code error "specified color depth too low"
        }
        set data  [lindex $palette 2]
        set palette [lindex $palette 1]
        append palette [string repeat \000 [expr {(1 << ($bpp + 2)) - [string length $palette]}]]
    }
    set and [getAndMaskFromColors $data]
    set xor [getXORFromColors $bpp $data]
    # writeIcon$type file index w h bpp palette xor and
    writeIcon${type}Index [file normalize $file] $name \
        [llength [lindex $data 0]] [llength $data] $bpp $palette $xor $and $index
}

proc ico::writeIconEXEIndex {file name w h bpp palette xor and index} {
    variable RES

    set file [file normalize $file]
    set members [getIconMembersEXE $file $name]

    if {![info exists RES($file,icon,$index,data)]} {
        return -code error "no icon \"$name\""
    }
    if {![string match "* $w $h $bpp" $RES($file,icon,$index,data)]} {
        #puts " * $w $h $bpp != $RES($file,icon,$index,data) "
        #return -code error "icon format differs from original"
    }
    
    set fh [open $file r+]
    fconfigure $fh -eofchar {} -encoding binary -translation lf
    seek $fh [expr {$RES($file,icon,$index,offset) + 40}] start

    puts -nonewline $fh $palette$xor$and
    close $fh
}

proc removeTransparency {img} {
    set w [image width $img]
    set h [image height $img]
    for {set y [expr $h - 1]} {$y > -1} {incr y -1} {
      for {set x 0} {$x < $w} {incr x} {
        $img transparency set $x $y 0
      }
    }
    $img transparency set 0 0 1
}



# ico.tcl --
#
# Win32 ico manipulation code
#
# Copyright (c) 2003-2007 Aaron Faupell
# Copyright (c) 2003-2004 ActiveState Corporation
#
# RCS: @(#) $Id: ico.tcl,v 1.28 2008/03/12 07:25:49 hobbs Exp $

# Sample usage:
#        set file bin/wish.exe
#        set icos [::ico::icons $file]
#        set img  [::ico::getIcon $file [lindex $icos 1] -format image -res 32]


# icons --
#
# List of icons in a file
#
# ARGS:
#        file        File to extract icon info from.
#        ?-type?        Type of file.  If not specified, it is derived from
#                the file extension.  Currently recognized types are
#                EXE, DLL, ICO, ICL, BMP, and ICODATA
#
# RETURNS:
#        list of icon names or numerical IDs
#
proc ::ico::icons {file args} {
    parseOpts type $args
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands getIconList$type]]} {
        return -code error "unsupported file format $type"
    }
    getIconList$type [file normalize $file]
}

# iconMembers --
#
# Get info on images which make up an icon
#
# ARGS:
#        file                File containing icon
#       name                Name of the icon in the file
#        ?-type?                Type of file.  If not specified, it is derived from
#                        the file extension.  Currently recognized types are
#                        EXE, DLL, ICO, ICL, BMP, and ICODATA
#
# RETURNS:
#        list of icons as tuples {name width height bpp}
#
proc ::ico::iconMembers {file name args} {
    parseOpts type $args
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands getIconMembers$type]]} {
        return -code error "unsupported file format $type"
    }
    getIconMembers$type [file normalize $file] $name
}

# getIcon --
#
# Get pixel data or image of icon
#
# ARGS:
#        file                File to extract icon info from.
#        name                Name of image in the file to use.  The name is the first element
#                        in the sublists returned by iconMembers.
#        ?-res?                Set the preferred resolution.
#        ?-bpp?                Set the preferred color depth in bits per pixel.
#        ?-exact?        Accept only exact matches for res and bpp. Returns
#                        an error if there is no exact match.
#        ?-type?                Type of file.  If not specified, it is derived from
#                        the file extension.  Currently recognized types are
#                        EXE, DLL, ICO, ICL, BMP, and ICODATA
#        ?-format?        Output format. Must be one of "image" or "colors"
#                        'image' will return the name of a Tk image.
#                        'colors' will return a list of pixel values
#        ?-image?        If output is image, use this as the name of Tk image
#                        created
#
# RETURNS:
#        pixel data as a list that could be passed to 'image create'
#        or the name of a Tk image
#
proc ::ico::getIcon {file name args} {
    set image {}
    set format image
    set exact 0
    set bpp 24
    parseOpts {type format image res bpp exact} $args
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands getRawIconData$type]]} {
        return -code error "unsupported file format $type"
    }
    # ICODATA is a pure data type - not a real file
    if {$type ne "ICODATA"} {
        set file [file normalize $file]
    }

    set mem [getIconMembers$type $file $name]

    if {![info exists res]} {
        set icon [lindex $mem 0 0]
    } elseif {$exact} {
        set icon [lsearch -inline -glob $mem "* $res $bpp"]
        if {$icon == ""} { return -code error "No matching icon" }
    } else {
        set mem [lsort -integer -index 1 $mem]
        set match ""
        foreach x $mem {
            if {[lindex $x 1] == [lindex $res 0]} { lappend match $x }
        }
        if {$match == ""} {
            # todo: resize a larger icon
            #return -code error "No matching icon"
            set match [list [lindex $mem end]]
        }
        set match [lsort -integer -decreasing -index 3 $match]
        foreach x $match {
            if {[lindex $x 3] <= $bpp} { set icon [lindex $x 0]; break }
        }
        if {![info exists icon]} { set icon [lindex $match end 0]}
    }
    if {$format eq "name"} {
        return $icon
    }
    set colors [eval [linsert [getRawIconData$type $file $icon] 0 getIconAsColorList]]
    if {$format eq "image"} {
        return [createImage $colors $image]
    }
    return $colors
}

# getIconByName --
#
# Get pixel data or image of icon name in file. The icon name
# is the first element of the sublist from [iconMembers].
#
# ARGS:
#        file                       File to extract icon info from.
#        name                Name of image in the file to use.  The name is the first element
#                        in the sublists returned by iconMembers.
#        ?-type?                Type of file.  If not specified, it is derived from
#                        the file extension.  Currently recognized types are
#                               EXE, DLL, ICO, ICL, BMP, and ICODATA
#        ?-format?        Output format. Must be one of "image" or "colors"
#                        'image' will return the name of a Tk image.
#                        'colors' will return a list of pixel values
#        ?-image?        If output is image, use this as the name of Tk image
#                        created
#
# RETURNS:
#        pixel data as a list that could be passed to 'image create'
#
proc ::ico::getIconByName {file name args} {
    set format image
    set image {}
    parseOpts {type format image} $args
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands getRawIconData$type]]} {
        return -code error "unsupported file format $type"
    }
    # ICODATA is a pure data type - not a real file
    if {$type ne "ICODATA"} {
        set file [file normalize $file]
    }
    set colors [eval [linsert [getRawIconData$type $file $name] 0 getIconAsColorList]]
    if {$format eq "image"} {
        return [createImage $colors $image]
    }
    return $colors
}

# getFileIcon --
#
# Get the registered icon for the file under Windows
#
# ARGS:
#        file        File to get icon for.
#        
#        optional arguments and return values are the same as getIcon
#
proc ::ico::getFileIcon {file args} {
    set icon "%SystemRoot%\\System32\\shell32.dll,0"
    if {[file isdirectory $file] || $file == "Folder"} {
        if {![catch {registry get HKEY_CLASSES_ROOT\\Folder\\DefaultIcon ""} reg]} {
            set icon $reg
        }
    } else {
        set ext [file extension $file]
        if {![catch {registry get HKEY_CLASSES_ROOT\\$ext ""} doctype]} {
            if {![catch {registry get HKEY_CLASSES_ROOT\\$doctype\\CLSID ""} clsid] && \
                ![catch {registry get HKEY_CLASSES_ROOT\\CLSID\\$clsid\\DefaultIcon ""} reg]} {
                set icon $reg
            } elseif {![catch {registry get HKEY_CLASSES_ROOT\\$doctype\\DefaultIcon ""} reg]} {
                set icon $reg
            }
        }
    }
    set index [lindex [split $icon ,] 1]
    set icon [lindex [split $icon ,] 0]
    if {$index == ""} { set index 0 }
    set icon [string trim $icon "@'\" "]
    while {[regexp -nocase {%([a-z]+)%} $icon -> var]} {
        set icon [string map [list %$var% $::env($var)] $icon]
    }
    set icon [string map [list %1 $file] $icon]
    if {$index < 0} {
        if {![catch {eval [list getIcon $icon [string trimleft $index -]] $args} output]} {
            return $output
        }
        set index 0
    }
    return [eval [list getIcon $icon [lindex [icons $icon] $index]] $args]
}

# writeIcon --
#
# Overwrite write image in file with depth/pixel data
#
# ARGS:
#        file        File to extract icon info from.
#        name        Name of image in the file to use.  The name is the first element
#                in the sublists returned by iconMembers.
#        bpp        bit depth of icon we are writing
#        data        Either pixel color data (as returned by getIcon -format color)
#                or the name of a Tk image.
#        ?-type?        Type of file.  If not specified, it is derived from
#                the file extension.  Currently recognized types are
#                EXE, DLL, ICO and ICL
#
# RETURNS:
#        nothing
#
proc ::ico::writeIcon {file name bpp data args} {
    parseOpts type $args
    if {![info exists type]} {
        # $type wasn't specified - get it from the extension
        set type [fileext $file]
    }
    if {![llength [info commands writeIcon$type]]} {
        return -code error "unsupported file format $type"
    }
    if {[llength $data] == 1} {
        set data [getColorListFromImage $data]
    } elseif {[lsearch -glob [join $data] #*] > -1} {
        set data [translateColors $data]
    }
    if {$bpp != 1 && $bpp != 4 && $bpp != 8 && $bpp != 24 && $bpp != 32} {
        return -code error "invalid color depth"
    }
    set palette {}
    if {$bpp <= 8} {
        set palette [getPaletteFromColors $data]
        if {[lindex $palette 0] > (1 << $bpp)} {
            return -code error "specified color depth too low"
        }
        set data  [lindex $palette 2]
        set palette [lindex $palette 1]
        append palette [string repeat \000 [expr {(1 << ($bpp + 2)) - [string length $palette]}]]
    }
    set and [getAndMaskFromColors $data]
    set xor [getXORFromColors $bpp $data]
    # writeIcon$type file index w h bpp palette xor and
    writeIcon$type [file normalize $file] $name \
        [llength [lindex $data 0]] [llength $data] $bpp $palette $xor $and
}


# copyIcon --
#
# Copies an icon directly from one file to another
#
# ARGS:
#        file1                File to extract icon info from.
#        name1                Name of image in the file to use.  The name is the first element
#                        in the sublists returned by iconMembers.
#        file2                File to write icon to.
#        name2                Name of image in the file to use.  The name is the first element
#                        in the sublists returned by iconMembers.
#        ?-fromtype?        Type of source file.  If not specified, it is derived from
#                        the file extension.  Currently recognized types are
#                        EXE, DLL, ICO, ICL, BMP, and ICODATA
#        ?-totype?        Type of destination file.  If not specified, it is derived from
#                        the file extension.  Currently recognized types are
#                        EXE, DLL, ICO, ICL, BMP, and ICODATA
#
# RETURNS:
#        nothing
#
proc ::ico::copyIcon {file1 name1 file2 name2 args} {
    parseOpts {fromtype totype} $args
    if {![info exists fromtype]} {
        # $type wasn't specified - get it from the extension
        set fromtype [fileext $file1]
    }
    if {![info exists totype]} {
        # $type wasn't specified - get it from the extension
        set totype [fileext $file2]
    }
    if {![llength [info commands writeIcon$totype]]} {
        return -code error "unsupported file format $totype"
    }
    if {![llength [info commands getRawIconData$fromtype]]} {
        return -code error "unsupported file format $fromtype"
    }
    set src [getRawIconData$fromtype $file1 $name1]
    writeIcon $file2 $name2 [lindex $src 2] [eval getIconAsColorList $src] -type $totype
}

#
# transparentColor --
#
# Turns on transparency for all pixels in the image that match the color
#
# ARGS:
#        img                Name of the Tk image to modify, or an image in color list format
#        color                Color in #hex format which will be made transparent
#
# RETURNS:
#        the data or image after modification
#
proc ::ico::transparentColor {img color} {
    if {[llength $img] == 1} {
        package require Tk
        if {[string match "#*" $color]} {
            set color [scan $color "#%2x%2x%2x"]
        }
        set w [image width $img]
        set h [image height $img]
        for {set y 0} {$y < $h} {incr y} {
            for {set x 0} {$x < $w} {incr x} {
                if {[$img get $x $y] eq $color} {$img transparency set $x $y 1}
            }
        }
    } else {
        set y 0
        foreach row $img {
            set x 0
            foreach px $row {
                if {$px == $color} {lset img $y $x {}}
                incr x
            }
            incr y
        }
    }
    return $img
}

#
# clearCache --
#
# Clears the cache of icon offsets
#
# ARGS:
#        file        optional filename
#
#
# RETURNS:
#        nothing
#
proc ::ico::clearCache {{file {}}} {
    variable RES
    if {$file ne ""} {
        array unset RES $file,*
    } else {
        unset RES
        array set RES {}
    }
}

#
# EXEtoICO --
#
# Convert all icons found in exefile into regular icon files
#
# ARGS:
#        exeFile                Input EXE filename
#        ?icoDir?        Output ICO directory. Default is the
#                        same directory exeFile is located in
#
# RETURNS:
#        nothing
#
proc ::ico::EXEtoICO {exeFile {icoDir {}}} {
    variable RES

    set file [file normalize $exeFile]
    FindResources $file

    if {$icoDir == ""} { set icoDir [file dirname $file] }

    set fh [open $file]
    fconfigure $fh -eofchar {} -encoding binary -translation lf

    foreach group $RES($file,group,names) {
        set dir  {}
        set data {}
        foreach icon $RES($file,group,$group,members) {
            seek $fh $RES($file,icon,$icon,offset) start
            set ico $RES($file,icon,$icon,data)
            eval [list lappend dir] $ico
            append data [read $fh [eval calcSize $ico 40]]
        }

        # write them out to a file
        set ifh [open [file join $icoDir [file tail $exeFile]-$group.ico] w+]
        fconfigure $ifh -eofchar {} -encoding binary -translation lf

        bputs $ifh sss 0 1 [llength $RES($file,group,$group,members)]
        set offset [expr {6 + ([llength $RES($file,group,$group,members)] * 16)}]
        foreach {w h bpp} $dir {
            set len [calcSize $w $h $bpp 40]
            lappend fix $offset $len
            bputs $ifh ccccssii $w $h [expr {$bpp <= 8 ? 1 << $bpp : 0}] 0 1 $bpp $len $offset
            set offset [expr {$offset + $len}]
        }
        puts -nonewline $ifh $data
        foreach {offset size} $fix {
            seek $ifh [expr {$offset + 20}] start
            bputs $ifh i $size
        }
        close $ifh
    }
    close $fh
}



##
## Internal helper commands.
## Some may be appropriate for exposing later, but would need docs
## and make sure they "fit" in the API.
##

# gets the file extension as we use it internally (upper case, no '.')
proc ::ico::fileext {file} {
    return [string trimleft [string toupper [file extension $file]] .]
}

# helper proc to parse optional arguments to some of the public procs
proc ::ico::parseOpts {acc opts} {
    foreach {key val} $opts {
        set key [string trimleft $key -]
        if {[lsearch -exact $acc $key] >= 0} {
            upvar $key $key
            set $key $val
        } elseif {$key ne ""} {
            return -code error "unknown option \"$key\": must be one of $acc"
        }
    }
}

# formats a single color from a binary decimal list format to the #hex format
proc ::ico::formatColor {r g b} {
    format "#%02X%02X%02X" [scan $r %c] [scan $g %c] [scan $b %c]
}

# translates a color list from the #hex format to the decimal list format
#                                #0000FF                  {0 0 255}
proc ::ico::translateColors {colors} {
    set new {}
    foreach line $colors {
        set tline {}
        foreach x $line {
            if {$x eq ""} {lappend tline {}; continue}
            lappend tline [scan $x "#%2x%2x%2x"]
        }
        set new [linsert $new 0 $tline]
    }
    return $new
}

# reads a 32 bit signed integer from the filehandle
proc ::ico::getdword {fh} {
    binary scan [read $fh 4] i* tmp
    return $tmp
}

proc ::ico::getword {fh} {
    binary scan [read $fh 2] s* tmp
    return $tmp
}

proc ::ico::getulong {fh} {
    binary scan [read $fh 4] i tmp
    return [format %u $tmp]
}

proc ::ico::getushort {fh} {
    binary scan [read $fh 2] s tmp
    return [expr {$tmp & 0x0000FFFF}]
}

proc ::ico::bputs {fh format args} {
    puts -nonewline $fh [eval [list binary format $format] $args]
}

proc ::ico::createImage {colors {name {}}} {
    package require Tk
    set h [llength $colors]
    set w [llength [lindex $colors 0]]
    if {$name ne ""} {
        set img [image create photo $name -width $w -height $h]
    } else {
        set img [image create photo -width $w -height $h]
    }
    if {0} {
        # if image supported "" colors as transparent pixels,
        # we could use this much faster op
        $img put -to 0 0 $colors
    } else {
        for {set x 0} {$x < $w} {incr x} {
            for {set y 0} {$y < $h} {incr y} {
                set clr [lindex $colors $y $x]
                if {$clr ne ""} {
                    $img put -to $x $y $clr
                }
            }
        }
    }
    return $img
}

# return a list of colors in the #hex format from raw icon data
# returned by readDIB
proc ::ico::getIconAsColorList {w h bpp palette xor and} {
    # Create initial empty color array that we'll set indices in
    set colors {}
    set row    {}
    set empty  {}
    for {set x 0} {$x < $w} {incr x} { lappend row $empty }
    for {set y 0} {$y < $h} {incr y} { lappend colors $row }

    set x 0
    set y [expr {$h-1}]
    if {$bpp == 1} {
        binary scan $xor B* xorBits
        foreach i [split $xorBits {}] a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                lset colors $y $x [lindex $palette $i]
            }
            incr x
        }
    } elseif {$bpp == 4} {
        variable BITS
        binary scan $xor B* xorBits
        set i 0
        foreach a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                set bits [string range $xorBits $i [expr {$i+3}]]
                lset colors $y $x [lindex $palette $BITS($bits)]
            }
            incr i 4
            incr x
        }
    } elseif {$bpp == 8} {
        foreach i [split $xor {}] a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                lset colors $y $x [lindex $palette [scan $i %c]]
            }
            incr x
        }
    } elseif {$bpp == 16} {
        variable BITS
        binary scan $xor b* xorBits
        set i 0
        foreach a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                set b1 [string range $xorBits      $i        [expr {$i+4}]]
                set b2 [string range $xorBits [expr {$i+5}]  [expr {$i+9}]]
                set b3 [string range $xorBits [expr {$i+10}] [expr {$i+14}]]
                lset colors $y $x "#$BITS($b3)$BITS($b2)$BITS($b1)"
            }
            incr i 16
            incr x
        }
    } elseif {$bpp == 24} {
        foreach {b g r} [split $xor {}] a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                lset colors $y $x [formatColor $r $g $b]
            }
            incr x
        }
    } elseif {$bpp == 32} {
        foreach {b g r n} [split $xor {}] a [split $and {}] {
            if {$x == $w} { set x 0; incr y -1 }
            if {$a == 0} {
                lset colors $y $x [formatColor $r $g $b]
            }
            incr x
        }
    }
    return $colors
}

# creates a binary formatted AND mask by reading a list of colors in the decimal list format
# and checking for empty colors which designate transparency
proc ::ico::getAndMaskFromColors {colors} {
    set and {}
    foreach line $colors {
        set l {}
        foreach x $line {append l [expr {$x eq ""}]}
        append l [string repeat 0 [expr {[string length $l] % 32}]]
        foreach {a b c d e f g h} [split $l {}] {
            append and [binary format B8 $a$b$c$d$e$f$g$h]
        }
    }
    return $and
}

# creates a binary formatted XOR mask in the specified depth format from
# a list of colors in the decimal list format
proc ::ico::getXORFromColors {bpp colors} {
    set xor {}
    if {$bpp == 1} {
        foreach line $colors {
            foreach {a b c d e f g h} $line {
                foreach x {a b c d e f g h} {
                    if {[set $x] == ""} {set $x 0}
                }
                binary scan $a$b$c$d$e$f$g$h bbbbbbbb h g f e d c b a
                append xor [binary format b8 $a$b$c$d$e$f$g$h]
            }
        }
    } elseif {$bpp == 4} {
        foreach line $colors {
            foreach {a b} $line {
                if {$a == ""} {set a 0}
                if {$b == ""} {set b 0}
                binary scan $a$b b4b4 b a
                append xor [binary format b8 $a$b]
            }
        }
    } elseif {$bpp == 8} {
        foreach line $colors {
            foreach x $line {
                if {$x == ""} {set x 0}
                append xor [binary format c $x]
            }
        }
    } elseif {$bpp == 24} {
        foreach line $colors {
            foreach x $line {
                if {![llength $x]} {
                    append xor [binary format ccc 0 0 0]
                } else {
                    foreach {a b c n} $x {
                        append xor [binary format ccc $c $b $a]
                    }
                }
            }
        }
    } elseif {$bpp == 32} {
        foreach line $colors {
            foreach x $line {
                if {![llength $x]} {
                    append xor [binary format cccc 0 0 0 0]
                } else {
                    foreach {a b c n} $x {
                        if {$n == ""} {set n 0}
                        append xor [binary format cccc $c $b $a $n]
                    }
                }
            }
        }
    }
    return $xor
}

# translates a Tk image into a list of colors in the {r g b} format
# one element per pixel and {} designating transparent
# used by writeIcon when writing from a Tk image
proc ::ico::getColorListFromImage {img} {
    package require Tk
    set w [image width $img]
    set h [image height $img]
    set r {}
    for {set y [expr $h - 1]} {$y > -1} {incr y -1} {
        set l {}
        for {set x 0} {$x < $w} {incr x} {
            if {[$img transparency get $x $y]} {
                lappend l {}
            } else {
                lappend l [$img get $x $y]
            }
        }
        lappend r $l
    }
    return $r
}

# creates a palette from a list of colors in the decimal list format
# a palette consists of 3 values, the number of colors, the palette entry itself,
# and the color list transformed to point to palette entries instead of color names
# the palette entry itself is stored as 32bpp in "G B R padding" order
proc ::ico::getPaletteFromColors {colors} {
    set palette "\x00\x00\x00\x00"
    array set tpal {{0 0 0} 0}
    set new {}
    set i 1
    foreach line $colors {
        set tline {}
        foreach x $line {
            if {$x eq ""} {lappend tline {}; continue}
            if {![info exists tpal($x)]} {
                foreach {a b c n} $x {
                    append palette [binary format cccc $c $b $a 0]
                }
                set tpal($x) $i
                incr i
            }
            lappend tline $tpal($x)
        }
        lappend new $tline
    }
    return [list $i $palette $new]
}

# calculate byte size of an icon.
# often passed $w twice because $h is double $w in the binary data
proc ::ico::calcSize {w h bpp {offset 0}} {
    set s [expr {int(($w*$h) * ($bpp/8.0)) \
                     + ((($w*$h) + ($h*($w%32)))/8) + $offset}]
    if {$bpp <= 8} { set s [expr {$s + (1 << ($bpp + 2))}] }
    return $s
}

# read a Device Independent Bitmap from the current offset, return:
#        {width height depth palette XOR_mask AND_mask}
proc ::ico::readDIB {fh} {
    binary scan [read $fh 16] x4iix2s w h bpp
    set h [expr {$h / 2}]
    seek $fh 24 current

    set palette [list]
    if {$bpp == 1 || $bpp == 4 || $bpp == 8} {
        set colors [read $fh [expr {1 << ($bpp + 2)}]]
        foreach {b g r x} [split $colors {}] {
            lappend palette [formatColor $r $g $b]
        }
    } elseif {$bpp == 16 || $bpp == 24 || $bpp == 32} {
        # do nothing here
    } else {
        return -code error "unsupported color depth: $bpp"
    }

    set xor  [read $fh [expr {int(($w * $h) * ($bpp / 8.0))}]]
    set and1 [read $fh [expr {(($w * $h) + ($h * ($w % 32))) / 8}]]

    set and {}
    set row [expr {((($w - 1) / 32) * 32 + 32) / 8}]
    set len [expr {$row * $h}]
    for {set i 0} {$i < $len} {incr i $row} {
        binary scan [string range $and1 $i [expr {$i + $row}]] B$w tmp
        append and $tmp
    }

    return [list $w $h $bpp $palette $xor $and]
}

# read a Device Independent Bitmap from raw data, return:
#        {width height depth palette XOR_mask AND_mask}
proc ::ico::readDIBFromData {data loc} {
    # Read info from location
    binary scan $data @${loc}x4iix2s w h bpp
    set h [expr {$h / 2}]
    # Move over w/h/bpp info + magic offset to start of DIB
    set cnt [expr {$loc + 16 + 24}]

    set palette [list]
    if {$bpp == 1 || $bpp == 4 || $bpp == 8} {
        # Could do: [binary scan $data @${cnt}c$len colors]
        # and iter over colors, but this is more consistent with $fh version
        set len    [expr {1 << ($bpp + 2)}]
        set colors [string range $data $cnt [expr {$cnt + $len - 1}]]
        foreach {b g r x} [split $colors {}] {
            lappend palette [formatColor $r $g $b]
        }
        incr cnt $len
    } elseif {$bpp == 16 || $bpp == 24 || $bpp == 32} {
        # do nothing here
    } else {
        return -code error "unsupported color depth: $bpp"
    }

    # Use -1 to account for string range inclusiveness
    set end  [expr {$cnt + int(($w * $h) * ($bpp / 8.0)) - 1}]
    set xor  [string range $data $cnt $end]
    set and1 [string range $data [expr {$end + 1}] \
                  [expr {$end + ((($w * $h) + ($h * ($w % 32))) / 8) - 1}]]

    set and {}
    set row [expr {((($w - 1) / 32) * 32 + 32) / 8}]
    set len [expr {$row * $h}]
    for {set i 0} {$i < $len} {incr i $row} {
        # Has to be decoded by row, in order
        binary scan [string range $and1 $i [expr {$i + $row}]] B$w tmp
        append and $tmp
    }

    return [list $w $h $bpp $palette $xor $and]
}

proc ::ico::getIconListICO {file} {
    set fh [open $file r]
    fconfigure $fh -eofchar {} -encoding binary -translation lf

    if {"[getword $fh] [getword $fh]" ne "0 1"} {
        return -code error "not an icon file"
    }
    close $fh
    return 0
}

proc ::ico::getIconListICODATA {data} {
    if {[binary scan $data sss h1 h2 num] != 3 || $h1 != 0 || $h2 != 1} {
        return -code error "not icon data"
    }
    return 0
}

proc ::ico::getIconListBMP {file} {
    set fh [open $file]
    if {[read $fh 2] != "BM"} { close $fh; return -code error "not a BMP file" }
    close $fh
    return 0
}

proc ::ico::getIconListEXE {file} {
    variable RES

    set file [file normalize $file]
    if {[FindResources $file] > -1} {
        return $RES($file,group,names)
    } else {
        return ""
    }
}

# returns a list of images that make up the named icon
# as tuples {name width height bpp}. Called by [iconMembers]
proc ::ico::getIconMembersICO {file name} {
    variable RES

    if {$name ne "0"} { return -code error "no icon \"$name\"" }
    set file [file normalize $file]
    if {[info exists RES($file,group,$name,members)]} {
        set ret ""
        foreach x $RES($file,group,$name,members) {
            lappend ret [linsert $RES($file,icon,$x,data) 0 $x]
        }
        return $ret
    }

    set fh [open $file r]
    fconfigure $fh -eofchar {} -encoding binary -translation lf

    # both words must be read to keep in sync with later reads
    if {"[getword $fh] [getword $fh]" ne "0 1"} {
        close $fh
        return -code error "not an icon file"
    }

    set ret ""
    set num [getword $fh]
    for {set i 0} {$i < $num} {incr i} {
        set info ""
        lappend RES($file,group,$name,members) $i
        lappend info [scan [read $fh 1] %c] [scan [read $fh 1] %c]
        set bpp [scan [read $fh 1] %c]
        if {$bpp == 0} {
            set orig [tell $fh]
            seek $fh 9 current
            seek $fh [expr {[getdword $fh] + 14}] start
            lappend info [getword $fh]
            seek $fh $orig start
        } else {
            lappend info [expr {int(sqrt($bpp))}]
        }
        lappend ret [linsert $info 0 $i]
        set RES($file,icon,$i,data) $info
        seek $fh 13 current
    }
    close $fh
    return $ret
}

# returns a list of images that make up the named icon
# as tuples {name width height bpp}. Called by [iconMembers]
proc ::ico::getIconMembersICODATA {data} {
    if {[binary scan $data sss h1 h2 num] != 3 || $h1 != 0 || $h2 != 1} {
        return -code error "not icon data"
    }
    set r {}
    set cnt 6
    for {set i 0} {$i < $num} {incr i} {
        if {[binary scan $data @${cnt}ccc w h bpp] != 3} {
            return -code error "error decoding icon data"
        }
        incr cnt 3
        set info [list $i $w $h]
        if {$bpp == 0} {
            set off [expr {$cnt + 9}]
            binary scan $data @${off}i off
            incr off 14
            binary scan $data @${off}s bpp
            lappend info $bpp
        } else {
            lappend info [expr {int(sqrt($bpp))}]
        }
        lappend r $info
        incr cnt 13
    }
    return $r
}

# returns a list of images that make up the named icon
# as tuples {name width height bpp}. Called by [iconMembers]
proc ::ico::getIconMembersBMP {file {name 0}} {
    if {$name ne "0"} { return -code error "no icon \"$name\"" }
    set fh [open $file]
    if {[read $fh 2] != "BM"} { close $fh; return -code error "not a BMP file" }
    seek $fh 14 start
    binary scan [read $fh 16] x4iix2s w h bpp
    close $fh
    return [list 1 $w $h $bpp]
}

# returns a list of images that make up the named icon
# as tuples {name width height bpp}. Called by [iconMembers]
proc ::ico::getIconMembersEXE {file name} {
    variable RES
    set file [file normalize $file]
    FindResources $file
    if {![info exists RES($file,group,$name,members)]} { return -code error "no icon \"$name\"" }
    set ret ""
    foreach x $RES($file,group,$name,members) {
        lappend ret [linsert $RES($file,icon,$x,data) 0 $x]
    }
    return $ret
}

# returns an icon in the form:
#       {width height depth palette xor_mask and_mask}
proc ::ico::getRawIconDataICO {file name} {
    set fh [open $file r]
    fconfigure $fh -eofchar {} -encoding binary -translation lf

    # both words must be read to keep in sync with later reads
    if {"[getword $fh] [getword $fh]" ne "0 1"} {
        close $fh
        return -code error "not an icon file"
    }
    set num [getword $fh]
    if {![string is integer -strict $name] || $name < 0 || $name >= $num} { return -code error "no icon \"$name\"" }

    seek $fh [expr {(16 * $name) + 12}] current
    seek $fh [getdword $fh] start

    # readDIB returns: {w h bpp palette xor and}
    set dib [readDIB $fh]

    close $fh
    return $dib
}

# returns an icon in the form:
#       {width height depth palette xor_mask and_mask}
proc ::ico::getRawIconDataICODATA {data name} {
    if {[binary scan $data sss h1 h2 num] != 3 || $h1 != 0 || $h2 != 1} {
        return -code error "not icon data"
    }
    if {![string is integer -strict $name] || $name < 0 || $name >= $num} {
        return -code error "No icon $name"
    }
    # Move to ico location
    set cnt [expr {6 + (16 * $name) + 12}]
    binary scan $data @${cnt}i loc

    # readDIB returns: {w h bpp palette xor and}
    set dib [readDIBFromData $data $loc]

    return $dib
}

# returns an icon in the form:
#        {width height depth palette xor_mask and_mask}
proc ::ico::getRawIconDataBMP {file {name 1}} {
    if {$name ne "1"} {return -code error "No icon \"$name\""}
    set fh [open $file]
    if {[read $fh 2] != "BM"} { close $fh; return -code error "not a BMP file" }
    seek $fh 14 start
    binary scan [read $fh 16] x4iix2s w h bpp
    seek $fh 24 current

    set palette [list]
    if {$bpp == 1 || $bpp == 4 || $bpp == 8} {
        set colors [read $fh [expr {1 << ($bpp + 2)}]]
        foreach {b g r x} [split $colors {}] {
            lappend palette [formatColor $r $g $b]
        }
    } elseif {$bpp == 16 || $bpp == 24 || $bpp == 32} {
        # do nothing here
    } else {
        return -code error "unsupported color depth: $bpp"
    }

    set xor  [read $fh [expr {int(($w * $h) * ($bpp / 8.0))}]]
    set and [string repeat 0 [expr {$w * $h}]]
    close $fh

    return [list $w $h $bpp $palette $xor $and]
}

# returns an icon in the form:
#        {width height depth palette xor_mask and_mask}
proc ::ico::getRawIconDataEXE {file name} {
    variable RES

    set file [file normalize $file]
    FindResources $file

    if {![info exists RES($file,icon,$name,offset)]} { error "No icon \"$name\"" }
    set fh [open $file]
    fconfigure $fh -eofchar {} -encoding binary -translation lf
    seek $fh $RES($file,icon,$name,offset) start

    # readDIB returns: {w h bpp palette xor and}
    set dib [readDIB $fh]
    close $fh
    return $dib
}

proc ::ico::writeIconICO {file name w h bpp palette xor and} {
    if {![file exists $file]} {
        set fh [open $file w+]
        fconfigure $fh -eofchar {} -encoding binary -translation lf
        set num 0
    } else {
        set fh [open $file r+]
        fconfigure $fh -eofchar {} -encoding binary -translation lf
        if {"[getword $fh] [getword $fh]" ne "0 1"} {
            close $fh
            return -code error "not an icon file"
        }
        set num [getword $fh]
        seek $fh [expr {6 + (16 * $num)}] start
    }

    set size [expr {[string length $palette] + [string length $xor] + [string length $and]}]
    set newicon [binary format iiissiiiiii 40 $w [expr {$h * 2}] 1 $bpp 0 $size 0 0 0 0]$palette$xor$and

    set data {}
    for {set i 0} {$i < $num} {incr i} {
        binary scan [read $fh 24] ix16i a b
        seek $fh -24 current
        lappend data [read $fh [expr {$a + $b}]]
    }

    if {![string is integer -strict $name] || $name < 0 || $name >= $num} {
        set name [llength $data]
        lappend data $newicon
    } else {
        set data [lreplace $data $name $name $newicon]
    }
    set num [llength $data]

    seek $fh 0 start
    bputs $fh sss 0 1 $num
    set offset [expr {6 + (16 * $num)}]
    foreach x $data {
        binary scan $x x4iix2s w h bpp
        set len [string length $x]
        # use original height in icon table header
        bputs $fh ccccssii $w [expr {$h / 2}] [expr {$bpp <= 8 ? 1 << $bpp : 0}] 0 0 $bpp $len $offset
        incr offset $len
    }
    puts -nonewline $fh [join $data {}]
    close $fh

    return $name
}

proc ::ico::writeIconICODATA {file name w h bpp palette xor and} {
    upvar 2 [file tail $file] input
    if {![info exists input] || ([binary scan $input sss h1 h2 num] != 3 || $h1 != 0 || $h2 != 1)} {
        set num 0
    }

    set size [expr {[string length $palette] + [string length $xor] + [string length $and]}]
    set newicon [binary format iiissiiiiii 40 $w [expr {$h * 2}] 1 $bpp 0 $size 0 0 0 0]$palette$xor$and

    set readpos [expr {6 + (16 * $num)}]
    set data {}
    for {set i 0} {$i < $num} {incr i} {
        binary scan $input @{$readpos}ix16i a b
        lappend data [string range $data $readpos [expr {$readpos + $a + $b}]]
        incr readpos [expr {$readpos + $a + $b}]
    }

    if {![string is integer -strict $name] || $name < 0 || $name >= $num} {
        set name [llength $data]
        lappend data $newicon
    } else {
        set data [lreplace $data $name $name $newicon]
    }
    set num [llength $data]

    set new [binary format sss 0 1 $num]
    set offset [expr {6 + (16 * $num)}]
    foreach x $data {
        binary scan $x x4iix2s w h bpp
        set len [string length $x]
        # use original height in icon table header
        append new [binary format ccccssii $w [expr {$h / 2}] [expr {$bpp <= 8 ? 1 << $bpp : 0}] 0 0 $bpp $len $offset]
        incr offset $len
    }
    set input $new
    append input [join $data {}]

    return $name
}

proc ::ico::writeIconBMP {file name w h bpp palette xor and} {
    set fh [open $file w+]
    fconfigure $fh -eofchar {} -encoding binary -translation lf
    set size [expr {[string length $palette] + [string length $xor]}]
    # bitmap header: magic, file size, reserved, reserved, offset of bitmap data
    bputs $fh a2issi BM [expr {14 + 40 + $size}] 0 0 54
    bputs $fh iiissiiiiii 40 $w $h 1 $bpp 0 $size 0 0 0 0
    puts -nonewline $fh $palette$xor
    close $fh
}

proc ::ico::writeIconEXE {file name w h bpp palette xor and} {
    variable RES

    set file [file normalize $file]
    set members [getIconMembersEXE $file $name]

    if {![info exists RES($file,icon,$name,data)]} {
        return -code error "no icon \"$name\""
    }
    if {![string match "* $w $h $bpp" $RES($file,icon,$name,data)]} {
        return -code error "icon format differs from original"
    }
    
    set fh [open $file r+]
    fconfigure $fh -eofchar {} -encoding binary -translation lf
    seek $fh [expr {$RES($file,icon,$name,offset) + 40}] start

    puts -nonewline $fh $palette$xor$and
    close $fh
}

proc ::ico::FindResources {file} {
    variable RES

    if {[info exists RES($file,group,names)]} {
        return [llength $RES($file,group,names)]
    }

    set fh [open $file]
    fconfigure $fh -eofchar {} -encoding binary -translation lf
    if {[read $fh 2] ne "MZ"} {
        close $fh
        return -code error "unknown file format"
    }
    seek $fh 60 start
    seek $fh [getword $fh] start
    set sig [read $fh 4]
    seek $fh -4 current
    if {$sig eq "PE\000\000"} {
        return [FindResourcesPE $fh $file]
    } elseif {[string match NE* $sig]} {
        return [FindResourcesNE $fh $file]
    } else {
        return -code error "unknown file format"
    }
}

# parse the resource table of 16 bit windows files for icons
proc ::ico::FindResourcesNE {fh file} {
    variable RES

    seek $fh 36 current
    seek $fh [expr {[getword $fh] - 38}] current
    set base [tell $fh]
    set shift [expr {int(pow(2, [getushort $fh]))}]
    while {[set type [expr {[getushort $fh] & 0x7fff}]] != 0} {
        set num [getushort $fh]
        if {$type != 3 && $type != 14} {
            seek $fh [expr {($num * 12) + 4}] current
            continue
        }
        set type [string map {3 icon 14 group} $type]
        seek $fh 4 current
        for {set i 0} {$i < $num} {incr i} {
            set offset [expr {[getushort $fh] * $shift}]
            seek $fh 4 current
            set name [getNEResName $fh $base [getushort $fh]]
            set RES($file,$type,$name,offset) $offset
            lappend RES($file,$type,names) $name
            seek $fh 4 current
        }
    }
    if {[array names RES $file,*] == ""} {
        close $fh
        return -1
    }
    foreach x [array names RES $file,group,*,offset] {
        seek $fh [expr {$RES($x) + 4}] start
        binary scan [read $fh 2] s a
        set x [lindex [split $x ,] 2]
        for {set i 0} {$i < $a} {incr i} {
            binary scan [read $fh 14] x12s n
            lappend RES($file,group,$x,members) $n
        }
    }
    foreach x [array names RES $file,icon,*,offset] {
        seek $fh [expr {$RES($x)}] start
        set x [lindex [split $x ,] 2]
        binary scan [read $fh 16] x4iix2s w h bpp
        set RES($file,icon,$x,data) [list $w [expr {$h / 2}] $bpp]
    }
    close $fh
    return [llength $RES($file,group,names)]
}

proc ::ico::getNEResName {fh base data} {
    if {$data == 0} {
        return 0
    }
    binary scan $data b* tmp
    if {[string index $tmp 0] == 0} {
        set cur [tell $fh]
        seek $fh [expr {$data + $base}] start
        binary scan [read $fh 1] c len
        set name [read $fh $len]
        seek $fh $cur start
        return $name
    } else {
        return [expr {$data & 0x7fff}]
    }
}

# parse the resource tree of 32 bit windows files for icons
proc ::ico::FindResourcesPE {fh file} {
    variable RES

    # find the .rsrc section by reading the coff header
    binary scan [read $fh 24] x6sx12s sections headersize
    seek $fh $headersize current
    for {set i 0} {$i < $sections} {incr i} {
        binary scan [read $fh 40] a8x4ix4i type baserva base
        if {[string match .rsrc* $type]} {break}
    }
    # no resource section found = no icons
    if {![string match .rsrc* $type]} {
        close $fh
        return -1
    }
    seek $fh $base start

    seek $fh 12 current
    # number of entries in the resource table. each one is a different resource type
    set entries [expr {[getushort $fh] + [getushort $fh]}]
    for {set i 0} {$i < $entries} {incr i} {
        set type [getulong $fh]
        set offset [expr {[getulong $fh] & 0x7fffffff}]
        if {$type != 3 && $type != 14} {continue}
        set type [string map {3 icon 14 group} $type]

        set cur [tell $fh]
        seek $fh [expr {$base + $offset + 12}] start
        set entries2 [expr {[getushort $fh] + [getushort $fh]}]
        for {set i2 0} {$i2 < $entries2} {incr i2} {
            set name [getPEResName $fh $base [getulong $fh]]
            lappend RES($file,$type,names) $name
            set offset [expr {[getulong $fh] & 0x7fffffff}]

            set cur2 [tell $fh]
            seek $fh [expr {$offset + $base + 12}] start
            set entries3 [expr {[getushort $fh] + [getushort $fh]}]
            for {set i3 0} {$i3 < $entries3} {incr i3} {
                seek $fh 4 current
                set offset [expr {[getulong $fh] & 0x7fffffff}]
                set cur3 [tell $fh]

                seek $fh [expr {$offset + $base}] start
                set rva [getulong $fh]
                set RES($file,$type,$name,offset) [expr {$rva - $baserva + $base}]

                seek $fh $cur3 start
            }

            seek $fh $cur2 start
        }
        seek $fh $cur start
    }
    if {[array names RES $file,*] == ""} {
        close $fh
        return -1
    }
    foreach x [array names RES $file,group,*,offset] {
        seek $fh [expr {$RES($x) + 4}] start
        binary scan [read $fh 2] s a
        set x [lindex [split $x ,] 2]
        for {set i 0} {$i < $a} {incr i} {
            binary scan [read $fh 14] x12s n
            lappend RES($file,group,$x,members) $n
        }
    }
    foreach x [array names RES $file,icon,*,offset] {
        seek $fh [expr {$RES($x)}] start
        set x [lindex [split $x ,] 2]
        binary scan [read $fh 16] x4iix2s w h bpp
        set RES($file,icon,$x,data) [list $w [expr {$h / 2}] $bpp]
    }

    close $fh
    return [llength $RES($file,group,names)]
}

proc ::ico::getPEResName {fh start data} {
    if {($data & 0x80000000) != 0} {
        set cur [tell $fh]
        seek $fh [expr {($data & 0x7fffffff) + $start}] start
        set len [getushort $fh]
        set name [read $fh [expr {$len * 2}]]
        seek $fh $cur start
        return [encoding convertfrom unicode $name]
    } else {
        return $data
    }
}

interp alias {} ::ico::getIconListDLL    {} ::ico::getIconListEXE
interp alias {} ::ico::getIconMembersDLL {} ::ico::getIconMembersEXE
interp alias {} ::ico::getRawIconDataDLL {} ::ico::getRawIconDataEXE
interp alias {} ::ico::writeIconDLL      {} ::ico::writeIconEXE
interp alias {} ::ico::getIconListICL    {} ::ico::getIconListEXE
interp alias {} ::ico::getIconMembersICL {} ::ico::getIconMembersEXE
interp alias {} ::ico::getRawIconDataICL {} ::ico::getRawIconDataEXE
interp alias {} ::ico::writeIconICL      {} ::ico::writeIconEXE

package provide ico 1.0.3
