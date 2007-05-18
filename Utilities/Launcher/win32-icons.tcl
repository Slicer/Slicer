


package require Tk
package require Img
package require ico

source ico.debug.tcl

proc modifyLauncherIcons { logoDir exe {ico ""} } {
  set iconSpecs {
    {0 32 4 16}
    {1 16 4 16}
    {2 32 8 256}
    {3 48 8 256}
    {4 48 4 16}
    {5 16 8 256}
  }

  foreach spec $iconSpecs {


    foreach {index dim bpp numColors} $spec {}

    set im [image create photo]
    #puts "$im read $logoDir/3DSlicerLogo-DesktopIcon-${dim}x${dim}x${numColors}.png"
    $im read $logoDir/3DSlicerLogo-DesktopIcon-${dim}x${dim}x${numColors}.png
    #removeTransparency $im
    #puts "::ico::writeIcon $exe $index $bpp $im"
    ::ico::writeIcon $exe $index $bpp $im

  }

  if { $ico != "" } {
    set im [image create photo]
    $im read $logoDir/3DSlicerLogo-DesktopIcon-32x32x256.png
    ::ico::writeIcon $ico 0 $bpp $im -type ICO
  }
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



