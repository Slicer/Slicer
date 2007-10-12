
#----------------------------------------------------------------------------------------------------
# Descrciption:
# Initializes ontology resource files: not currently used
# Label is the term in the sourceTermSet; targetTermSet contains the translation 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasVocabularyInit { term sourceTermSet targetTermSet } {

    set ::QA(controlledVocabulary) "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/controlledVocabulary.csv"
    set ::QA(braininfoSynonyms) "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/NN2002-2-synonyms.csv"
    set ::QA(braininfoURI) "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/NN2002-3-BrainInfoURI.csv"
}


#----------------------------------------------------------------------------------------------------
# Descrciption:
# Checks to see that the naming system is recognized.
# If new naming schemes are added, such as FMA, add to
# this list.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasValidSystemCheck { termSet } {

    #--- what systems are valid?
    set candidates { FreeSurfer
        freesurfer
        BIRN_String
        birn_string
        BIRN_ID
        birn_id
        BIRN_URI
        birn_uri
        NN_ID
        NN_id
        NN_String
        NN_string
        IBVD
        UMLS_CN
        umls_cn
        UMLS_CID
        umls_cid }

    foreach c $candidates {
        if { $termSet == $c } {
            return 1
        } 
    }

    return 0
    

}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFilterLocalTerms { terms } {

    regsub -all "/" $terms "+" terms
    regsub -all -- "-" $terms "+" terms
    regsub -all -- "_" $terms "+" terms
    regsub -all -- " " $terms "+" terms
    regsub -all "ctx" $terms "cortex" terms
    regsub -all "rh" $terms "right+hemisphere" terms
    regsub -all "lh" $terms "left+hemisphere" terms
    set terms [ string trimright $terms "+" ]
    set terms [ string trimleft $terms "+" ]
    return $terms

}




#----------------------------------------------------------------------------------------------------
# Descrciption:
# Returns 1 if entry is mapped to its parent structure
# in the controlled vocabulary
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMappedToBIRNLexParentCheck {  } {

}


#----------------------------------------------------------------------------------------------------
# Descrciption:
# Parses the NeuroNames Synonyms for a structure
# into global array ::QA(Synonyms) for later indexing.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseNeuroNamesSynonyms { } {
    #--- start fresh
    if { [ info exists ::QA(Synonyms) ] } {
        unset -nocomplain ::QA(Synonyms)
    }
    set ::QA(Synonyms) ""

    #--- The controlled vocabulary must be a CSV file
    set synonyms "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/NN2002-2-synonyms.csv"

    #--- get number of columns each entry should have
    set fp [ open $synonyms r ]
    gets $fp line
    set sline [ ::QueryAtlasCSV::split $line ]
    set numCols [ llength $sline ]
    close $fp
    
    set fp [ open $synonyms r ]
    #--- parse the file into a list of lists called $::QA(Synonyms)
    while { ! [eof $fp ] } {
        gets $fp line
        set sline [ ::QueryAtlasCSV::split $line ]
        set len [ llength $sline ]
        #--- if the line is the wrong length, blow it off
        if { $len == $numCols } {
            lappend ::QA(Synonyms) $sline
        } 
    }
    close $fp

}


#----------------------------------------------------------------------------------------------------
# Descrciption:
# Parses BrainInfo's NeuroNames URIs into a global array
# called ::QA(BrainInfoURIs) for later lookup. 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseBrainInfoURIs { } {
    #--- start fresh
    if { [ info exists ::QA(BrainInfoURIs) ] } {
        unset -nocomplain ::QA(BrainInfoURIs)
    }
    set ::QA(BrainInfoURIs) ""

    #--- The controlled vocabulary must be a CSV file
    set uris "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/NN2002-3-BrainInfoURI.csv"

    #--- get number of columns each entry should have
    set fp [ open $uris r ]
    gets $fp line
    set sline [ ::QueryAtlasCSV::split $line ]
    set numCols [ llength $sline ]
    close $fp
    
    set fp [ open $uris r ]
    #--- parse the file into a list of lists called $::QA(Synonyms)
    while { ! [eof $fp ] } {
        gets $fp line
        set sline [ ::QueryAtlasCSV::split $line ]
        set len [ llength $sline ]
        #--- if the line is the wrong length, blow it off
        if { $len == $numCols } {
            lappend ::QA(BrainInfoURIs) $sline
        } 
    }
    close $fp

}

