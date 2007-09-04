#!/usr/bin/wish -f

# This script puts up a small GUI that allows FIPS users to enter search
# fields to search their  fips.database.dat files for existing first level
# functional analyses. Users can select individual studies from the
# set of returned match results, and write mrml2 and mrml3 files
# (for slicer2 and slicer3) describing scenes that contain the
# anatomical scan (brain.mgz), the example functional scan (example_func),
# all zstat files (in the form of symbolic links slicer must create due to
# a current inability to interpret filenames that include numbers),
# the aparc+aseg.mgz file from the freesurfer analysis, the
# lh.pial pial surface file from freesurfer, and the exf2anat.fsl.mat
# registration matrix that transforms all analysis results into
# anatomical space for visualization.
# usage: ./fips2mrml

    #--- How this works:
    #--- basically, we parse the file fips.database.dat to find all first-level analyses
    #--- then we list those in the GUI
    #--- a user selects one to load
    #--- we retrieve the appropriate analysis files to load
    #--- and we map the BIRNID to a FreeSurfer ID through FSBIRNID.dat
    #--- then we navigate the FreeSurfer directory tree and grab all the appropriate data
    #--- and we also find a matrix that maps the anatomical back to the example functional
    #--- used during the FIPS analysis (anat2exf.mat)
    #--- we have to add some additional xforms to display the volume correctly.


proc fips2mrml_InitializeParseFields { } {

    #--- clear listbox
    if { [info exists ::fips2mrml_Listbox] } {
        $::fips2mrml_Listbox delete 0 end
    }
    
    #--- number of entry in fips database file
    set ::fips2mrml_Nth ""
    set ::fips2mrml_Nth_index 0

    #--- subject's id
    set ::fips2mrml_BIRNId ""
    set ::fips2mrml_BIRNId_index 1

    #--- not sure
    set ::fips2mrml_RecSite ""
    set ::fips2mrml_RecSite_index 2

    #--- acquisition site
    set ::fips2mrml_AcqSite ""
    set ::fips2mrml_AcqSite_index 3

    #--- name given to a project
    set ::fips2mrml_Proj ""
    set ::fips2mrml_Proj_index 4

    #--- name given to a study
    set ::fips2mrml_StudyName ""
    set ::fips2mrml_StudyName_index 5

    #--- not sure
    set ::fips2mrml_StudyId ""
    set ::fips2mrml_StudyID_index 6

    #--- name given to a visit
    set ::fips2mrml_VisitName ""
    set ::fips2mrml_VisitName_index 7

    #--- visit id ?
    set ::fips2mrml_VisitId ""
    set ::fips2mrml_VisitId_index 8

    #---  functional or structural?
    set ::fips2mrml_RunType ""
    set ::fips2mrml_RunType_index 9

    #---  name of associated paradigm
    set ::fips2mrml_Paradigm ""
    set ::fips2mrml_Paradigm_index 10
    
    #---  run number
    set ::fips2mrml_RunNo ""
    set ::fips2mrml_RunNo_index 11

    #---  analysis level: (none, FirstLevel, FFxVisit)
    set ::fips2mrml_AnalysisLevel ""
    set ::fips2mrml_AnalysisLevel_index 12

    #---  name of the analysis or format? (ANALYZE_7.5, DICOM, NIFTI, filename.flac)    
    set ::fips2mrml_AnalysisName ""
    set ::fips2mrml_AnalysisName_index 13

    #--- directory from which xml image wrapper can be found, or data can be
    #--- found with the FIPS directory structure in mind.
    set ::fips2mrml_XMLDir ""    
    set ::fips2mrml_XMLDir_index 14
    
    #--- for now keep these constant.
    set ::fips2mrml_AnalysisLevel "FirstLevel"
    set ::fips2mrml_RunType "functional"
    set ::fips2mrml_MRMLFile "testScene"

    if { [info exists ::fips2mrml_Message]} {
        $::fips2mrml_Message config -text ""
    }
}



proc fips2mrml_InitializeNumToCharArray { } {

    #--- only works now for up to fifty stats files.
    #--- type more if you need more
    set ::fips2mrml_CharArray " Zero One Two Three Four Five Six Seven Eight Nine Ten Eleven Twelve Thirteen Fourteen Fifteen Sixteen Seventeen Eighteen Nineteen Twenty TwentyOne TwentyTwo TwentyThree TwentyFour TwentyFive TwentySix TwentySevenTwentyEight TwentyNine Thirty Thirtyone Thirtytwo Thirtythree Thirtyfour Thirtyfive Thirtysix Thirtysevel Thirtyeight Thirtynine Forty Fortyone Fortytwo Fortythree Fortyfour Fortyfive Fortysix Fortyseven Fortyeight Fortynine Fifty"
}



proc fips2mrml_InitializeGlobals { } {

    #---  set lists of lists used in search to null
    set ::fipsmrml_SearchInput ""
    set ::fips2mrml_SearchResults ""
    set ::fips2mrml_MatchSelection ""

    #--- list of functional scans to load
    set ::fips2mrml_ExampleFuncVolume ""

    #--- list of structural scans to load
    set ::fips2mrml_T1Volume ""
    set ::fips2mrml_T2Volume ""

    #--- list of statistics volumes to load
    set ::fips2mrml_ZstatVolumes ""
    set ::fips2mrml_ZstatVolumeNames ""
    set ::fips2mrml_TstatVolumes ""
    set ::fips2mrml_CopeVolumes ""
    set ::fips2mrml_StatisticsBaseNameList ""

    #--- list of matrices
    # wjp note: matrix in data\fBIRN-AHM2006\scanVisit__0006__0002\MRI__0001\sirp1\Analysis\Original__0001\sirp-hp65-stc-to7-gam.feat\reg\freesurfer
    set ::fips2mrml_exf2anatList ""

    #--- mrml file name
    set ::fips2mrml_mrmlFilename ""

    #--- freesurfer brain, labels and pial surface
    set ::fips2mrml_FreeSurferBrainMGZ ""
    set ::fips2mrml_FreeSurferLabels ""
    set ::fips2mrml_FreeSurferModels ""
    set ::fips2mrml_FreeSurferId ""
    set ::fips2mrml_FreeSurferSubjectPath ""
}



proc fips2mrml_GetSearchInput { } {

    #--- clear search input
    set ::fips2mrml_SearchInput ""
    #--- construct it fresh
    set ::fips2mrml_SearchInput [ lappend ::fips2mrml_SearchInput \
                          $::fips2mrml_Nth\
                          $::fips2mrml_BIRNId \
                          $::fips2mrml_RecSite \
                          $::fips2mrml_AcqSite \
                          $::fips2mrml_Proj \
                          $::fips2mrml_StudyName \
                          $::fips2mrml_StudyId \
                          $::fips2mrml_VisitName \
                          $::fips2mrml_VisitId \
                          $::fips2mrml_RunType \
                          $::fips2mrml_Paradigm \
                          $::fips2mrml_RunNo \
                          $::fips2mrml_AnalysisLevel \
                          $::fips2mrml_AnalysisName \
                          $::fips2mrml_XMLDir ]
}



proc fips2mrml_GetMatchingXMLFiles { } {


    puts "Searching database for matches to: $::fips2mrml_SearchInput."
    fips2mrml_DisplayMessage "Searching database for matches..."
    set fid [ open $::fips_database_dat r ]
    while { ! [eof $fid] } {
        #--- get a line of the database file.
        gets $fid line
        set dbColumns [ regexp -inline -all -- {\S+} $line ]

        #--- if the line is a comment or empty line, ignore it.
        set comment [ lindex $dbColumns 0 ]
        if { $comment != "\#" && $dbColumns != "" } {

            #--- check to see if we have the correct number of fields
            set len1 [ llength $::fips2mrml_SearchInput ]
            set len2 [ llength $dbColumns ]
            if { $len1 != $len2 } {
                puts "number of possible search terms ($len1) is different than number of fields in file entries ($len2)."
                fips2mrml_DisplayMessage "number of possible search terms ($len1) is different than number of fields in file entries ($len2)."
                break
            } else {

                #--- find matches
                set match 1
                for { set i 0 } { $i < $len1 } { incr i } {
                    set val1 [ lindex $::fips2mrml_SearchInput  $i ]
                    set val2 [ lindex $dbColumns $i ]
                    if { ($val1 != "") && ($val1 != $val2) } {
                        set match 0
                        break
                    }
                }
                #--- report matches
                if { $match ==1 } {
                    set ::fips2mrml_SearchResults [ lappend ::fips2mrml_SearchResults $dbColumns ]
                }
            }
        }
    }
    close $fid
    set len [ llength $::fips2mrml_SearchResults ]
    puts "Found $len matches."
    fips2mrml_DisplayMessage "Found $len matches."
    #--- if we have at least one match, return 1
    if { $len > 0 } {
        return 1
    } else {
        return 0
    }
}







proc fips2rmrml_GetFreeSurferID { } {

    set fid [ open $::fsbirnid_dat "r" ]
    while { ! [eof $fid] } {
        #--- get a line of the database file.
        gets $fid line
        #--- look to see if the first field is the birn ID.
        #--- if yes, set the freesurfer id and return.

        set id [ lindex $line 0 ]
        if { [ string compare $id $::fips2mrml_BIRNId ] == 0 } {
            set ::fips2mrml_FreeSurferId [ lindex $line 1 ]
            puts "BIRN ID: $::fips2mrml_BIRNId FreeSurfer ID: $::fips2mrml_FreeSurferId"
            fips2mrml_DisplayMessage "BIRN ID: $::fips2mrml_BIRNId FreeSurfer ID: $::fips2mrml_FreeSurferId"
            set ::fips2mrml_FreeSurferSubjectPath [ format "%s%s/" $::freesurfer_subjectsdir $::fips2mrml_FreeSurferId ]
            return 1
        }
    }
    puts "No corresponding FreeSurfer ID found in $::fsbirnid_dat"
    fips2mrml_DisplayMessage "No corresponding FreeSurfer ID found in $::fsbirnid_dat"
    return 0
}





