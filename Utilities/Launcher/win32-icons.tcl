


package require Tk
package require Img
package require ico


proc modifyLauncherIcons {logoDir exe} {
  set iconSpecs {
    {0 32 4 16}
    {1 16 4 16}
    {2 32 8 256}
    {3 48 8 256}
    {4 48 4 16}
    {5 16 8 256}
  }

  foreach spec $iconSpecs {

    puts $spec

    foreach {index dim bpp numColors} $spec {}
    set im [image create photo]
    $im read $logoDir/3DSlicerLogo-DesktopIcon-${dim}x${dim}x${numColors}.png
    ::ico::writeIcon $exe $index $bpp $im

  }
}