#----------------------------------------------------------------------------------------------------
# Descrciption:
# Parses the controlled vocabular into a global list of lists
# called ::QA(CV) for later indexing. 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseControlledVocabulary { } {

    
    #--- start fresh
    if { [ info exists ::QA(CV) ] } {
        unset -nocomplain ::QA(CV)
    }
    set ::QA(CV) ""

    #--- The controlled vocabulary must be a CSV file
    set controlledVocabulary "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Resources/controlledVocabulary.csv"

    #--- get number of columns each entry should have
    set fp [ open $controlledVocabulary r ]
    gets $fp line
    set sline [ ::QueryAtlasCSV::split $line ]
    set numCols [ llength $sline ]
    close $fp
    
    set fp [ open $controlledVocabulary r ]
    #--- parse the file into a list of lists called $::QA(CV)
    while { ! [eof $fp ] } {
        gets $fp line
        set sline [ ::QueryAtlasCSV::split $line ]
        set len [ llength $sline ]
        #--- if the line is the wrong length, blow it off
        if { $len == $numCols } {
            lappend ::QA(CV) $sline
        } 
    }
    close $fp
}

#----------------------------------------------------------------------------------------------------
# Descrciption:
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetBrainInfoURI { term } {
    #--- if there's no controlled vocabulary parsed out, return.
    if { ![ info exists ::QA(BrainInfoURIs) ] } {
        return ""
    }
    if { $::QA(BrainInfoURIs) == "" } {
        return ""
    }

    set NNID [ QueryAtlasMapTerm $term "FreeSurfer" "NN_ID" ]

    #--- FIND the columns in the controlled vocabulary
    #--- that map to target and source TermSets
    set targetColName "BrainInfo URL"
    set targetColNum -1
    set line [ lindex $::QA(BrainInfoURIs) 0 ]
    set len [ llength $line ]
    for { set i 0 } { $i < $len } { incr i } {
        set col [ lindex $line $i ]
       if { $col == $targetColName } {
            set targetColNum $i
        }
    }
    if {$targetColNum > 0 } {
        #--- now march thru ::QA(BrainInfoURIs) down the source Column to find term
        set numRows [ llength $::QA(BrainInfoURIs) ]
        for { set i 0 } { $i < $numRows } { incr i } {
            set row [ lindex $::QA(BrainInfoURIs) $i ]
            set getT [ lindex $row 0 ]
            if { $getT == $NNID } {
                set targetT [ lindex $row $targetColNum ]
                return $targetT
            }
        }
    }

    #--- well, nothing in the table. Return top search page.
    return "http://braininfo.rprc.washington.edu/indexsearchby.html"

}


#----------------------------------------------------------------------------------------------------
# Descrciption:
# Given a term in the FreeSurfer
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSynonyms { term termType } {

    #--- if there's no controlled vocabulary parsed out, return.
    if { ![ info exists ::QA(Synonyms) ] } {
        return ""
    }
    if { $::QA(Synonyms) == "" } {
        return ""
    }
    
    set NNterm [ QueryAtlasMapTerm $term $termType "NN_String" ]

    if { $NNterm != "" } {
        #--- FIND the columns in the controlled vocabulary
        #--- that map to target and source TermSets
        set targetColName "Hierarchy Lookup"
        set target1ColNum -1
        set line [ lindex $::QA(Synonyms) 0 ]
        set len [ llength $line ]
        for { set i 0 } { $i < $len } { incr i } {
            set col [ lindex $line $i ]
            if { $col == $targetColName } {
                set target1ColNum $i
            }
        }
        set targetColName "Species"
        set target2ColNum -1
        set line [ lindex $::QA(Synonyms) 0 ]
        set len [ llength $line ]
        for { set i 0 } { $i < $len } { incr i } {
            set col [ lindex $line $i ]
            if { $col == $targetColName } {
                set target2ColNum $i
            }
        }
        
        set targetColName "Synonym"
        set target3ColNum -1
        set line [ lindex $::QA(Synonyms) 0 ]
        set len [ llength $line ]
        for { set i 0 } { $i < $len } { incr i } {
            set col [ lindex $line $i ]
            if { $col == $targetColName } {
                set target3ColNum $i
            }
        }

        set targetTerms ""
        if { ($target1ColNum > 0) && ($target2ColNum > 0) && ($target3ColNum > 0) } {
            #--- now march thru ::QA(Synonyms) down the
            #--- Hierarchy Lookup column, checking the species column
            #--- and find all synonyms
            set numRows [ llength $::QA(Synonyms) ]
            for { set i 0 } { $i < $numRows } { incr i } {
                set row [ lindex $::QA(Synonyms) $i ]
                #--- get the Heirarchy look up term in this row
                set termT [ lindex $row $target1ColNum ]
                #--- get the species entry in this row
                set speciesT [lindex $row $target2ColNum ]
                #--- if the Hierarchy Lookup term matches the NNterm
                #--- and the term applies to humans, add the synonym
                if { $termT == $NNterm && $speciesT == "human" } {
                    set syn [ lindex $row $target3ColNum ]
                    lappend targetTerms $syn
                }
            }
        }

        #--- return the list of lists, each is a synonym
        return $targetTerms
    }
    return ""

}