proc fips2mrml_PrintMatchResults  { } {

    set len [ llength $::fips2mrml_SearchResults ]

    if { $len == 0 } {
        puts "No matches found."
        fips2mrml_DisplayMessage "No matches found."
    } else {
        puts "Matches:"
        for { set i 0 } { $i < $len } { incr i } {
            set val [ lindex $::fips2mrml_SearchResults $i ]
            puts "$i: $val"
        }
    }
}


proc fips2mrml_GetT1Volume {} {
    #--- search will have returned a .feat directory.
    #--- Navigate the standard directory tree up to find the T1

    puts "Getting T1 volumes...."
    fips2mrml_DisplayMessage "Getting T1 volumes...."
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set oldtmp ""
    #--- strip off the filename leaving only directory path
    #--- hopping up 4 levels to where we find the anatomicals
    for { set j 0 } { $j < 4 } { incr j } {
        #--- continue only if there's still a valid directory...
        if  { ([string compare $tmp $oldtmp ] != 0) && ($tmp != "") } {
            set oldtmp $tmp
            set tmp [ string trimright $tmp "/"]
            set k [ string last "/" $tmp ]
            if { $k > 0 } {
                set tmp [ string range $tmp 0 $k ]
            }
        }
    }
    #--- now construct the T1 DICOM directory
    set T1path [format "%s%s" $tmp "t1/Native/Original__0001/DICOM/" ]

    #--- find the list of files in the DICOM directory
    #--- below is the real line we want
    set fileList [ glob $T1path*.dcm -nocomplain]
    set flen [ llength $fileList ]        

    #--- now figure out the first and last file number...
    set gotDICOM 0
    for { set m 0 } { $m < $flen } { incr m } {
        #--- find ANY file in here
        set f [ lindex $fileList $m ]
        #--- is this a DICOM file? (.dcm)
        set n [ string last ".dcm" $f ]
        if { $n > 0 } {
            #--- this is a dicom file
            set gotDICOM 1
            break
        }
    }
    if { $gotDICOM } {
        #--- add the file with complete pathname to the list of T1 Volumes
        set ::fips2mrml_T1Volume [ lappend ::fips2mrml_T1Volume $f ]
    } else {
        #--- add a null entry to the list of T1 Volumes
        set ::fips2mrml_T1Volume [ lappend ::fips2mrml_T1Volume "" ] 
    }

    if {$gotDICOM } {
        puts "T1 volumes found:  $::fips2mrml_T1Volume"
        fips2mrml_DisplayMessage "T1 volumes found:  $::fips2mrml_T1Volume"
    } else {
        puts "No T1 volumes found."
        fips2mrml_DisplayMessage "No T1 volumes found."
    }
    return $gotDICOM
}


proc fips2mrml_GetT2Volume { } {
    #--- search will have returned a .feat directory.
    #--- Navigate the standard directory tree up to find the T2

    puts "Getting T2 volumes...."
    fips2mrml_DisplayMessage "Getting T2 volumes...."
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set oldtmp ""
    #--- strip off the filename leaving only directory path
    #--- hopping up 4 levels to where we find the anatomicals
    for { set j 0 } { $j < 4 } { incr j } {
        #--- continue only if there's still a valid directory...
        if  { ([string compare $tmp $oldtmp ] != 0) && ($tmp != "") } {
            set oldtmp $tmp
            set tmp [ string trimright $tmp "/"]
            set k [ string last "/" $tmp ]
            if { $k > 0 } {
                set tmp [ string range $tmp 0 $k ]
            }
        }
    }
    #--- now construct the T2 DICOM directory
    set T2path [format "%s%s" $tmp "t2/Native/Original__0001/DICOM/" ]

    #--- find the list of files in the DICOM directory
    #--- below is the real line we want
    set fileList [ glob $T2path*.dcm -nocomplain]
    set flen [ llength $fileList ]        

    #--- now figure out the first and last file number...
    set gotDICOM 0
    for { set m 0 } { $m < $flen } { incr m } {
        #--- find ANY file in here
        set f [ lindex $fileList $m ]
        #--- is this a DICOM file? (.dcm)
        set n [ string last ".dcm" $f ]
        if { $n > 0 } {
            #--- this is a dicom file
            set gotDICOM 1
            break
        }
    }
    if { $gotDICOM } {
        #--- add the file with complete pathname to the list of T2 Volumes
        set ::fips2mrml_T2Volume [ lappend ::fips2mrml_T2Volume $f ]
    } else {
        #--- add a null entry to the list of T1 Volumes
        set ::fips2mrml_T2Volume [ lappend ::fips2mrml_T2Volume "" ] 
    }

    if { $gotDICOM } {
        puts "T2 volumes found:  $::fips2mrml_T2Volume"
        fips2mrml_DisplayMessage "T2 volumes found:  $::fips2mrml_T2Volume"
    } else {
        puts "No T2 volumes found."
        fips2mrml_DisplayMessage "No T2 volumes found."
    }
    
    return $gotDICOM
}


proc fips2mrml_GetExampleFuncVolume { } {
    #--- search will have returned a .feat directory.
    #--- Navigate the standard directory tree one step down to find the
    #--- example functional image (one from the middle of the time series
    #--- that is used as the target for motion correction.

    puts "Getting example functional volumes..."
    fips2mrml_DisplayMessage "Getting example functional volumes..."
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set tmp [format "%s%s" $tmp "/example_func.nii" ]
    #--- does file exist?
    set found [ file exists $tmp ]
    set ::fips2mrml_ExampleFuncVolume [ lappend ::fips2mrml_ExampleFuncVolume $tmp ] 

    if { $found } {
        puts "Example functional volumes found:  $::fips2mrml_ExampleFuncVolume"
        fips2mrml_DisplayMessage "Example functional volumes found:  $::fips2mrml_ExampleFuncVolume"
    } else {
        puts "No example functional volume found."
        fips2mrml_DisplayMessage "No example functional volume found."        
    }
    return $found

}


proc fips2mrml_GetZstatVolumes { } {
    #--- search will have returned a .feat directory.
    #--- Navigate the standard directory tree down two steps into
    #--- the stats directory to find Zstat volumes
    #--- for now, just grab zstat1

    set at_least_one_found 0
    set ::fips2mrml_ZstatVolumes ""
    puts "Getting Zstat volumes..."
    fips2mrml_DisplayMessage "Getting Zstat volumes..."
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set tmp [format "%s%s" $tmp "/stats/" ]

    #--- now cd into that directory to find out how many zstat files there are.
    set cwd [ pwd ]
    set zz [cd $tmp ]
    set fileList [ glob zstat* -nocomplain]
    set len2 [ llength $fileList ]
    #--- append each of the zstat files to a list
    puts "Zstat volumes found:"
    fips2mrml_DisplayMessage "Zstat volumes found:"
    for { set j 0 } { $j < $len2 } { incr j } {
        set val [ lindex $fileList $j ]
        set val [ format "%s%s" $tmp $val]
        puts "$val"
        set ::fips2mrml_ZstatVolumes [ lappend ::fips2mrml_ZstatVolumes $val ]             
        #--- does file exist?
        set found [ file exists $val ]
        if { $found } {
            set at_least_one_found 1
        }
    }
    set zz [cd $cwd ]
    if { $at_least_one_found == 0 } {
        puts "No Tstat volumes found."
        fips2mrml_DisplayMessage "No Tstat volumes found."
    }
    return $at_least_one_found
    
}

proc fips2mrml_CreateUnnumberedZstatFiles { } {

    #--- for each statistics file with a number,
    #--- create an unnumbered version and
    #--- rebuild the list of ZstatVolumes to write to MRML
    #--- with new names
    #--- **Assume files are of the form char1.ext, char2.ext etc.
    #--- and start being numbered at 1**

    puts "Creating symbolic links..."
    set len [ llength $::fips2mrml_ZstatVolumes ]

    for { set i 0 } { $i < $len } { incr i } {
        set fulln [ lindex $::fips2mrml_ZstatVolumes $i ]
        #--- get directory.
        set dirn [ string trimright $fulln "/"]
        set j [ string last "/" $dirn ]
        if { $j > 0 } {
            set dirn [ string range $dirn 0 $j ]        
        }
        set filen [ string trimleft $fulln $dirn ]

        #--- get the extension
        set indx [ string last "." $filen ]
        set ext [ string range $filen $indx end ]

        #--- strip off the basename
        set basename [ string range $filen 0 [expr $indx - 1 ] ]

        #--- get the first integer in the basename
        set len2 [ string length $basename ]
        set isaNum 0
        for { set k 0 } { $k < $len2 } { incr k } {
            set c [ string index $basename $k ]
            if { [ string is integer -strict $c ] } {
                set isaNum 1 
                break;
            }
        }

        #--- get numberless basename
        set numberlessBasename [ string range $basename 0 [ expr $k -1] ]
        
        #--- pull out the number
        if  { $isaNum && [ file exists $fulln ] } {
            #--- update list and create symbolic link
            set fileNum [ string range $basename $k end ]
            set charNum [ lindex $::fips2mrml_CharArray $fileNum]
            set linkBaseName [ format "%s%s" "slicerZstat" $charNum ]
            set newFile [ format "%s%s%s%s" $dirn "slicerZstat" $charNum $ext ]

            #--- make symlink
            puts "creating symbolic link: $newFile"
            set fcopy [ file copy -force $fulln $newFile ]            
            #set mklink [ file link -symbolic $newFile $fulln ]

            #--- update zStatVolumes list and zStatVolumeNames
            set ::fips2mrml_ZstatVolumes [ lreplace $::fips2mrml_ZstatVolumes $i $i $newFile ]
            set ::fips2mrml_ZstatVolumeNames [ lappend ::fips2mrml_ZstatVolumeNames $linkBaseName]
        } else {
            #--- update zStatVolumes list to include a null string
            puts "Problem creating symlink to file $fulln; may have unexpected naming convention. Not including in MRML file."
            set ::fips2mrml_ZstatVolumes [ lreplace $::fips2mrml_ZstatVolumes $i $i "" ]
            set ::fips2mrml_ZstatVolumeNames [ lappend ::fips2mrml_ZstatVolumeNames $linkBaseName]
        }
    }
}


