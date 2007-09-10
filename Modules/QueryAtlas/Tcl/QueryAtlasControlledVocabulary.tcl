package require csv

#----------------------------------------------------------------------------------------------------
#--- label is the term in the sourceTermSet; targetTermSet contains the translation 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasVocabularyInit { term sourceTermSet targetTermSet } {

    set ::QA(controlledVocabulary) "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Resources/controlledVocabulary.csv"
    set ::QA(braininfoSynonyms) "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Resources/NN2002-2-synonyms.csv"
    set ::QA(braininfoURI) "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Resources/NN2002-3-BrainInfoURI.csv"
}


#----------------------------------------------------------------------------------------------------
#--- checks to see that the naming system is recognized
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
#--- returns 1 if entry is mapped to its parent structure
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMappedToBIRNLexParentCheck {  } {

}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseControlledVocabulary { } {

    
    #--- start fresh
    if { [ info exists ::QA(CV) ] } {
        unset -nocomplain ::QA(CV)
    }
    set ::QA(CV) ""

    if { [catch "package require csv"] } {
        puts "Can't parse controlled vocabulary without package csv"
        return 
    }

    #--- The controlled vocabulary must be a CSV file
    set controlledVocabulary "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Resources/controlledVocabulary_new.csv"

    #--- get number of columns each entry should have
    set fp [ open $controlledVocabulary r ]
    gets $fp line
    set sline [ ::csv::split $line ]
    set numCols [ llength $sline ]
    close $fp
    
    set fp [ open $controlledVocabulary r ]
    #--- parse the file into a list of lists called $::QA(CV)
    while { ! [eof $fp ] } {
        gets $fp line
        set sline [ ::csv::split $line ]
        set len [ llength $sline ]
        #--- if the line is the wrong length, blow it off
        if { $len == $numCols } {
            lappend ::QA(CV) $sline
        } 
    }
    close $fp
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMapTerm { term sourceTermSet targetTermSet } {

    #--- if there's no controlled vocabulary parsed out, return.
    if { ![ info exists ::QA(CV) ] } {
        puts "Can't map term: no controlled vocabulary exists."
        return
    }
    if { $::QA(CV) == "" } {
        puts "Can't map term: no controlled vocabulary exists."
        return
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
    if { $sourceTermSet == "FreeSurfer" && $targetTermSet == "FreeSurfer" } {
        regsub -all -- "-" $term " " term
        regsub -all "ctx" $term "Cortex" term
        regsub -all "rh" $term "Right" term
        regsub -all "lh" $term "Left" term
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
#--- returns synonyms from NeuroNames resources
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetNeuroNamesURI { term sourceTermSet } {

}


#----------------------------------------------------------------------------------------------------
#--- returns synonyms from NeuroNames resources
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetNeuroNamesSynonyms { term sourceTermSet } {

}