#----------------------------------------------------------------------------------------------------
# Descrciption:
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMapTerm { term sourceTermSet targetTermSet } {

    #--- if there's no controlled vocabulary parsed out, return.
    if { ![ info exists ::QA(CV) ] } {
        puts "Can't map term: no controlled vocabulary exists."
        return ""
    }
    if { $::QA(CV) == "" } {
        puts "Can't map term: no controlled vocabulary exists."
        return ""
    }

    #--- make sure sets of terms are valid
    set check [ QueryAtlasValidSystemCheck $sourceTermSet ]
    if { $check == 0 } {
        puts "QueryAtlasVocabularyMapper: $sourceTermSet is not a recognized set of terms"
        return ""
    }
    set check [ QueryAtlasValidSystemCheck $targetTermSet ]
    if { $check == 0 } {
        puts "QueryAtlasVocabularyMapper: $targetTermSet is not a recognized set of terms"
        return ""
    }

    #-- if we're just grabbing the freesurfer term, condition a little
    #-- to get rid of ugly underscores, dashes, etc.

    if { $sourceTermSet == $targetTermSet } {
        #--- turn off conditioning for now.
        if { 0 } {
            if { $sourceTermSet == "FreeSurfer" } {
                regsub -all -- "-" $term " " term
                regsub -all "ctx" $term "Cortex" term
                regsub -all "rh" $term "Right" term
                regsub -all "lh" $term "Left" term
            }
        }
        return $term
    }


    #--- FIND the columns in the controlled vocabulary
    #--- that map to target and source TermSets
    set sourceCol -1
    set targetCol -1
    set line [ lindex $::QA(CV) 0 ]
    set len [ llength $line ]
    for { set i 0 } { $i < $len } { incr i } {
        set col [ lindex $line $i ]
        if { $col == $sourceTermSet } {
            set sourceCol $i
        } elseif { $col == $targetTermSet } {
            set targetCol $i
        }
    }

    if { ($sourceCol > 0) && ($targetCol > 0) } {
        #--- now march thru ::QA(CV) down the source Column to find term
        set numRows [ llength $::QA(CV) ]
        for { set i 0 } { $i < $numRows } { incr i } {
            set row [ lindex $::QA(CV) $i ]
            set sourceT [ lindex $row $sourceCol ]
            if { $sourceT == $term } {
                set targetT [ lindex $row $targetCol ]
                return $targetT
            }
        }
    }

    #--- well, nothing in the table.
    return ""
}

#----------------------------------------------------------------------------------------------------
# Descrciption:
#----------------------------------------------------------------------------------------------------
proc QueryAtlasUpdateSynonymsMenu { term termType } {

    set synonyms [QueryAtlasGetSynonyms $term $termType ]
    set mb [ $::slicer3::QueryAtlasGUI GetSynonymsMenuButton ]
    $mb SetValue ""
    set m [ $mb GetMenu ]

    #--- clear menu
    $m DeleteAllItems

    if { $synonyms != ""  } {
        #--- build menu
        set len [ llength $synonyms ]
        for { set i 0 } { $i < $len } { incr i } {
            #--- add new menuitems
            set item [ lindex $synonyms $i ]
            $m AddRadioButton $item
        }
        $m AddSeparator
        $m AddCommand "close"
        $m SelectItem 0
    }
}