proc fips2mrml_GetTstatVolumes { } {
    #--- search will have returned a .feat directory.
    #--- Navigate the standard directory tree down two steps into
    #--- the stats directory to find Tstat volumes

    set at_least_one_found 0
    puts "Getting Tstat volumes..."
    fips2mrml_DisplayMessage "Getting Tstat volumes..."
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set tmp [format "%s%s" $tmp "/stats/" ]
    #--- now cd into that directory to find out how many zstat files there are.
    set cwd [ pwd ]
    set zz [cd $tmp ]
    set fileList [ glob tstat* -nocomplain]
    set len2 [ llength $fileList ]
    #--- append each of the tstat files to a list
    puts "Tstat volumes found:"
    fips2mrml_DisplayMessage "Tstat volumes found:"
    for { set j 0 } { $j < $len2 } { incr j } {
        set val [ lindex $fileList $j ]
        set val [ format "%s%s" $tmp $val]            
        puts "$val"
        set ::fips2mrml_TstatVolumes [ lappend ::fips2mrml_TstatVolumes $val ]             
        #--- does file exist?
        set found [ file exists $val ]
        if { $found } {
            set at_least_one_found 1
        }
    }
    set zz [cd $cwd ]

    if { $at_least_one_found == 0 } {
        puts "No Tstat volumes found."
        fips2mrml_DisplayMessage "No Tstat volumes found."
    }
    return $at_least_one_found
}



proc fips2mrml_GetAnat2ExfMatrix { } {
    puts "Getting anatomical to func registrations..."
    fips2mrml_DisplayMessage "Getting anatomical to functional registrations..."
    set cwd [ pwd ]
    #--- first get all the mat files

    set matrixFileList ""
    set val $::fips2mrml_MatchSelection
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set tmp [format "%s%s" $tmp "/reg/freesurfer/" ]
    set matrixFile [ format "%s%s" $tmp "anat2exf.register.dat" ]
    set matrixFile [ glob $matrixFile -nocomplain ]
    set found [ file exists $matrixFile ]
    set matrixFileList [ lappend matrixFileList $matrixFile ]

    if { $found } {
        puts "anat2exf matrix files found: $matrixFileList"
        fips2mrml_DisplayMessage "exf2anat matrix files found: $matrixFileList"

        #--- next, open each, and slurp out its values, and
        #--- build the list of lists of lists ::fsl2mrml_anat2exfList
        set len2 [llength $matrixFileList ]
        for { set j 0 } { $j < $len2 } { incr j } {
            set f [ lindex $matrixFileList $j ]
            set matrix ""

            if { $f != "" } {
                set fid [ open $f r ]
                #--- throw away first 4 lines
                if { ! [eof $fid] } {
                    gets $fid rowStr
                }
                if { ! [eof $fid] } {
                    gets $fid rowStr
                }
                if { ! [eof $fid] } {
                    gets $fid rowStr
                }
                if { ! [eof $fid] } {
                    gets $fid rowStr
                }
                #--- get four lines of the matrix file.
                if { ! [eof $fid] } {
                    set rowStr ""
                    gets $fid rowStr
                    if { $rowStr != "" } {
                        #--- append it to the list
                        set matrix [ concat $matrix $rowStr ]
                    }
                }
                if { ! [eof $fid] } {
                    set rowStr ""
                    gets $fid rowStr
                    if { $rowStr != "" } {
                        #--- append it to the list
                        set matrix [ concat $matrix $rowStr ]
                    }
                }
                if { ! [eof $fid] } {
                    set rowStr ""
                    gets $fid rowStr
                    if { $rowStr != "" } {
                        #--- append it to the list
                        set matrix [ concat $matrix $rowStr ]
                    }
                }
                if { ! [eof $fid] } {
                    set rowStr ""
                    gets $fid rowStr
                    if { $rowStr != "" } {
                        #--- append it to the list
                        set matrix [ concat $matrix $rowStr ]
                    }
                }
                close $fid
            }
            set ::fips2mrml_anat2exfList [ lappend ::fips2mrml_anat2exf_List $matrix ]
            puts "$::fips2mrml_anat2exfList"
        }
    } else {
        puts "No anat2exf matrix files found"
        fips2mrml_DisplayMessage "No anat2exf matrix files found"
    }
    return $found
}


proc fips2mrml_GetExf2AnatMatrix { } {

    #--- oy. what to put here.
    #--- build a list of rows of matrices.
    #--- when parsing them out later,
    #--- rely on the fact that four rows of four values
    #--- make up a single matrix.

    puts "Getting func to anatomical registrations..."
    fips2mrml_DisplayMessage "Getting func to anatomical registrations..."
    set cwd [ pwd ]
    #--- first get all the mat files

    set matrixFileList ""
    set val $::fips2mrml_MatchSelection 
    set tmp [lindex $val $::fips2mrml_XMLDir_index ]
    set tmp [format "%s%s" $tmp "/reg/freesurfer/" ]
    set matrixFile [ format "%s%s" $tmp "exf2anat.fsl.mat" ]
    set matrixFile [ glob $matrixFile -nocomplain ]
    set found [ file exists $matrixFile ]
    set matrixFileList [ lappend matrixFileList $matrixFile ]

    if { $found } {
        puts "exf2anat matrix files found: $matrixFileList"
        fips2mrml_DisplayMessage "exf2anat matrix files found: $matrixFileList"

        #--- next, open each, and slurp out its values, and
        #--- build the list of lists of lists ::fsl2mrml_exf2anatList
        set len2 [llength $matrixFileList ]
        for { set j 0 } { $j < $len2 } { incr j } {
            set f [ lindex $matrixFileList $j ]
            set matrix ""

            if { $f != "" } {
                set fid [ open $f r ]
                while { ! [eof $fid] } {
                    #--- get a line of the matrix file.
                    set rowStr ""
                    gets $fid rowStr
                    if { $rowStr != "" } {
                        #--- append it to the list
                        set matrix [ concat $matrix $rowStr ]
                    }
                }
                close $fid
            }
            set ::fips2mrml_exf2anatList [ lappend ::fips2mrml_exf2anat_List $matrix ]
        }
    } else {
        puts "No exf2anat matrix files found"
        fips2mrml_DisplayMessage "No exf2anat matrix files found"
    }

    return $found
}



proc fips2mrml_GetFreeSurferLabels { } {

    #--- get the freesurfer laparc+aseg labels
    set cwd [ pwd ]
    set labels [ format "%s%s%s" $::fips2mrml_FreeSurferSubjectPath "mri/" "aparc+aseg.mgz"]
    #--- check to see if file exists
    set found [ file exists $labels ]
    if  { $found } {
        puts "Found FreeSurfer file $labels"
        fips2mrml_DisplayMessage "Found FreeSurfer file $labels"
        set ::fips2mrml_FreeSurferLabels [ lappend ::fips2mrml_FreeSurferLabels $labels ]
    } else {
        puts "No aparc+aseg.mgz found"
        fips2mrml_DisplayMessage "No aparc+aseg.mgz found"
    }

    return $found
}


proc fips2mrml_GetFreeSurferBrainMGZ { } {

    #--- get the freesurfer brain.mgz file
    #--- return 1 if found, return 0 if not found
    set cwd [ pwd ]
    set brain [ format "%s%s%s" $::fips2mrml_FreeSurferSubjectPath "mri/" "brain.mgz"]
    #--- check to see if file exists
    set found [ file exists $brain ]
    if  { $found } {
        puts "Found FreeSurfer file $brain"
        fips2mrml_DisplayMessage "Found FreeSurfer file $brain"
        set ::fips2mrml_FreeSurferBrainMGZ [ lappend ::fips2mrml_FreeSurferBrainMGZ $brain ]
    } else {
        puts "No brain.mgz found"
        fips2mrml_DisplayMessage "No brain.mgz found"
    }

    return $found
}


proc fips2mrml_GetFreeSurferModels { } {

    #--- get the freesurfer pial surface for now
    #--- return 1 if found, return 0 if not found
    set cwd [ pwd ]
    set pial [ format "%s%s%s" $::fips2mrml_FreeSurferSubjectPath "surf/" "lh.pial"]
    #--- check to see if file exists
    set found [ file exists $pial ]
    if  { $found } {
        puts "Found Freesurfer $pial"
        fips2mrml_DisplayMessage "Found Freesurfer $pial"
        set ::fips2mrml_FreeSurferModels [ lappend ::fips2mrml_FreeSurferModels $pial ]
    } else {
        puts "No lh.pial found"
        fips2mrml_DisplayMessage "No lh.pial found"
    }

    return $found

}


