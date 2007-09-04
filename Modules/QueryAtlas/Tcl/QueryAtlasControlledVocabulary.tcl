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
        BIRN
        birn
        BIRN_String
        birn_string
        BIRN_ID
        birn_id
        BIRN_URI
        birn_uri
        NeuroNames_ID
        neuronames_id
        NeuroNames_String
        neuronames_string
        neuronames
        UMLS_CID
        umls_cid
        UMLS_CN
        UMLS_cn
        UMLS
        umls
        FMA_ID
        fma_id
        FMA_String
        fma_string
        FMA
        fma }

    foreach c $candidates {
        if { $termSet == $c } {
            return 1
        } 
    }

    return 0
    

}

#----------------------------------------------------------------------------------------------------
#--- counts the number of times character c appears in string str
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCharacterCount { c str  } {
    #--- try this.
    set count [ regexp -all $c $str ]
    return $count
}

#----------------------------------------------------------------------------------------------------
#--- returns 1 if entry is mapped to its parent structure
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMappedToBIRNLexParentCheck {  } {

}



#----------------------------------------------------------------------------------------------------
#--- returns index of Nth occurrance of character c in string str
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetIndexOfCharInstance { c str N } {

    set len [ string length $str ]
    set count 0
    for { set i 0 } { $i < $N } { incr i } {
        set index [ string first $c $str ]
        if { $index < $len } {
            incr index
            set count [ expr $count + $index ]
            set str [ string range $str $index end ]
            set len [ string length $str ]
        } else {
            break
        }
    }  
    return $count
}


#----------------------------------------------------------------------------------------------------
#--- label is the term in the sourceTermSet; targetTermSet contains the translation 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasVocabularyMapper { term sourceTermSet targetTermSet } {
    
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

    #--- The controlled vocabulary must be a CSV file
    set controlledVocabulary "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Resources/controlledVocabulary.csv"
    set fp [ open $controlledVocabulary r ]

    #--- FIND the columns in the controlled vocabulary that map to target and source
    while { ! [eof $fp ] } {
        gets $fp line
        #--- FIND the terms
        set iTarget [ string first $targetTermSet $line ]
        set iSource [ string first $sourceTermSet $line ]

        if { $iTarget >= 0 && $iSource >= 0 } {

            #--- FIND how many commas between line start and index of term
            set tmp [ string range $line 0 [ expr $iTarget - 1 ] ]
            set targetColumn [ QueryAtlasCharacterCount "," $tmp ]

            set tmp [ string range $line 0 [ expr $iSource - 1 ] ]
            set sourceColumn [ QueryAtlasCharacterCount "," $tmp ]
        }

        if { $targetColumn >= 0 && $sourceColumn >= 0 } {
            break
        }
    }

    if { $targetColumn  < 0 || $sourceColumn < 0 } {
        if { $targetColumn < 0 } {
            puts "QueryAtlasVocabularyMapper: Can't find $targetTermSet in the controlled vocabulary."
        } elseif { $sourceColumn < 0 } {
            puts "QueryAtlasVocabularyMapper: Can't find $sourceTermSet in the controlled vocabulary."
        }
        return ""
    }
    
    #--- continue reading the file to find the term
    set retTerm ""
    set mapped 0
    while { ! [eof $fp ] } {

        gets $fp line
        
        #--- is the source term in this line?
        set found [ string first $term $line ] 
        if { $found >= 0 } {

            #-- FIND the character index of the target translation
            set termIndex [ QueryAtlasGetIndexOfCharInstance  "," $line $targetColumn ]

            set zz [ string range $line $termIndex end ]
            set zzi [ string first "," $zz ]
            set retTerm [ string range $zz 0 [ expr $zzi - 1 ] ]

            break
        }
    }
    close $fp
    puts "Translation = $retTerm."
    return $retTerm

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