#----------------------------------------------------------------------------------------------------
# Descrciption:
# In the Ontology Frame of the GUI, when a user adds a new
# structure term, this proc looks that term up in all other
# naming systems and populates the various widgets with
# that term's translation.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPopulateOntologyInformation { term infoType } {

    
    #--- convert the "local" naming convention
    #--- into whatever namespace that means.
    #--- for now, all local terms come from FreeSurfer
    #--- so that's straightforward.
    #--- TODO: generalize this so local naming systems
    #--- other than freesurfer are recognized.
    if { $infoType == "local" } {
        set infoType "FreeSurfer"
    }

    #--- when a new term is entered,
    #--- update ontology entries and synonym pulldown
    #--- wrap these up so that hopefully, all will be updated
    #--- once, and the update of each will not trigger repeat
    #--- updates of the others...

    set syn_up 0
    #--- local String
    set val [ QueryAtlasMapTerm $term $infoType "FreeSurfer" ]        
    set curval  [ [$::slicer3::QueryAtlasGUI GetLocalSearchTermEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetLocalSearchTermEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
    #--- BIRN String
    set val [ QueryAtlasMapTerm $term $infoType "BIRN_String" ]        
    set curval  [ [$::slicer3::QueryAtlasGUI GetBIRNLexEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetBIRNLexEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
    #--- BIRN ID
    set val [ QueryAtlasMapTerm $term $infoType "BIRN_ID" ]        
    set curval  [ [$::slicer3::QueryAtlasGUI GetBIRNLexIDEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetBIRNLexIDEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
    #--- NN String
    set val [ QueryAtlasMapTerm $term $infoType "NN_String" ]        
    set curval  [ [$::slicer3::QueryAtlasGUI GetNeuroNamesEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetNeuroNamesEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
    #--- NN ID
    set val [ QueryAtlasMapTerm $term $infoType "NN_ID" ]
    set curval  [ [$::slicer3::QueryAtlasGUI GetNeuroNamesIDEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetNeuroNamesIDEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
    #--- UMLS CID
    set val [ QueryAtlasMapTerm $term $infoType "UMLS_CID" ]                
    set curval  [ [$::slicer3::QueryAtlasGUI GetUMLSCIDEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetUMLSCIDEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }

    #--- UMLS CN
    set val [ QueryAtlasMapTerm $term $infoType "UMLS_CN" ]                
    set curval  [ [$::slicer3::QueryAtlasGUI GetUMLSCNEntry] GetValue ]
    if { $val != $curval } {
        [$::slicer3::QueryAtlasGUI GetUMLSCNEntry] SetValue $val
        #--- update synonyms
        if { ! $syn_up } {
            QueryAtlasUpdateSynonymsMenu $term $infoType
            set syn_up 1
        }
    }
}





#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFreeSurferLabelsToBirnLexLabels { label } {
    
    set retLabel ""
    
    set labelTable "$::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/QueryAtlas/Tcl/FreeSurferLabels2BirnLexLabels.txt"

    set fp [ open $labelTable r ]

    while { ! [eof $fp ] } {
        gets $fp line
        set tst [ string first $label $line ]
        if  { $tst > 0 } {
            #--- get second term in line
            set retLabel [ lindex $line 1 ]
            #--- get rid of underscores
            regsub -all -- "_" $retLabel " " retLabel
            break
        }
    }
    close $fp
    return $retLabel
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasDisplayDefaultBIRNPaths { FSw, FIPSw} {

    set ::QueryAtlas(fipsDir) $::env(FIPSHOME)
    set ::QueryAtlas(fsSubjectDir) $::env(SUBJECTSHOME)

    FSw SetText $::QueryAtlas(fsSubjectDir)
    FIPSw SetText $::QueryAtlas(fipsDir)

}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTrimDirectoryPath { str } {

    #--- get a dir path
    #--- remove trailing "/" slashes
    set tmp [ string trim $str "/" ]
    
    #--- keep all chars up to and including the first "/"
    set index [ string first "/" $tmp ]
    if { $index >= 0 } {
        set front [ string range $tmp 0 $index ]
    } else {
        set front ""
    }
    
    #--- keep all chars after and including the last "/"
    set index2 [ string last "/" $tmp ]
    #--- if there's only one slash in path, don't trim
    if { $index2 == $index1 } {
        set trimpath $str
    } else {
        if { $index2 >= 0 } {
            set back [ string range $tmp $index end ]
            #--- fill the middle with "..."
            set trimpath "$front...$back"
        } else {
            set trimpath "$front..."
        }
    }
}