proc fips2mrml_WriteMRML2File {  } {

    #--- open a new mrmlFile


    set cwd [ pwd ]
    set ::fips2mrml_mrmlFilename [format "%s/%s%s%s" $::env(SLICER_DATA) $::fips2mrml_MRMLFile "_MRML2" ".xml" ]
    puts "Writing $::fips2mrml_mrmlFilename"
    fips2mrml_DisplayMessage "Writing $::fips2mrml_mrmlFilename"
    set fid [ open $::fips2mrml_mrmlFilename "w" ]

    #--- put standard mrml hdr
    set hdr "<?xml version=\"1.0\" standalone='no'?>"
    puts $fid $hdr
    set hdr "<!DOCTYPE MRML SYSTEM \"mrml20.dtd\">"
    puts $fid $hdr

    #--- open MRML tag
    set hdr "<MRML>"
    puts $fid $hdr

    #--- volume counter model counter, xform counter
    set vcount 1
    set mcount 1
    set xcount 1

    #--- write brain.mgz
    set len [ llength $::fips2mrml_FreeSurferBrainMGZ ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferBrainMGZ $i ]
        if { $filename != "" } {
            set line "<Volume name='brain_mgz$i' filePattern='%s' filePrefix='$filename' fileType='MGH' fullPrefix='$filename'></Volume>"
            puts $fid $line
        }
        incr vcount
    }

    #--- write aparc+aseg
    set len [ llength $::fips2mrml_FreeSurferLabels]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferLabels $i ]
        if { $filename != "" } {
            set line "<Volume name='aparc+aseg_mgz$i' filePattern='%s' filePrefix='$filename' fileType='MGH' fullPrefix='$filename' interpolate='false' labelMap='true'></Volume>"
            puts $fid $line
            set line "<Color name='Unknown' labels='0' diffuseColor='0 0 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebral-Exterior' labels='1' diffuseColor='0.803922 0.243137 0.305882'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebral-White-Matter' labels='2' diffuseColor='0.960784 0.960784 0.960784'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebral-Cortex' labels='3' diffuseColor='0.803922 0.243137 0.305882'></Color>"
            puts $fid $line
            set line "<Color name='Left-Lateral-Ventricle' labels='4' diffuseColor='0.470588 0.0705882 0.52549'></Color>"
            puts $fid $line
            set line "<Color name='Left-Inf-Lat-Vent' labels='5' diffuseColor='0.768627 0.227451 0.980392'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebellum-Exterior' labels='6' diffuseColor='0 0.580392 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebellum-White-Matter' labels='7' diffuseColor='0.862745 0.972549 0.643137'></Color>"
            puts $fid $line
            set line "<Color name='Left-Cerebellum-Cortex' labels='8' diffuseColor='0.901961 0.580392 0.133333'></Color>"
            puts $fid $line
            set line "<Color name='Left-Thalamus' labels='9' diffuseColor='0 0.462745 0.054902'></Color>"
            puts $fid $line
            set line "<Color name='Left-Thalamus-Proper' labels='10' diffuseColor='0 0.462745 0.054902'></Color>"
            puts $fid $line
            set line "<Color name='Left-Caudate' labels='11' diffuseColor='0.478431 0.729412 0.862745'></Color>"
            puts $fid $line
            set line "<Color name='Left-Putamen' labels='12' diffuseColor='0.92549 0.0509804 0.690196'></Color>"
            puts $fid $line
            set line "<Color name='Left-Pallidum' labels='13' diffuseColor='0.0470588 0.188235 1'></Color>"
            puts $fid $line
            set line "<Color name='3rd-Ventricle' labels='14' diffuseColor='0.8 0.713725 0.556863'></Color>"
            puts $fid $line
            set line "<Color name='4th-Ventricle' labels='15' diffuseColor='0.164706 0.8 0.643137'></Color>"
            puts $fid $line
            set line "<Color name='Brain-Stem' labels='16' diffuseColor='0.466667 0.623529 0.690196'></Color>"
            puts $fid $line
            set line "<Color name='Left-Hippocampus' labels='17' diffuseColor='0.862745 0.847059 0.0784314'></Color>"
            puts $fid $line
            set line "<Color name='Left-Amygdala' labels='18' diffuseColor='0.403922 1 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-Insula' labels='19' diffuseColor='0.313725 0.768627 0.384314'></Color>"
            puts $fid $line
            set line "<Color name='Left-Operculum' labels='20' diffuseColor='0.235294 0.227451 0.823529'></Color>"
            puts $fid $line
            set line "<Color name='Line-1' labels='21' diffuseColor='0.235294 0.227451 0.823529'></Color>"
            puts $fid $line
            set line "<Color name='Line-2' labels='22' diffuseColor='0.235294 0.227451 0.823529'></Color>"
            puts $fid $line
            set line "<Color name='Line-3' labels='23' diffuseColor='0.235294 0.227451 0.823529'></Color>"
            puts $fid $line
            set line "<Color name='CSF' labels='24' diffuseColor='0.235294 0.235294 0.235294'></Color>"
            puts $fid $line
            set line "<Color name='Left-Lesion' labels='25' diffuseColor='1 0.647059 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Accumbens-area' labels='26' diffuseColor='1 0.647059 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Substancia-Nigra' labels='27' diffuseColor='0 1 0.498039'></Color>"
            puts $fid $line
            set line "<Color name='Left-VentralDC' labels='28' diffuseColor='0.647059 0.164706 0.164706'></Color>"
            puts $fid $line
            set line "<Color name='Left-undetermined' labels='29' diffuseColor='0.529412 0.807843 0.921569'></Color>"
            puts $fid $line
            set line "<Color name='Left-vessel' labels='30' diffuseColor='0.627451 0.12549 0.941176'></Color>"
            puts $fid $line
            set line "<Color name='Left-choroid-plexus' labels='31' diffuseColor='0 1 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-F3orb' labels='32' diffuseColor='0.392157 0.196078 0.392157'></Color>"
            puts $fid $line
            set line "<Color name='Left-lOg' labels='33' diffuseColor='0.529412 0.196078 0.290196'></Color>"
            puts $fid $line
            set line "<Color name='Left-aOg' labels='34' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Left-mOg' labels='35' diffuseColor='0.2 0.196078 0.529412'></Color>"
            puts $fid $line
            set line "<Color name='Left-pOg' labels='36' diffuseColor='0.290196 0.607843 0.235294'></Color>"
            puts $fid $line
            set line "<Color name='Left-Stellate' labels='37' diffuseColor='0.470588 0.243137 0.168627'></Color>"
            puts $fid $line
            set line "<Color name='Left-Porg' labels='38' diffuseColor='0.290196 0.607843 0.235294'></Color>"
            puts $fid $line
            set line "<Color name='Left-Aorg' labels='39' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebral-Exterior' labels='40' diffuseColor='0.803922 0.243137 0.305882'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebral-White-Matter' labels='41' diffuseColor='0 0.882353 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebral-Cortex' labels='42' diffuseColor='0.803922 0.243137 0.305882'></Color>"
            puts $fid $line
            set line "<Color name='Right-Lateral-Ventricle' labels='43' diffuseColor='0.470588 0.0705882 0.52549'></Color>"
            puts $fid $line
            set line "<Color name='Right-Inf-Lat-Vent' labels='44' diffuseColor='0.768627 0.227451 0.980392'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebellum-Exterior' labels='45' diffuseColor='0 0.580392 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebellum-White-Matter' labels='46' diffuseColor='0.862745 0.972549 0.643137'></Color>"
            puts $fid $line
            set line "<Color name='Right-Cerebellum-Cortex' labels='47' diffuseColor='0.901961 0.580392 0.133333'></Color>"
            puts $fid $line
            set line "<Color name='Right-Thalamus' labels='48' diffuseColor='0 0.462745 0.054902'></Color>"
            puts $fid $line
            set line "<Color name='Right-Thalamus-Proper' labels='49' diffuseColor='0 0.462745 0.054902'></Color>"
            puts $fid $line
            set line "<Color name='Right-Caudate' labels='50' diffuseColor='0.478431 0.729412 0.862745'></Color>"
            puts $fid $line
            set line "<Color name='Right-Putamen' labels='51' diffuseColor='0.92549 0.0509804 0.690196'></Color>"
            puts $fid $line
            set line "<Color name='Right-Pallidum' labels='52' diffuseColor='0.0509804 0.188235 1'></Color>"
            puts $fid $line
            set line "<Color name='Right-Hippocampus' labels='53' diffuseColor='0.862745 0.847059 0.0784314'></Color>"
            puts $fid $line
            set line "<Color name='Right-Amygdala' labels='54' diffuseColor='0.403922 1 1'></Color>"
            puts $fid $line
            set line "<Color name='Right-Insula' labels='55' diffuseColor='0.313725 0.768627 0.384314'></Color>"
            puts $fid $line
            set line "<Color name='Right-Operculum' labels='56' diffuseColor='0.235294 0.227451 0.823529'></Color>"
            puts $fid $line
            set line "<Color name='Right-Lesion' labels='57' diffuseColor='1 0.647059 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Accumbens-area' labels='58' diffuseColor='1 0.647059 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Substancia-Nigra' labels='59' diffuseColor='0 1 0.498039'></Color>"
            puts $fid $line
            set line "<Color name='Right-VentralDC' labels='60' diffuseColor='0.647059 0.164706 0.164706'></Color>"
            puts $fid $line
            set line "<Color name='Right-undetermined' labels='61' diffuseColor='0.529412 0.807843 0.921569'></Color>"
            puts $fid $line
            set line "<Color name='Right-vessel' labels='62' diffuseColor='0.627451 0.12549 0.941176'></Color>"
            puts $fid $line
            set line "<Color name='Right-choroid-plexus' labels='63' diffuseColor='0 1 1'></Color>"
            puts $fid $line
            set line "<Color name='Right-F3orb' labels='64' diffuseColor='0.392157 0.196078 0.392157'></Color>"
            puts $fid $line
            set line "<Color name='Right-lOg' labels='65' diffuseColor='0.529412 0.196078 0.290196'></Color>"
            puts $fid $line
            set line "<Color name='Right-aOg' labels='66' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Right-mOg' labels='67' diffuseColor='0.2 0.196078 0.529412'></Color>"
            puts $fid $line
            set line "<Color name='Right-pOg' labels='68' diffuseColor='0.290196 0.607843 0.235294'></Color>"
            puts $fid $line
            set line "<Color name='Right-Stellate' labels='69' diffuseColor='0.470588 0.243137 0.168627'></Color>"
            puts $fid $line
            set line "<Color name='Right-Porg' labels='70' diffuseColor='0.290196 0.607843 0.235294'></Color>"
            puts $fid $line
            set line "<Color name='Right-Aorg' labels='71' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='5th-Ventricle' labels='72' diffuseColor='0.470588 0.745098 0.588235'></Color>"
            puts $fid $line
            set line "<Color name='Left-Interior' labels='73' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Right-Interior' labels='74' diffuseColor='0.478431 0.529412 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Left-Lateral-Ventricles' labels='75' diffuseColor='0.470588 0.0705882 0.52549'></Color>"
            puts $fid $line
            set line "<Color name='Right-Lateral-Ventricles' labels='76' diffuseColor='0.470588 0.0705882 0.52549'></Color>"
            puts $fid $line
            set line "<Color name='WM-hypointensities' labels='77' diffuseColor='0.784314 0.27451 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-WM-hypointensities' labels='78' diffuseColor='0.486275 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-WM-hypointensities' labels='79' diffuseColor='0.486275 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='non-WM-hypointensities' labels='80' diffuseColor='0.643137 0.423529 0.886275'></Color>"
            puts $fid $line
            set line "<Color name='Left-non-WM-hypointensities' labels='81' diffuseColor='0.643137 0.423529 0.886275'></Color>"
            puts $fid $line
            set line "<Color name='Right-non-WM-hypointensities' labels='82' diffuseColor='0.643137 0.423529 0.886275'></Color>"
            puts $fid $line
            set line "<Color name='Left-F1' labels='83' diffuseColor='1 0.854902 0.72549'></Color>"
            puts $fid $line
            set line "<Color name='Right-F1' labels='84' diffuseColor='1 0.854902 0.72549'></Color>"
            puts $fid $line
            set line "<Color name='Optic-Chiasm' labels='85' diffuseColor='0.917647 0.662745 0.117647'></Color>"
            puts $fid $line
            set line "<Color name='Corpus_Callosum' labels='86' diffuseColor='0.980392 1 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Left-Amygdala-Anterior' labels='96' diffuseColor='0.803922 0.0392157 0.490196'></Color>"
            puts $fid $line
            set line "<Color name='Right-Amygdala-Anterior' labels='97' diffuseColor='0.803922 0.0392157 0.490196'></Color>"
            puts $fid $line
            set line "<Color name='Dura' labels='98' diffuseColor='0.627451 0.12549 0.941176'></Color>"
            puts $fid $line
            set line "<Color name='Left-wm-intensity-abnormality' labels='100' diffuseColor='0.486275 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-caudate-intensity-abnormality' labels='101' diffuseColor='0.490196 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-putamen-intensity-abnormality' labels='102' diffuseColor='0.494118 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-accumbens-intensity-abnormality' labels='103' diffuseColor='0.498039 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-pallidum-intensity-abnormality' labels='104' diffuseColor='0.486275 0.552941 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-amygdala-intensity-abnormality' labels='105' diffuseColor='0.486275 0.556863 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-hippocampus-intensity-abnormality' labels='106' diffuseColor='0.486275 0.560784 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-thalamus-intensity-abnormality' labels='107' diffuseColor='0.486275 0.564706 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Left-VDC-intensity-abnormality' labels='108' diffuseColor='0.486275 0.54902 0.701961'></Color>"
            puts $fid $line
            set line "<Color name='Right-wm-intensity-abnormality' labels='109' diffuseColor='0.486275 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-caudate-intensity-abnormality' labels='110' diffuseColor='0.490196 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-putamen-intensity-abnormality' labels='111' diffuseColor='0.494118 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-accumbens-intensity-abnormality' labels='112' diffuseColor='0.498039 0.54902 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-pallidum-intensity-abnormality' labels='113' diffuseColor='0.486275 0.552941 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-amygdala-intensity-abnormality' labels='114' diffuseColor='0.486275 0.556863 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-hippocampus-intensity-abnormality' labels='115' diffuseColor='0.486275 0.560784 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-thalamus-intensity-abnormality' labels='116' diffuseColor='0.486275 0.564706 0.698039'></Color>"
            puts $fid $line
            set line "<Color name='Right-VDC-intensity-abnormality' labels='117' diffuseColor='0.486275 0.54902 0.701961'></Color>"
            puts $fid $line
            set line "<Color name='Epidermis' labels='118' diffuseColor='1 0.0784314 0.576471'></Color>"
            puts $fid $line
            set line "<Color name='Conn-Tissue' labels='119' diffuseColor='0.803922 0.701961 0.545098'></Color>"
            puts $fid $line
            set line "<Color name='SC-Fat/Muscle' labels='120' diffuseColor='0.933333 0.933333 0.819608'></Color>"
            puts $fid $line
            set line "<Color name='Cranium' labels='121' diffuseColor='0.784314 0.784314 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='CSF-SA' labels='122' diffuseColor='0.290196 1 0.290196'></Color>"
            puts $fid $line
            set line "<Color name='Muscle' labels='123' diffuseColor='0.933333 0 0'></Color>"
            puts $fid $line
            set line "<Color name='Ear' labels='124' diffuseColor='0 0 0.545098'></Color>"
            puts $fid $line
            set line "<Color name='Adipose' labels='125' diffuseColor='0.678431 1 0.184314'></Color>"
            puts $fid $line
            set line "<Color name='Spinal-Cord' labels='126' diffuseColor='0.521569 0.796078 0.898039'></Color>"
            puts $fid $line
            set line "<Color name='Soft-Tissue' labels='127' diffuseColor='0.101961 0.929412 0.223529'></Color>"
            puts $fid $line
            set line "<Color name='Nerve' labels='128' diffuseColor='0.133333 0.545098 0.133333'></Color>"
            puts $fid $line
            set line "<Color name='Bone' labels='129' diffuseColor='0.117647 0.564706 1'></Color>"
            puts $fid $line
            set line "<Color name='Air' labels='130' diffuseColor='0.576471 0.0745098 0.678431'></Color>"
            puts $fid $line
            set line "<Color name='Orbital-Fat' labels='131' diffuseColor='0.933333 0.231373 0.231373'></Color>"
            puts $fid $line
            set line "<Color name='Tongue' labels='132' diffuseColor='0.866667 0.152941 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Nasal-Structures' labels='133' diffuseColor='0.933333 0.682353 0.933333'></Color>"
            puts $fid $line
            set line "<Color name='Globe' labels='134' diffuseColor='1 0.00392157 0.00392157'></Color>"
            puts $fid $line
            set line "<Color name='Teeth' labels='135' diffuseColor='0.282353 0.239216 0.545098'></Color>"
            puts $fid $line
            set line "<Color name='Left-Caudate/Putamen' labels='136' diffuseColor='0.0823529 0.152941 0.517647'></Color>"
            puts $fid $line
            set line "<Color name='Right-Caudate/Putamen' labels='137' diffuseColor='0.0823529 0.152941 0.517647'></Color>"
            puts $fid $line
            set line "<Color name='Left-Claustrum' labels='138' diffuseColor='0.254902 0.529412 0.0784314'></Color>"
            puts $fid $line
            set line "<Color name='Right-Claustrum' labels='139' diffuseColor='0.254902 0.529412 0.0784314'></Color>"
            puts $fid $line
            set line "<Color name='Cornea' labels='140' diffuseColor='0.52549 0.0156863 0.627451'></Color>"
            puts $fid $line
            set line "<Color name='Diploe' labels='142' diffuseColor='0.866667 0.886275 0.266667'></Color>"
            puts $fid $line
            set line "<Color name='Vitreous-Humor' labels='143' diffuseColor='1 1 0.996078'></Color>"
            puts $fid $line
            set line "<Color name='Lens' labels='144' diffuseColor='0.203922 0.819608 0.886275'></Color>"
            puts $fid $line
            set line "<Color name='Aqueous-Humor' labels='145' diffuseColor='0.937255 0.627451 0.87451'></Color>"
            puts $fid $line
            set line "<Color name='Outer-Table' labels='146' diffuseColor='0.27451 0.509804 0.705882'></Color>"
            puts $fid $line
            set line "<Color name='Inner-Table' labels='147' diffuseColor='0.27451 0.509804 0.709804'></Color>"
            puts $fid $line
            set line "<Color name='Periosteum' labels='148' diffuseColor='0.545098 0.47451 0.368627'></Color>"
            puts $fid $line
            set line "<Color name='Endosteum' labels='149' diffuseColor='0.878431 0.878431 0.878431'></Color>"
            puts $fid $line
            set line "<Color name='R/C/S' labels='150' diffuseColor='1 0.00784314 0.00784314'></Color>"
            puts $fid $line
            set line "<Color name='Iris' labels='151' diffuseColor='0.803922 0.803922 0'></Color>"
            puts $fid $line
            set line "<Color name='SC-Adipose/Muscle' labels='152' diffuseColor='0.933333 0.933333 0.819608'></Color>"
            puts $fid $line
            set line "<Color name='SC-Tissue' labels='153' diffuseColor='0.545098 0.47451 0.368627'></Color>"
            puts $fid $line
            set line "<Color name='Orbital-Adipose' labels='154' diffuseColor='0.933333 0.231373 0.231373'></Color>"
            puts $fid $line
            set line "<Color name='alveus' labels='201' diffuseColor='1 0.8 0.6'></Color>"
            puts $fid $line
            set line "<Color name='perforant_pathway' labels='202' diffuseColor='1 0.501961 0.501961'></Color>"
            puts $fid $line
            set line "<Color name='parasubiculum' labels='203' diffuseColor='1 1 0'></Color>"
            puts $fid $line
            set line "<Color name='presubiculum' labels='204' diffuseColor='0.25098 0 0.25098'></Color>"
            puts $fid $line
            set line "<Color name='subiculum' labels='205' diffuseColor='0 0 1'></Color>"
            puts $fid $line
            set line "<Color name='CA1' labels='206' diffuseColor='1 0 0'></Color>"
            puts $fid $line
            set line "<Color name='CA2' labels='207' diffuseColor='0.501961 0.501961 1'></Color>"
            puts $fid $line
            set line "<Color name='CA3' labels='208' diffuseColor='0 0.501961 0'></Color>"
            puts $fid $line
            set line "<Color name='CA4' labels='209' diffuseColor='0.768627 0.627451 0.501961'></Color>"
            puts $fid $line
            set line "<Color name='GC-DG' labels='210' diffuseColor='1 0.501961 1'></Color>"
            puts $fid $line
            set line "<Color name='HATA' labels='211' diffuseColor='0.501961 1 0.501961'></Color>"
            puts $fid $line
            set line "<Color name='fimbria' labels='212' diffuseColor='0.8 0.6 0.8'></Color>"
            puts $fid $line
            set line "<Color name='lateral_ventricle' labels='213' diffuseColor='1 0 1'></Color>"
            puts $fid $line
            set line "<Color name='molecular_layer_HP' labels='214' diffuseColor='0.501961 0 0'></Color>"
            puts $fid $line
            set line "<Color name='hippocampal_fissure' labels='215' diffuseColor='0.25098 1 0.768627'></Color>"
            puts $fid $line
            set line "<Color name='entorhinal_cortex' labels='216' diffuseColor='1 0.8 0.4'></Color>"
            puts $fid $line
            set line "<Color name='molecular_layer_subiculum' labels='217' diffuseColor='0.501961 0.501961 0.501961'></Color>"
            puts $fid $line
            set line "<Color name='Amygdala' labels='218' diffuseColor='0.407843 1 1'></Color>"
            puts $fid $line
            set line "<Color name='Cerebral_White_Matter' labels='219' diffuseColor='0 0.886275 0'></Color>"
            puts $fid $line
            set line "<Color name='Cerebral_Cortex' labels='220' diffuseColor='0.803922 0.247059 0.305882'></Color>"
            puts $fid $line
            set line "<Color name='Inf_Lat_Vent' labels='221' diffuseColor='0.772549 0.227451 0.980392'></Color>"
            puts $fid $line
            set line "<Color name='Aorta' labels='331' diffuseColor='1 0 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Common-IliacA' labels='332' diffuseColor='1 0.313725 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Common-IliacA' labels='333' diffuseColor='1 0.627451 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-External-IliacA' labels='334' diffuseColor='1 1 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-External-IliacA' labels='335' diffuseColor='0 1 0'></Color>"
            puts $fid $line
            set line "<Color name='Left-Internal-IliacA' labels='336' diffuseColor='1 0 0.627451'></Color>"
            puts $fid $line
            set line "<Color name='Right-Internal-IliacA' labels='337' diffuseColor='1 0 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-Lateral-SacralA' labels='338' diffuseColor='1 0.196078 0.313725'></Color>"
            puts $fid $line
            set line "<Color name='Right-Lateral-SacralA' labels='339' diffuseColor='0.313725 1 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Left-ObturatorA' labels='340' diffuseColor='0.627451 1 0.196078'></Color>"
            puts $fid $line
            set line "<Color name='Right-ObturatorA' labels='341' diffuseColor='0.627451 0.784314 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-Internal-PudendalA' labels='342' diffuseColor='0 1 0.627451'></Color>"
            puts $fid $line
            set line "<Color name='Right-Internal-PudendalA' labels='343' diffuseColor='0 0 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-UmbilicalA' labels='344' diffuseColor='0.313725 0.196078 1'></Color>"
            puts $fid $line
            set line "<Color name='Right-UmbilicalA' labels='345' diffuseColor='0.627451 0 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-Inf-RectalA' labels='346' diffuseColor='1 0.823529 0'></Color>"
            puts $fid $line
            set line "<Color name='Right-Inf-RectalA' labels='347' diffuseColor='0 0.627451 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-Common-IliacV' labels='348' diffuseColor='1 0.784314 0.313725'></Color>"
            puts $fid $line
            set line "<Color name='Right-Common-IliacV' labels='349' diffuseColor='1 0.784314 0.627451'></Color>"
            puts $fid $line
            set line "<Color name='Left-External-IliacV' labels='350' diffuseColor='1 0.313725 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Right-External-IliacV' labels='351' diffuseColor='1 0.627451 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Left-Internal-IliacV' labels='352' diffuseColor='0.117647 1 0.313725'></Color>"
            puts $fid $line
            set line "<Color name='Right-Internal-IliacV' labels='353' diffuseColor='0.313725 0.784314 1'></Color>"
            puts $fid $line
            set line "<Color name='Left-ObturatorV' labels='354' diffuseColor='0.313725 1 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Right-ObturatorV' labels='355' diffuseColor='0.764706 1 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Left-Internal-PudendalV' labels='356' diffuseColor='0.470588 0.784314 0.0784314'></Color>"
            puts $fid $line
            set line "<Color name='Right-Internal-PudendalV' labels='357' diffuseColor='0.666667 0.0392157 0.784314'></Color>"
            puts $fid $line
            set line "<Color name='Pos-Lymph' labels='358' diffuseColor='0.0784314 0.509804 0.705882'></Color>"
            puts $fid $line
            set line "<Color name='Neg-Lymph' labels='359' diffuseColor='0.0784314 0.705882 0.509804'></Color>"
            puts $fid $line
        }
        incr vcount
    }


    #--- write pial surface
    set len [ llength $::fips2mrml_FreeSurferModels ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferModels $i ]
        if { $filename != "" } {
            set line "<Model id='M$mcount' name='lh-pial$i' fileName='$filename' color='Left-Cerebral-Cortex'></Model>"
            puts $fid $line
        }
        incr mcount
    }
    
    
    #--- write matrices
    set mcount 0
    set row1 "1 0 0 0"
    set row2 "0 0 -1 0"
    set row3 "0 -1 0 0"
    set row4 "0 0 0 1"
    set vals [ concat $row1 $row2 $row3 $row4 ]
    set line "<Transform name='transform$mcount'>"
    puts $fid $line
    set line "<Matrix name='RLrotation$xcount' matrix='$vals'></Matrix>"
    puts $fid $line
    incr mcount    

    set mat [ lindex $::fips2mrml_exf2anatList 0 ]
    if { $mat != ""  } {
        #--- strip out translations
        set mat [ lreplace $mat 3 3 "0.0"]
        puts "mat= $mat"
        set mat [ lreplace $mat 7 7 "0.0"]
        puts "mat= $mat"
        set mat [ lreplace $mat 11 11 "0.0"]
        puts "mat= $mat"
        set line "<Transform name='transform$mcount'>"
        puts $fid $line
        set line "<Matrix id='M$mcount' name='exf2anat' matrix='$mat'></Matrix>"
        puts $fid $line
        incr mcount
    }
    
    #--- write example functional volume
    set len [ llength $::fips2mrml_ExampleFuncVolume ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_ExampleFuncVolume $i ]
        if { $filename != "" } {
            set line "<Volume name='example_func$i' filePattern='%s' filePrefix='$filename' fileType='Generic'></Volume>"
            puts $fid $line
        }
        incr vcount
    }
    
    #--- write statistics volumes
    set len [ llength $::fips2mrml_ZstatVolumes ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_ZstatVolumes $i ]
        if { $filename != "" } {
            set txtname [ lindex $::fips2mrml_ZstatVolumeNames $i ]
            set line "<Volume name='$txtname' filePattern='%s' filePrefix='$filename' fileType='Generic'></Volume>"
            puts $fid $line
        }
        incr vcount
    }


    set line "</Transform>"
    puts $fid $line
    set line "</Transform>"
    puts $fid $line
        
    #--- close file
    set line "</MRML>"
    puts $fid $line
    close $fid
    fips2mrml_DisplayMessage "Wrote $::fips2mrml_mrmlFilename"
}






