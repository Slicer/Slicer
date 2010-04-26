#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

if { [file exists build_BRAINSTracer.tcl] && [file exists BRAINSTracer.cxx ] } {
    puts "Don't run this script inside the source directory!!"
    puts "Please build inside the directory containing the BRAINSTracer source directory"
    exit 1
}

set ::TOP [pwd]
set ::CMAKE $::TOP/Cmake-build/bin/cmake
set ::COMPILE_TYPE DEBUG
set ::CMAKE_GENERATOR ""
set ::MAKE make
set ::Update ""
set ::Clean ""
set ::BuildDirSuffix ""

set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--KDE" {
            set CMAKE_GENERATOR "-G KDevelop3"
        }
        "--update" {
            set ::Update "--update"
        }
        "--Clean" {
            set ::Clean "--Clean"
        }
        "--Coverage" {
            set ::BuildDirSuffix "-coverage"
        }
    }
}

if {[info exists ::env(CVS)]} {
    set ::CVS "{$::env(CVS)}"
} else {
    set ::CVS cvs
}

# for subversion repositories (Sandbox)
if {[info exists ::env(SVN)]} {
    set ::SVN $::env(SVN)
} else {
    set ::SVN svn
}

#initialize platform variables
switch $tcl_platform(os) {
    "SunOS" {
        set isSolaris 1
        set isWindows 0
        set isDarwin 0
        set isLinux 0
    }
    "Linux" { 
        set isSolaris 0
        set isWindows 0
        set isDarwin 0
        set isLinux 1
    }
    "Darwin" { 
        set isSolaris 0
        set isWindows 0
        set isDarwin 1
        set isLinux 0
    }
    default { 
        set isSolaris 0
        set isWindows 1
        set isDarwin 0
        set isLinux 0
    }
}
#
# 
proc runcmd {args} {
    global isWindows
    puts "running: $args"

    # print the results line by line to provide feedback during long builds
    # interleaves the results of stdout and stderr, except on Windows
    if { $isWindows } {
        # Windows does not provide native support for cat
        set fp [open "| $args" "r"]
    } else {
        set fp [open "| $args |& cat" "r"]
    }
    while { ![eof $fp] } {
        gets $fp line
        puts $line
    }
    set ret [catch "close $fp" res] 
    if { $ret } {
        puts stderr $res
        if { $isWindows } {
            # Does not work on Windows
        } else {
            error $ret
        }
    } 
}

#
# CMAKE
set ::CMAKE $::TOP/CMake-build/bin/cmake
if { ![file exists $::CMAKE] || ![file executable $::CMAKE] || $::Update != "" } {
    runcmd $::CVS -d :pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake login
#    runcmd $::CVS -d :pserver:anonymous@www.cmake.org:/cvsroot/CMake checkout -r CMake-2-6-0 CMake
    runcmd $::CVS -d :pserver:anonymous@www.cmake.org:/cvsroot/CMake checkout CMake
    file mkdir CMake-build
    cd CMake-build
    runcmd ../CMake/bootstrap
    runcmd $::MAKE
    cd $::TOP
}


#
# ITK
if { ![file exists $::TOP/Insight-build$::BuildDirSuffix/bin/libITKCommon.a] || \
    $::Update != "" || $::Clean != "" } {
    runcmd $::CVS -d :pserver:anoncvs:@www.itk.org:/cvsroot/Insight login
    runcmd $::CVS -d :pserver:anoncvs@www.itk.org:/cvsroot/Insight checkout Insight
    if { $::Clean != "" } {
            runcmd rm -fr Insight-build$::BuildDirSuffix
    }
    file mkdir Insight-build$::BuildDirSuffix
    cd Insight-build$::BuildDirSuffix
    runcmd $::CMAKE $::CMAKE_GENERATOR \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DITK_USE_REVIEW:BOOL=ON \
        -DITK_USE_ORIENTED_IMAGE_DIRECTION:BOOL=ON \
        -DCMAKE_BUILD_TYPE:STRING=$::COMPILE_TYPE ../Insight
    runcmd $::MAKE
    cd $::TOP
}

#
#
# build brainsdemonwarp
if { $::Update != "" } {
    runcmd $::SVN update brainsdemonwarp
}

if { $::Clean != "" } {
        runcmd rm -fr brainsdemonwarp-build$::BuildDirSuffix
}

file mkdir brainsdemonwarp-build$::BuildDirSuffix
cd brainsdemonwarp-build$::BuildDirSuffix
runcmd $::CMAKE  $::CMAKE_GENERATOR \
-DCMAKE_BUILD_TYPE:STRING=$::COMPILE_TYPE \
-DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
-DBUILD_SHARED_LIBS:BOOL=OFF \
-DITK_DIR:FILE_PATH=$::TOP/Insight-build$::BuildDirSuffix \
../brainsdemonwarp

cd $::TOP/brainsdemonwarp-build$::BuildDirSuffix
if { $::BuildDirSuffix != "-coverage" } {
} else {
        runcmd $::MAKE NightlyCoverage
}