proc fips2mrml_WriteMRML3File { } {

    #--- open a new mrmlFile
    set cwd [ pwd ]
    set ::fips2mrml_mrmlFilename [format "%s/%s%s%s" $::env(SLICER_DATA) $::fips2mrml_MRMLFile "_MRML3" ".mrml" ]
    puts "Writing $::fips2mrml_mrmlFilename"
    fips2mrml_DisplayMessage "Writing $::fips2mrml_mrmlFilename"
    set fid [ open $::fips2mrml_mrmlFilename "w+" ]

    #--- open MRML tag
    set hdr "<MRML>"
    puts $fid $hdr

    #--- set the active volume to be the brain.mgz (in the background) and active label to be the aparc+aseg.
    #--- fix parameters to refer to actual nodes created below
    set line "<Selection id=\"vtkMRMLSelectionNode1\" name=\"\"  hideFromEditors=\"true\" activeVolumeID=\"vtkMRMLScalarVolumeNode1\" activeLabelVolumeID=\"vtkMRMLScalarVolumeNode2\" ></Selection>"
    puts $fid $line
    puts $fid ""
    
    #--- set up the three main slice and slice composite nodes
    #--- fix parameters to refer to actual nodes created below
    #--- we put brain.mgz in the background layer, and aparc+aseg.mgz in the label layer. fg is null for now.
    set line "<Slice id=\"vtkMRMLSliceNode1\" name=\"\"  hideFromEditors=\"true\" layoutName=\"Red\" orientation=\"Axial\" ></Slice>"
    puts $fid $line
    set line "<SliceComposite id=\"vtkMRMLSliceCompositeNode1\" name=\"\"  hideFromEditors=\"true\" backgroundVolumeID=\"vtkMRMLScalarVolumeNode1\" foregroundVolumeID=\"\" labelVolumeID=\"vtkMRMLScalarVolumeNode2\" labelOpacity=\"1\" layoutName=\"Red\" ></SliceComposite>"
    puts $fid $line
    puts $fid ""

    set line "<Slice id=\"vtkMRMLSliceNode2\" name=\"\"  hideFromEditors=\"true\" layoutName=\"Yellow\" orientation=\"Sagittal\" ></Slice>"
    puts $fid $line
    set line "<SliceComposite id=\"vtkMRMLSliceCompositeNode2\" name=\"\"  hideFromEditors=\"true\" backgroundVolumeID=\"vtkMRMLScalarVolumeNode1\" foregroundVolumeID=\"\" labelVolumeID=\"vtkMRMLScalarVolumeNode2\" labelOpacity=\"1\" layoutName=\"Yellow\" ></SliceComposite>"
    puts $fid $line
    puts $fid ""
    
    set line "<Slice id=\"vtkMRMLSliceNode3\" name=\"\"  hideFromEditors=\"true\" layoutName=\"Green\" orientation=\"Coronal\" ></Slice>"
    puts $fid $line
    set line "<SliceComposite id=\"vtkMRMLSliceCompositeNode3\" name=\"\"  hideFromEditors=\"true\" backgroundVolumeID=\"vtkMRMLScalarVolumeNode1\" foregroundVolumeID=\"\" labelVolumeID=\"vtkMRMLScalarVolumeNode2\" labelOpacity=\"1\" layoutName=\"Green\" ></SliceComposite>"
    puts $fid $line
    puts $fid ""

    #--- set volume counters and model counters and xform counters to 1
    set mcount 1
    set vcount 1
    set xcount 1
    
    #--- write pial surface model
    set len [ llength $::fips2mrml_FreeSurferModels ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferModels $i ]
        #--- create model storage node
        set line "<ModelStorage id=\"FIPS2MRMLModelStorageNode$mcount\" name=\"\" hideFromEditors=\"true\" fileName=\"$filename\" > </ModelStorage>"
        puts $fid $line

        #--- create model display node
        set line "<ModelDisplay  id=\"FIPS2MRMLModelDisplayNode$mcount\" name=\"\"  hideFromEditors=\"true\"  color=\"0.5 0.5 0.5\" ambient=\"0\" diffuse=\"1\" specular=\"0\" power=\"1\" opacity=\"1\" visibility=\"true\" clipping=\"false\" backfaceCulling=\"true\" scalarVisibility=\"false\" vectorVisibility=\"false\" tensorVisibility=\"false\" scalarRange=\"0 100\"></ModelDisplay>"
        puts $fid $line
        
        #--- create model node
        set line "<Model id=\"FIPS2MRMLModelNode$mcount\" name=\"lh.pial\"  hideFromEditors=\"false\" storageNodeRef=\"FIPS2MRMLModelStorageNode$mcount\" displayNodeRef=\"FIPS2MRMLModelDisplayNode$mcount\" ></Model>"
        puts $fid $line

        incr mcount
    }
    puts $fid ""


    #--- write Brain MGZ
    set len [ llength $::fips2mrml_FreeSurferBrainMGZ ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferBrainMGZ $i ]
        #--- create volume archetype storage
        set line "<VolumeArchetypeStorage id=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" name=\"\"  hideFromEditors=\"true\" fileName=\"$filename\" centerImage=\"1\" ></VolumeArchetypeStorage>"
        puts $fid $line

        #--- create volume display node
        set line "<VolumeDisplay id=\"vtkMRMLVolumeDisplayNode$vcount\" name=\"\"  hideFromEditors=\"true\" interpolate=\"1\" autoWindowLevel=\"1\" applyThreshold=\"0\" autoThreshold=\"0\" ></VolumeDisplay>"
        puts $fid $line
        
        #--- create scalar volume node
        set line "<Volume id=\"vtkMRMLScalarVolumeNode$vcount\" name=\"brain.mgz\"  hideFromEditors=\"false\" storageNodeRef=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" displayNodeRef=\"vtkMRMLVolumeDisplayNode$vcount\" labelMap=\"0\" ></Volume>"
        puts $fid $line

        incr vcount
    }
    puts $fid ""
    
    
    #--- write color information for aparc+aseg file
    set line "<Color id=\"vtkMRMLFreeSurferColorNode1\" name=\"File\" hideFromEditors=\"true\" type=\"12\" filename=\"$::fips2mrml_FScolors\"> </Color> "
    puts $fid $line
    puts $fid ""


    #--- write aparc+aseg label file
    set len [ llength $::fips2mrml_FreeSurferLabels ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_FreeSurferLabels $i ]
        #--- create volume archetype storage
        set line "<VolumeArchetypeStorage id=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" name=\"\"  hideFromEditors=\"true\" fileName=\"$filename\" centerImage=\"1\" ></VolumeArchetypeStorage>"
        puts $fid $line

        #--- create volume display node
        set line "<VolumeDisplay id=\"vtkMRMLVolumeDisplayNode$vcount\" name=\"\"  hideFromEditors=\"true\" interpolate=\"1\" autoWindowLevel=\"1\" applyThreshold=\"0\" autoThreshold=\"0\" colorNodeRef=\"vtkMRMLFreeSurferColorNode1\"></VolumeDisplay>"
        puts $fid $line
        
        #--- create scalar volume node
        set line "<Volume id=\"vtkMRMLScalarVolumeNode$vcount\" name=\"aparc+aseg.mgz\"  hideFromEditors=\"false\" storageNodeRef=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" displayNodeRef=\"vtkMRMLVolumeDisplayNode$vcount\" labelMap=\"1\" ></Volume>"
        puts $fid $line

        incr vcount
    }
    puts $fid ""


    set xcount 1
    #--- write nested transformation matrices;
    #--- this one takes care of rotation around R/L
    #--- to partially convert freesurfer RAS to Slicer RAS:
    set row1 "1 0 0 0"
    set row2 "0 0 -1 0"
    set row3 "0 -1 0 0"
    set row4 "0 0 0 1"
    set vals [ concat $row1 $row2 $row3 $row4 ]
    set line "<LinearTransform id=\"vtkMRMLLinearTransformNode$xcount\" name=\"RLrotation$xcount\" hideFromEditors=\"false\" matrixTransformToParent=\"$vals\"></LinearTransform>"
    puts $fid $line
    puts $fid ""
    incr xcount
    set rxcount 1

    #--- and this is the FreeSurfer exf2anat.fsl.mat with
    #--- translation stripped off for now
    set mat [ lindex $::fips2mrml_exf2anatList 0 ]
    if { $mat != ""  } {
        #--- strip out translations
        set mat [ lreplace $mat 3 3 "0.0"]
        set mat [ lreplace $mat 7 7 "0.0"]
        set mat [ lreplace $mat 11 11 "0.0"]
        set line "<LinearTransform id=\"vtkMRMLLinearTransformNode$xcount\" name=\"exf2anat$xcount\" hideFromEditors=\"false\" transformNodeRef=\"vtkMRMLLinearTransformNode$rxcount\" matrixTransformToParent=\"$mat\"></LinearTransform>"
        puts $fid $line
    puts $fid ""
    }



    #--- write example functional volume
    set len [ llength $::fips2mrml_ExampleFuncVolume ]
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_ExampleFuncVolume $i ]
        #--- create volume archetype storage
        set line "<VolumeArchetypeStorage id=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" name=\"\"  hideFromEditors=\"true\" fileName=\"$filename\" centerImage=\"1\" ></VolumeArchetypeStorage>"
        puts $fid $line

        #--- create volume display node
        set line "<VolumeDisplay id=\"vtkMRMLVolumeDisplayNode$vcount\" name=\"\"  hideFromEditors=\"true\" interpolate=\"1\" autoWindowLevel=\"1\" applyThreshold=\"0\" autoThreshold=\"0\" ></VolumeDisplay>"
        puts $fid $line
        
        #--- create scalar volume node
        set line "<Volume id=\"vtkMRMLScalarVolumeNode$vcount\" name=\"example_functional\"  hideFromEditors=\"false\" storageNodeRef=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" displayNodeRef=\"vtkMRMLVolumeDisplayNode$vcount\" transformNodeRef=\"vtkMRMLLinearTransformNode$xcount\" labelMap=\"0\" ></Volume>"
        puts $fid $line
        incr vcount
    }
    puts $fid ""

    
    
    #--- write statistics volumes
    set len [ llength $::fips2mrml_ZstatVolumes ]
    #--- for now, just write zstat1
    #set len 1
    for { set i 0 } { $i < $len } { incr i } {
        set filename [ lindex $::fips2mrml_ZstatVolumes $i ]
        set txtname [ lindex $::fips2mrml_ZstatVolumeNames $i ]
        #--- create volume archetype storage
        set line "<VolumeArchetypeStorage id=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" name=\"\"  hideFromEditors=\"true\" fileName=\"$filename\" centerImage=\"1\" ></VolumeArchetypeStorage>"
        puts $fid $line

        #--- create volume display node
        set line "<VolumeDisplay id=\"vtkMRMLVolumeDisplayNode$vcount\" name=\"\"  hideFromEditors=\"true\" interpolate=\"1\" autoWindowLevel=\"1\" applyThreshold=\"0\" autoThreshold=\"0\" colorNodeRef=\"vtkMRMLColorNodefMRIPA\"></VolumeDisplay>"
        puts $fid $line
        
        #--- create scalar volume node
        set line "<Volume id=\"vtkMRMLScalarVolumeNode$vcount\" name=\"$txtname\"  hideFromEditors=\"false\" storageNodeRef=\"vtkMRMLVolumeArchetypeStorageNode$vcount\" displayNodeRef=\"vtkMRMLVolumeDisplayNode$vcount\" transformNodeRef=\"vtkMRMLLinearTransformNode$xcount\" labelMap=\"0\" ></Volume>"
        puts $fid $line

        incr vcount
    }
    puts $fid ""
    
    
    #--- close file
    set line "</MRML>"
    puts $fid $line
    close $fid
    fips2mrml_DisplayMessage "Wrote $::fips2mrml_mrmlFilename"
}



proc fips2mrml_SearchAndReport { } {

    $::fips2mrml_Message  config -text ""
    fips2mrml_InitializeGlobals
    fips2mrml_GetSearchInput
    fips2mrml_GetMatchingXMLFiles
    fips2mrml_PopulateList 
}




proc fips2mrml_PopulateList { } {

    #--- clear listbox
    $::fips2mrml_Listbox delete 0 end

    #--- repopulate list box
    if { $::fips2mrml_SearchResults != "" } {
        set len [ llength $::fips2mrml_SearchResults ]
        #--- add a listbox entry for each match found
        for { set i 0 } { $i < $len } {incr i} {
            set val [ lindex $::fips2mrml_SearchResults $i ]
            $::fips2mrml_Listbox  insert end $val
        }
    }
    
}


proc fips2mrml_DisplayMessage { str } {

    set w $::fips2mrml_Message
    $w configure -text $str

}



proc fips2mrml_LoadSelection { } {
#--- load the queriable data into 3D Slicer

    fips2mrml_DisplayMessage "humpa dumpa"
}




proc fips2mrml_ConvertSelection  { } {

    set sel [ $::fips2mrml_Listbox curselection ]
    if { $sel != "" } {
        set ::fips2mrml_MatchSelection [ $::fips2mrml_Listbox get $sel ]
        #--- set BIRNid for later mapping to FreeSurfer ID
        set ::fips2mrml_BIRNId [ lindex $::fips2mrml_MatchSelection $::fips2mrml_BIRNId_index  ]

        if { [ fips2rmrml_GetFreeSurferID ] } {
            puts "starting.............................................."
            puts "Assembling scene for: $::fips2mrml_MatchSelection"
            puts ".........................................................."
            fips2mrml_GetExampleFuncVolume
            puts ".........................................................."
            fips2mrml_GetZstatVolumes
            puts ".........................................................."
            fips2mrml_CreateUnnumberedZstatFiles
            puts ".........................................................."
            #--- if FreeSurfer analysis exists, get brain, labels, surf
            if { $::fips2mrml_FreeSurferId != "" } {
                set got [ fips2mrml_GetExf2AnatMatrix ]
                puts ".........................................................."
                set got [ fips2mrml_GetAnat2ExfMatrix ]
                puts ".........................................................."                
                set got [ fips2mrml_GetFreeSurferBrainMGZ ]
                puts ".........................................................."
                set got [ fips2mrml_GetFreeSurferLabels ]
                puts ".........................................................."
                set got [ fips2mrml_GetFreeSurferModels ]
            }
            puts ".........................................................."
            fips2mrml_WriteMRML3File 
            fips2mrml_WriteMRML2File 
            puts "... done"
            $::fips2mrml_Message config -text "MRML files written sucessfully."
            
        } else {
            tk_messageBox -message "No corresponding FreeSurfer ID was found; no files written."
            puts "No corresponding FreeSurfer ID was found; no MRML file written.."
        }
    } elseif { [$::fips2mrml_Listbox get 0 ] == "" } {
        tk_messageBox -message "No matches are listed; nothing to write."        
        puts "Nothing selected; no MRML file written."
        fips2mrml_DisplayMessage "Nothing selected; no MRML file written."
    } else {
        tk_messageBox -message "Please select an entry from the list of matches."        
        puts "Nothing selected; no MRML file written."
        fips2mrml_DisplayMessage "Nothing selected; no MRML file written."
    }
}




proc QueryAtlasDeleteFips2MrmlToplevel { w } {

    destroy $w
    if { [ info exists ::fips2mrml_Listbox ] } {
        unset -nocomplain ::fips2mrml_Listbox
    }
    if { [ info exists ::fips2mrml_Message ] } {
        unset -nocomplain ::fips2mrml_Message
    }
}




proc QueryAtlasLaunchFips2Mrml { } {

    #--- set some stuff
    set ::fips2mrml_FIPSwin "fips2mrml"

    #--- create toplevel GUI (raise if already created.)
    set w .$::fips2mrml_FIPSwin
    puts "window is $w"
    
    if { [winfo exists $w ] } {
        puts "raising win"
        wm deiconify $w
        raise $w
        return
    }

    #--- check to see if some environment variables are set:
    if { [ info exists ::env(FIPSHOME) ] } {
        set fipsDir $::env(FIPSHOME)
    } else {
        tk_messageBox -message "Please set your (FIPS) FIPSHOME environment variable and try again."
        fips2mrml_DisplayMessage "Please set your (FIPS) FIPSHOME environment variable and try again."
        exit
    }
    if { [info exists ::env(SUBJECTS_DIR) ] } {
        set fsSubDir $::env(SUBJECTS_DIR)
    } else {
        tk_messageBox -message "Please set your (FreeSurfer) SUBJECTS_DIR environment variable and try again."
        fips2mrml_DisplayMessage "Please set your (FreeSurfer) SUBJECTS_DIR environment variable and try again."
        exit
    }
    if { [ info exists ::env(SLICER_DATA) ] } {
        set dataDir $::env(SLICER_DATA)
    } else {
        tk_messageBox -message "Please set your (Slicer data directory) SLICER_DATA environment variable and try again."
        fips2mrml_DisplayMessage "Please set your (Slicer data directory) SLICER_DATA environment variable and try again."
        exit
    }
    if { [ info exists ::env(SLICER_HOME) ] } {
        set slicer3Dir $::env(SLICER_HOME)
    } else {
        tk_messageBox -message "Please set your (Slicer3 home directory) SLICER_HOME environment variable and try again."
        fips2mrml_DisplayMessage "Please set your (Slicer3 home directory) SLICER_HOME environment variable and try again."
        exit
    }
    
    #--- building some filenames from environment variables containing paths
    set ::fips_database_dat [ format "%s%s" $fipsDir "fips.database.dat" ]
    puts "Using fips database file: $::fips_database_dat"
    set ::fsbirnid_dat [format "%s%s" $fipsDir "FSBIRNID.dat" ]
    puts "Mapping BIRN ID to FreeSurferID with $::fsbirnid_dat"
    set ::freesurfer_subjectsdir  $fsSubDir 
    puts "Using freesurfer subject dir: $::freesurfer_subjectsdir"
    set ::fips2mrml_FScolors [ format "%s%s" $slicer3Dir "/Libs/FreeSurfer/FreeSurferColorLUT.txt" ]
    if { [ file exists $::fips2mrml_FScolors ] } {
        puts "Using freesurfer colors: $::fips2mrml_FScolors"
    } else {
        puts "Freesurfer colors file $::fips2mrml_FScolors not found."
        exit
    }

    
    #--- creating a new toplevel window 
    puts "making new $w"
    toplevel $w -bg white
    wm title $w "FIPS to MRML"
    wm protocol $w WM_DELETE_WINDOW "QueryAtlasDeleteFips2MrmlToplevel $w"

    fips2mrml_InitializeNumToCharArray
    fips2mrml_InitializeParseFields
    
    #--- create and pack main frames 
    frame $w.container -width 200 -height 800 -bg white
    pack $w.container

    set f $w.container
    
    frame $f.fLogo -bg white
    frame $f.fEntries -bg white
    frame $f.fButton1 -bg white
    frame $f.fListbox -bg white
    frame $f.fButton2  -bg white
    frame $f.fButton3 -bg white
    frame $f.fMessage -bg white
    pack $f.fLogo $f.fEntries $f.fButton1 $f.fListbox $f.fButton2 $f.fButton3 $f.fMessage -side top -fill x -expand y -pady 4

    #--- create logo frame
    set nowframe $f.fLogo
    if { [ file exists "$::env(SLICER_HOME)/Modules/QueryAtlas/ImageData/BIRNLogo.gif" ] } {
        set logo [ image create photo -file "$::env(SLICER_HOME)/Modules/QueryAtlas/ImageData/BIRNLogo.gif" ]
        label $nowframe.lLogo -image $logo
        pack $nowframe.lLogo -side top -anchor e -padx 4
    }

    #--- create label and entry pairs to get search params
    set nowframe $f.fEntries
    label $nowframe.lBIRNID -text "BIRN ID: " -bg White
    entry $nowframe.eBIRNID -textvariable ::fips2mrml_BIRNId
    label $nowframe.lParadigm -text "Paradigm: "  -bg White
    entry $nowframe.eParadigm -textvariable ::fips2mrml_Paradigm
    label $nowframe.lRecSite -text "Rec Site: "  -bg White
    entry $nowframe.eRecSite -textvariable ::fips2mrml_RecSite
    label $nowframe.lAcqSite -text "Acq Site: "  -bg White
    entry $nowframe.eAcqSite -textvariable ::fips2mrml_AcqSite
    label $nowframe.lRunNum -text "Run Number: "  -bg White
    entry $nowframe.eRunNum -textvariable ::fips2mrml_RunNo
    label $nowframe.lStudyID -text "Study ID: "  -bg White
    entry $nowframe.eStudyID -textvariable ::fips2mrml_StudyId
    label $nowframe.lVisitID -text "Visit ID: "  -bg White
    entry $nowframe.eVisitID -textvariable ::fips2mrml_VisitiId
    grid $nowframe.lBIRNID $nowframe.eBIRNID -row 0 -padx 3 -pady 4
    grid $nowframe.lParadigm $nowframe.eParadigm -row 1 -padx 3 -pady 4
    grid $nowframe.lRecSite $nowframe.eRecSite -row 2 -padx 3 -pady 4
    grid $nowframe.lAcqSite $nowframe.eAcqSite -row 3 -padx 3 -pady 4
    grid $nowframe.lRunNum $nowframe.eRunNum -row 4 -padx 3 -pady 4
    grid $nowframe.lStudyID $nowframe.eStudyID -row 6 -padx 3 -pady 4
    grid $nowframe.lVisitID $nowframe.eVisitID -row 5 -padx 3 -pady 4


    #--- create buttons to clear search params and to initiate search
    set nowframe $f.fButton1
    button $nowframe.bClear -text "Clear" -command { fips2mrml_InitializeParseFields } -bg lavender
    button $nowframe.bSearch -text "Search" -command {fips2mrml_SearchAndReport } -bg lavender
    grid $nowframe.bClear $nowframe.bSearch -row 0 -padx 3 -pady 4

    #--- create listbox to display results of search
    set nowframe $f.fListbox
    listbox $nowframe.lbPicker -selectmode single -width 80 -xscrollcommand "$nowframe.scroll set"
    scrollbar $nowframe.scroll -orient horizontal -command "$nowframe.lbPicker xview"
    set ::fips2mrml_Listbox $nowframe.lbPicker
    pack $nowframe.lbPicker $nowframe.scroll -side top -fill x -expand y 

    #---  create entry to specify basename of mrml files
    set nowframe $f.fButton2
    label $nowframe.lFile -text "File to write: "  -bg White
    entry $nowframe.eFile -textvariable ::fips2mrml_MRMLFile -width 100
    grid $nowframe.lFile $nowframe.eFile -row 0 -padx 3 -pady 8


    #---  create buttons to write MRML file and to quit
    set nowframe $f.fButton3
    button $nowframe.bLoad -text "Load in Slicer" -command {fips2mrml_LoadSelection } -bg lavender
    button $nowframe.bWrite -text "Save as MRML" -command {fips2mrml_ConvertSelection } -bg lavender
    #--- lower but don't quit
    button $nowframe.bQuit -text "Quit" -command { $w destroy } -bg lavender
    grid $nowframe.bLoad $nowframe.bWrite $nowframe.bQuit -row 1 -padx 3 -pady 4

    set nowframe $f.fMessage
    $nowframe config -bg white
    label $nowframe.lMessage -text "" -bg lavender
    pack $nowframe.lMessage -side top -fill x -anchor c -padx 4 -pady 4
    set ::fips2mrml_Message $nowframe.lMessage


}
