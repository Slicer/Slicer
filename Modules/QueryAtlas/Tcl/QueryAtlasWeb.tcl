
#----------------------------------------------------------------------------------------------------
proc QueryAtlasWriteFirefoxBookmarkFile {bmfile } {
    
    #--- strip off filename
    set flist [ file split $bmfile ]
    set llen [ llength $flist ]
    set title [ lindex $flist [ expr $llen-1 ] ]
    set i [ string first "." $title ]
    set title [ string range $title 0 [expr $i-1] ]
    
    #--- open file for writing
    set fp [open $bmfile "w"]

    #--- write header
    set line "<!DOCTYPE NETSCAPE-Bookmark-file-1>"
    puts $fp $line
    set line "<!-- This is an automatically generated file."
    puts $fp $line
    set line "     It will be read and overwritten."
    puts $fp $line
    set line "     DO NOT EDIT! -->"
    puts $fp $line
    set line "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">"
    puts $fp $line

    #--- write title
    set line "<TITLE>$title</TITLE>"
    puts $fp $line
    set line "<H1>$title</H1>"
    puts $fp $line
    set line "<DT><H3>$title</H3>"
    puts $fp $line
    set line "   <DL><p>"
    puts $fp $line

    #--- now put in links
    set rl [ [ $::slicer3::QueryAtlasGUI GetAccumulatedResultsList ] GetWidget ]
    set num [ $rl GetNumberOfRows ]
    for { set i 0 } { $i < $num } { incr i } {
        set link [ $rl GetCellText $i 1 ]
        set line "        <DT><A HREF=\"$link\" > Slicer3:QueryAtlas link $i </A>"
        puts "adding $line"
        puts $fp $line
    }

    #--- close up
    set line "  </DL><p> "
    puts $fp $line
    close $fp
}




#----------------------------------------------------------------------------------------------------
proc QueryAtlasLoadFirefoxBookmarkFile {bmfile } {

    #--- open file for writing
    set fp [open $bmfile "r"]
    while { ![eof $fp] } {
        gets $fp line
        #--- throw away until we find an HREF
        set tst [ string first "HREF=" $line ]
        if { $tst >=0 } {
            #--- get links
            set tmpstr [ string range $line [ expr $tst + 5] end ]
            set tst [ string first ">" $tmpstr ]
            set tmpstr [ string range $tmpstr 0 [expr $tst -1 ] ]
            #--- trim off any leading or trailing white space or quotes
            set tmpstr [ string trimleft $tmpstr ]
            set tmpstr [ string trimleft $tmpstr "\"" ]
            set tmpstr [ string trimright $tmpstr ]
            set tmpstr [ string trimright $tmpstr "\"" ]
            #--- load them into accumulated results frame
            set rl [ [ $::slicer3::QueryAtlasGUI GetAccumulatedResultsList ] GetWidget ]
            $::slicer3::QueryAtlasGUI AccumulateUniqueResult $tmpstr
        }
    }
}




#----------------------------------------------------------------------------------------------------
proc QueryAtlasOpenLinkFromList { lw } {

    set url [ $lw GetSelection ]
    puts "$url"
    set browser [ $::slicer3::Application GetWebBrowser ]
    exec $browser -new-tab $url &    
    
}



#----------------------------------------------------------------------------------------------------
proc QueryAtlasOpenLink { url } {

    if { $url != "" } {
        if { $::tcl_platform(os) == "Darwin" } {
            exec open $url
        } else {
            set browser [ $::slicer3::Application GetWebBrowser ]

            if { ![file executable $browser] } {
                set dialog [vtkKWMessageDialog New]
                $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
                $dialog SetStyleToMessage
                $dialog SetText "Please use the following dialog to set the path to the Firefox Browser and then re-run your query."
                $dialog Create
                $dialog Invoke
                $dialog Delete

                set window [$::slicer3::ApplicationGUI GetMainSlicerWindow] 
                set interface [$window GetApplicationSettingsInterface] 
                $interface Show
                set manager [$interface GetUserInterfaceManager]
                $manager RaiseSection 0 "Slicer Settings"
                return
            } else {
                #--- Test: does this work on all windows os/versions?
                if { $::tcl_platform(platform) == "windows" } {
                    puts "opening $url"
                    exec $browser -new-tab $url &
                } else {
                    #--- what will work on macos?
                }
            }
        }
    }
}




#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetStructureTerm { } {
    [ $::slicer3::QueryAtlasGUI GetLocalSearchTermEntry ] SetValue $::QA(localLabel) 
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSearchTargets { } {

    set mb [$::slicer3::QueryAtlasGUI GetDatabasesMenuButton ]
    set target ""
    set target [ $mb GetValue ]
    return $target
}



#----------------------------------------------------------------------------------------------------
#--- adds a term into the ontology panel's listbox
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddToSavedTerms {} {

    #--- add term to listbox.
    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set mcl [[[$::slicer3::QueryAtlasGUI GetSavedTerms] GetMultiColumnList] GetWidget]

    puts "prefiltered terms are $::QA(lastLabels)"
    #--- filter terms for friendlier viewing
    set terms [ QueryAtlasFilterLocalTerms $::QA(lastLabels) ]
    
    set n [$mcl GetNumberOfRows]
    #--- check for uniqueness
    set unique 1
    for { set i 0 } { $i < $n } { incr i } {
        set str [ $mcl GetCellText $i 0 ]
        if { $terms == $str } {
            set unique 0
            break
        }
    }

    if { $unique } {
        #--- add new row
        $mcl AddRow
        set n [$mcl GetNumberOfRows]
        $mcl SetCellText [ expr $n - 1 ] 0 $terms
        $mcl SetCellBackgroundColor [ expr $n - 1 ] 0 1.0 1.0 1.0
    }

}


#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddEntryTermToSavedTerms { terms } {

    #--- add term to listbox.
    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set mcl [[[$::slicer3::QueryAtlasGUI GetSavedTerms] GetMultiColumnList] GetWidget]

    #--- filter local terms for friendlier viewing
    if { $terms != "<new>" } {
        set terms [ QueryAtlasFilterLocalTerms $terms ]
    }
    
    set n [$mcl GetNumberOfRows]
    #--- check for uniqueness
    set unique 1
    for { set i 0 } { $i < $n } { incr i } {
        set str [ $mcl GetCellText $i 0 ]
        if { $terms == $str } {
            set unique 0
            break
        }
    }

    if { $unique || ($terms == "<new>") } {
        #--- add new row
        $mcl AddRow
        set n [$mcl GetNumberOfRows]
        $mcl SetCellText [ expr $n - 1 ] 0 $terms
        $mcl SetCellBackgroundColor [ expr $n - 1 ] 0 1.0 1.0 1.0
    }

}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasQueryBrainInfo { url } {
    if { $url != "" } {
        QueryAtlasOpenLink $url 
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasEncodeTerms { terms } {

    #--- detect artifacts of local naming schemes
    #--- and try to prettify them.
    regsub -all "\/" $terms "\+" terms
    regsub -all " " $terms "\+" terms
    regsub -all -- "\-" $terms "\+" terms
    regsub -all "\"" $terms "%22" terms

    regsub -all "ctx" $terms "cortex" terms
    regsub -all "rh" $terms "right+hemisphere" terms
    regsub -all "lh" $terms "left+hemisphere" terms

    #--- TODO: make this work.
    if { 0 } {
        #--- reserved characters...
        regsub -all "\%" $terms "\%25" terms
        regsub -all "+" $terms "\%2B" terms
        regsub -all "\$" $terms "\%24" terms
        regsub -all "\&" $terms "\%26" terms
        regsub -all "\," $terms "\%2C" terms
        regsub -all "\/" $terms "\%2F" terms
        regsub -all "\:" $terms "\%3A" terms
        regsub -all "\;" $terms "\%3B" terms
        regsub -all "\=" $terms "\%3D" terms
        regsub -all "\?" $terms "\%3F" terms
        regsub -all "\@" $terms "\%40" terms

        #--- unsafe characters...
        regsub -all "\"" $terms "\%22" terms
        regsub -all "\>" $terms "\%3E" terms    
        regsub -all "\<" $terms "\%3C" terms    
        regsub -all "\#" $terms "\%23" terms    
        regsub -all "\{" $terms "\%7B" terms    
        regsub -all "\}" $terms "\%7D" terms    
        regsub -all "\|" $terms "\%7C" terms    
        regsub -all "\\" $terms "\%5C" terms    
        regsub -all "\^" $terms "\%5E" terms    
        regsub -all "\~" $terms "\%7E" terms    
        regsub -all "\[" $terms "\%5B" terms    
        regsub -all "\]" $terms "\%5D" terms    
        regsub -all "\`" $terms "\%60" terms
    }
    return $terms

}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasContextQuery { site } {

    if { $::QA(lastLabels) == "background" || $::QA(lastLabels) == "Unknown" } {
        set terms ""
    } else {
        set terms $::QA(lastLabels)
    }

    set terms [ QueryAtlasEncodeTerms $terms ]
    
    switch $site {
        "google" {
            set url "http://www.google.com/search?q=$terms"
            QueryAtlasOpenLink $url
        }
        "wikipedia" {
            set url "http://www.google.com/search?q=$terms+site:en.wikipedia.org"
            QueryAtlasOpenLink $url
        }
        "pubmed" {
            set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=search&db=PubMed&term=$terms"
            QueryAtlasOpenLink $url
        }
        "jneurosci" {
            set url "http://www.jneurosci.org/cgi/search?volume=&firstpage=&sendit=Search&author1=&author2=&titleabstract=&fulltext=$terms"
            QueryAtlasOpenLink $url
        }
        "braininfo" {
            set url [ QueryAtlasGetBrainInfoURI $::QA(lastLabels) ]
            QueryAtlasQueryBrainInfo $url
        }
        "ibvd form" {
            set anatomy [ QueryAtlasMapTerm $::QA(lastLabels) $::QA(annotationTermSet) "IBVD" ]
            set terms "human,normal,$anatomy"
            set url http://www.cma.mgh.harvard.edu/ibvd/search.php?f_submission=true&f_free=$commaterms
            QueryAtlasOpenLink $url
        }
        "ibvd: howbig?" {
            puts "$::QA(lastLabels)"
            if { $::QA(annotationTermSet) == "BIRNLex" } {
                set terms [ QueryAtlasMapTerm $::QA(lastLabels) "BIRN_String" "IBVD" ]
            } elseif { $::QA(annotationTermSet) == "NeuroNames" } {
                set terms [ QueryAtlasMapTerm $::QA(lastLabels) "NN_String" "IBVD" ]
            } elseif { $::QA(annotationTermSet) == "UMLS" } {
                set terms [ QueryAtlasMapTerm $::QA(lastLabels) "UMLS_CID" "IBVD" ]
            } elseif { $::QA(annotationTermSet) == "local" } {
                set terms [ QueryAtlasMapTerm $::QA(lastLabels) "FreeSurfer" "IBVD" ]                
            } elseif { $::QA(annotationTermSet) == "IBVD" } {
                set terms [ QueryAtlasMapTerm $::QA(lastLabels) "IBVD" "IBVD" ]                
            } else {
                set terms ""
            }
            puts "$::QA(lastLabels)"
            puts "$terms"
            #--- set url http://www.cma.mgh.harvard.edu/ibvd/how_big.php?structure=$terms&diagnosis=$dterms
            set url http://www.cma.mgh.harvard.edu/ibvd/how_big.php?structure=$terms
            QueryAtlasOpenLink $url
        }
    }
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetStructureTerms { } {


    set w [ $::slicer3::QueryAtlasGUI GetStructureListWidget ]
    set ww [ [$w GetMultiColumnList ] GetWidget ]
    
    set terms ""
    set ::QA(StructureTerms) ""
    set numrows [ $ww GetNumberOfRows ]
    for { set i 0 } { $i < $numrows } { incr i } {
        #--- if term is selected for use:
        if { [ $ww IsRowSelected $i ] } {
            set term [ $ww GetCellText $i 0 ]
            append terms $term
            append terms "+"
        }
    }
    set terms [ string trimright $terms "+" ]
    set ::QA(StructureTerms) $terms
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetOtherTerms { } {
    set w [ $::slicer3::QueryAtlasGUI GetOtherListWidget ]
    set ww [ [$w GetMultiColumnList ] GetWidget ]

    set terms ""
    set ::QA(OtherTerms) ""
    set numrows [ $ww GetNumberOfRows ]
    for { set i 0 } { $i < $numrows } { incr i } {
        #--- if term is selected for use:
        if { [ $ww IsRowSelected $i ] } {
            set term [ $ww GetCellText $i 0 ]
            append terms $term
            append terms "+"
        }
    }
    set terms [ string trimright $terms "+" ]
    set ::QA(OtherTerms) $terms
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetPopulationTerms { } {

    set terms ""
    set ::QA(PopulationTerms) ""

    set m $::slicer3::QueryAtlasGUI
    set termD [[[ $m GetDiagnosisMenuButton ] GetWidget ] GetValue ]
    if { $termD != "" && $termD != "n/a" && $termD != "Normal"} {
        append terms "%22"
        append terms $termD
        append terms "%22"
        append terms "+"
    }

    set termG [[[ $m GetGenderMenuButton ] GetWidget ] GetValue ]    
    if { $termG != "" && $termG != "n/a" } {
        #--- clarify some of the term selections from GUI
        if { $termG == "F" } {
            set termG "Female"
        } elseif { $termG == "M" } {
            set termG "Male"
        } elseif { $termG == "mixed" } {
            set termG "%22mixed gender%22"
        }
        append terms $termG
        append terms "+"
    }

    set termA [[[ $m GetAgeMenuButton ] GetWidget ] GetValue ]    
    if { $termA != "" && $termA != "n/a" } {
        append terms $termA
        append terms "+"
    }

    set termH [[[ $m GetHandednessMenuButton ] GetWidget ] GetValue ]    
    if { $termH != "" && $termH != "n/a" } {
        if { $termH == "left" } {
            set termH "%22left handed%22"
        } elseif { $termH == "right" } {
            set termH "%22right handed%22"
        } elseif { $termH == "both" } {
            set termH "ambidextrous"
        }
        append terms $termH
    }
    set terms [ string trimright $terms "+" ]
    set ::QA(PopulationTerms) $terms
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSpeciesTerms { } {

    set ::QA(speciesTerms) ""
    set b  [$::slicer3::QueryAtlasGUI GetSpeciesNoneButton ]
    set species [$b GetVariableValue ]
    if { $species == "human" || $species == "mouse" || $species == "macaque" } {
        set ::QA(SpeciesTerms) $species
    }
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAppendStructureTerms { terms } {
    
    QueryAtlasGetStructureTerms

    if { [ info exists ::QA(StructureTerms) ]  } {
        if { ($terms != "")  &&  ($::QA(StructureTerms) != "" ) } {
            append terms "+"
        }
        append terms $::QA(StructureTerms)
    }
    return $terms
}

    
#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAppendPopulationTerms { terms } {

    QueryAtlasGetPopulationTerms
    if { [ info exists ::QA(PopulationTerms) ]  } {
        if { ($terms) != ""  &&  ($::QA(PopulationTerms) != "" ) } {
            append terms "+"
        }
        append terms $::QA(PopulationTerms)        
    }
    return $terms
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAppendSpeciesTerms { terms } {

    QueryAtlasGetSpeciesTerms
    if { [ info exists ::QA(SpeciesTerms) ]  } {
        if { ( $terms != "")  && ( $::QA(SpeciesTerms) != "") } {
            append terms "+"
        }
        append terms $::QA(SpeciesTerms)        
    }
    return $terms
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAppendOtherTerms { terms } {

    QueryAtlasGetOtherTerms
    if { [ info exists ::QA(OtherTerms) ]  } {
        if { ($terms != "") && ( $::QA(OtherTerms) != "") } {
            append terms "+"
        }
        append terms $::QA(OtherTerms)
    }
    return $terms
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForGoogle { } {
    #--- get things from GUI:

    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]

    set terms [ QueryAtlasEncodeTerms $terms ]
    puts "terms = $terms"

    set ::QA(url,Google) ""
    #--- now terms contains all categories user chose
    if { $terms != "" } {
        set url "http://www.google.com/search?hl-en&q="
        append url $terms
        append url "&btnG=Google+Search"
        set ::QA(url,Google) $url
        puts "url = $url"
    }
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForWikipedia { } {

    #--- get things from GUI: try structure terms first.
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]

    #--- if there were no structure terms, get diagnosis terms
    if { $terms == "" } {
        if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
            set terms [ QueryAtlasAppendPopulationTerms $terms ]
        }
        #--- strip off handedness terms
        set idx [ string first "%22left handed" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "%22right handed" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "ambidextrous" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        #--- strip off any gender terms
        set idx [ string first "Female" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "Male" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "%22 mixed" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        #--- strip off any age terms
        set idx [ string first "neonate" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "infant" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "child" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "adolescent" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "adult" $terms]
        set tmpstr [ string range $terms 0 $idx ]
        set idx [ string first "elderly" $terms]
        set tmpstr [ string range $terms 0 $idx ]
    }
    set terms [ string trimright $terms "+" ]    

    #--- if there were no important diagnosis terms, use other terms
    if { $terms == "" || $terms == "\%22Normal\%22" } {
        if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
            set terms [ QueryAtlasAppendOtherTerms $terms ]
        }        
    }
    set terms [ string trimright $terms "+" ]

    #--- finally, if there were no other terms, use species terms
    if { $terms == "" } {
        if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
            set terms [ QueryAtlasAppendSpeciesTerms $terms ]
        }        
    }
    set terms [ string trimright $terms "+" ]

    #--- now terms contains all categories user chose
    set terms [ QueryAtlasEncodeTerms $terms ]
    puts "$terms"
    set ::QA(url,Wikipedia) ""
    if { $terms != "" } {
        set target [ QueryAtlasGetSearchTargets ]
        set url "http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search="
        append url "$terms"
        append url "\&fulltext=Search"
        set ::QA(url,Wikipedia) $url
    }    
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForIBVD { } {

    #--- get things from GUI:
    set terms ""
    set terms [ QueryAtlasAppendStructureTerms $terms ]
#    set terms [ QueryAtlasAppendPopulationTerms $terms ]
#    set terms [ QueryAtlasAppendSpeciesTerms $terms ]
#    set terms [ QueryAtlasAppendOtherTerms $terms ]
    set terms [ string trimright $terms "+" ]
    #--- now terms contains all.
    set target [ QueryAtlasGetSearchTargets ]

    set ::QA(url,IBVD) ""
    if { $terms != "" } {
        set url "http://www.cma.mgh.harvard.edu/ibvd/how_big.php?"
        #--- get only the first structure from the structurelist
        set i [ string first "+" $structure ]
        if { $i > 0 } {
            #--- found a "+", so grab all chars up to it
            set singleStructure [ string range $structure 0 [ expr $i - 1 ] ]
        } else {
            singleStructure = $structure
        }
        
        # break this down for all demographics that ibvd allow and we offer
        # structure from menu...
        append url "structure=$singleStructure"

        #--- TODO: figure out how to format url
        # diagnosis from menu
        #append url "\&diagnosis=$diagnosis"
        # gender from menu
        #if { $gender != "n/a" } {
        #    append url "\&gender=$gender"
        #}
        set ::QA(url,IBVD) $url
    }
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForPubMed { } {

    #--- get things from GUI:
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]    

    set ::QA(url,PubMed) ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]
        #--- TODO:
        #--- pubmed wants multi word thing + otherthing to look like
        #-- "multi+word+thing"+otherthing
        set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=search&db=PubMed&term=$terms"
        set ::QA(url,PubMed) $url
    }
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForPubMedCentral { } {

    #--- get things from GUI:
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]    

    set ::QA(url,PubMedCentral) ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]
        #--- TODO:
        #--- pubmed wants multi word thing + otherthing to look like
        #-- "multi+word+thing"+otherthing
        set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=search&db=pmc&term=$terms"
        set ::QA(url,PubMedCentral) $url
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForJNeurosci { } {


    #--- get things from GUI:
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]    
    set ::QA(url,JNeurosci) ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]

        #--- TODO:
        #--- here, we want to put double quotes around each separate
        #--- structure, cell, gene and misc term.
        #--- jneurosci wants multi word thing + otherthing to look like
        #--- "multi word thing"+otherthing
        set url "http://www.jneurosci.org/cgi/search?volume=&firstpage=&sendit=Search&author1=&author2=&titleabstract=&fulltext=$terms"
        set ::QA(url,JNeurosci) $url
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForPLoS { } {


    #--- get things from GUI:
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]    
    set ::QA(url,PLoS) ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]

        #--- TODO:
        #--- here, we want to put double quotes around each separate
        #--- structure, cell, gene and misc term.
        #--- jneurosci wants multi word thing + otherthing to look like
        #--- "multi word thing"+otherthing
        set url "http://www.jneurosci.org/cgi/search?volume=&firstpage=&sendit=Search&author1=&author2=&titleabstract=&fulltext=$terms"
        set ::QA(url,PLoS) $url
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForMetasearch { } {

    #--- get things from GUI:
    set terms ""
    if { [ [$::slicer3::QueryAtlasGUI GetUseStructureTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendStructureTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseGroupTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendPopulationTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseSpeciesTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendSpeciesTerms $terms ]
    }
    if { [ [$::slicer3::QueryAtlasGUI GetUseOtherTerms ] GetSelectedState ] == 1 } {
        set terms [ QueryAtlasAppendOtherTerms $terms ]
    }
    set terms [ string trimright $terms "+" ]    
    set ::QA(url,Metasearch)  ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]
        set url "https://loci.ucsd.edu/qametasearch/query.do?query=$terms"
        set ::QA(url,Metasearch) $url
    }
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForBraininfo { } {

    #--- get things from GUI:
    set terms ""
    set terms [ QueryAtlasAppendStructureTerms $terms ]
#    set terms [ QueryAtlasAppendPopulationTerms $terms ]
#    set terms [ QueryAtlasAppendSpeciesTerms $terms ]
#    set terms [ QueryAtlasAppendOtherTerms $terms ]
    set terms [ string trimright $terms "+" ]    
    set ::QA(url,Braininfo) ""
    if { $terms != "" } {
        #--- now terms contains all.
        set target [ QueryAtlasGetSearchTargets ]

        #--- TODO: just take first structure term.
        set i [ string first "+" $structure ]
        if { $i >= 0 } {
            set structure [ string range $structure 0 [expr $i - 1 ] ]
        } 
        set url "http://braininfo.rprc.washington.edu/Scripts/hiercentraldirectory.aspx?ID=$structure"
        set ::QA(url,Braininfo) $url
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLsForTargets { } {

    set target [ QueryAtlasGetSearchTargets ]

    unset -nocomplain ::QA(url,PubMed)
    unset -nocomplain ::QA(url,PubMedCentral)
    unset -nocomplain ::QA(url,JNeurosci)
    unset -nocomplain ::QA(url,Metasearch)
    unset -nocomplain ::QA(url,PLoS)
    unset -nocomplain ::QA(url,IBVD)
    unset -nocomplain ::QA(url,Google)
    unset -nocomplain ::QA(url,Wikipedia)
    
    
    #--- Form urls for each Search Target requested.
    #--- PubMed
    if { ($target == "PubMed") || ($target == "all") } {
        QueryAtlasFormURLForPubMed
        if {$::QA(url,PubMed) == "" } {
            QueryAtlasEmptyURLMessage "PubMed"
        } else {
            puts "$::QA(url,PubMed)"
        }
    }

    #--- PUBMED CENTRAL     
    if { ($target == "all") || ($target == "PubMedCentral") } {
        QueryAtlasFormURLsForPubMedCentral
        if { $::QA(url,PubMedCentral) == "" } {
            QueryAtlasEmptyURLMessage "PubMedCentral"
        } else {
            puts "$::QA(url,PubMedCentral)"
        }
    }
         
    if { ($target == "PubMed") || ($target == "all") } {
        QueryAtlasFormURLForPLoS
        if {$::QA(url,PubMed) == "" } {
            QueryAtlasEmptyURLMessage "PLoS"
        } else {
            puts "$::QA(url,PLoS)"
        }
    }

    #--- J Neurosci
    if { ($target == "JNeurosci") || ($target == "all") } {
        QueryAtlasFormURLForJNeurosci
        if { $::QA(url,JNeurosci) == "" } {
            QueryAtlasEmptyURLMessage "Journal of Neuroscience"
        } else {
            puts "$::QA(url,JNeurosci)"
        }
    }
    
    #--- metasearch
    if { ($target == "Metasearch") || ($target == "all") } {
        QueryAtlasFormURLForMetasearch 
        if { $::QA(url,Metasearch) == "" } {
            QueryAtlasEmptyURLMessage "Metasearch"
        } else {
            puts "$::QA(url,Metasearch)"
        }
    }
    #--- IBVD
    if { ($target == "all") || ($target == "IBVD") } {
        QueryAtlasFormURLForIBVD
        if { $::QA(url,IBVD) == "" } {
            QueryAtlasEmptyURLMessage "IBVD"
        } else {
            puts "$::QA(url,IBVD)"
        }
    }
    #--- GOOGLE
    if { ($target == "all") || ($target == "Google") } {
        QueryAtlasFormURLForGoogle
        if { $::QA(url,Google) == "" } {
            QueryAtlasEmptyURLMessage "Google"
        } else {
            puts "$::QA(url,Google)"
        }
    }
    #--- WIKIPEDIA
    if { ($target == "all") || ($target == "Wikipedia") } {
        QueryAtlasFormURLForWikipedia
        if { $::QA(url,Wikipedia) == "" } {
            QueryAtlasEmptyURLMessage "Wikipedia"
        } else {
            puts "$::QA(url,Wikipedia)"
        }
    }
    QueryAtlasPopulateSearchResultsBox
    
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasEmptyURLMessage { target } {
    set dialog [vtkKWMessageDialog New]
    $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetStyleToMessage
    $dialog SetText "Unable to formulate good url for $target: please check your selected search terms."
    $dialog Create
    $dialog Invoke
    $dialog Delete
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPopulateSearchResultsBox { } {

    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set lb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]    

    if { [ info exists ::QA(url,Google) ] } {
        if { $::QA(url,Google) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,Google)
        }
    }
    if { [ info exists ::QA(url,Wikipedia) ] } {    
        if { $::QA(url,Wikipedia) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,Wikipedia)
        }
    }
    if { [ info exists ::QA(url,PubMed) ] } {    
        if { $::QA(url,PubMed) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,PubMed)
        }
    }
    if { [ info exists ::QA(url,PubMedCentral) ] } {    
        if { $::QA(url,PubMed) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,PubMed)
        }
    }
    if { [ info exists ::QA(url,Metasearch) ] } {    
        if { $::QA(url,PubMed) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,PubMed)
        }
    }
    if { [ info exists ::QA(url,PLoS) ] } {    
        if { $::QA(url,PubMed) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,PubMed)
        }
    }
    if { [ info exists ::QA(url,JNeurosci) ] } {    
        if { $::QA(url,JNeurosci) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,JNeurosci)
        }
    }
    if { [ info exists ::QA(url,IBVD) ] } {
        if { $::QA(url,IBVD) != "" } {
            $::slicer3::QueryAtlasGUI AppendUniqueResult $::QA(url,IBVD)
        }
    }
}






#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMakeEntrezCountQuery { url } {

    package require http

    puts "making count query........."
     set ::QA(entrezCountFileName) "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/tmp/QA.counts"
    set ::QA(entrezCountFile) [ open $::QA(entrezCountFileName) w+ ]
    set token [ ::http::geturl $url -channel $::QA(entrezCountFile) ]
    close $::QA(entrezCountFile)

    
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetAllEntrezCounts { } {

    #--- TODO: do some error checking here.

    set ret 0
    
    if {  [ info exists ::QA(entrezCountFileName) ] } {
        set ::QA(entrezCountFile) [ open $::QA(entrezCountFileName) r ]
        set numResults [QueryAtlasCountNumberOfResults ]
        close $::QA(entrezCountFile)
        
        if { $numResults > 0 } {
            set ::QA(entrezCountFile) [ open $::QA(entrezCountFileName) r ]
            set ret [ QueryAtlasParseAllEntrezCounts ]
            close $::QA(entrezCountFile)
            if { $ret != $numResults } {
                puts "QueryAtlasGetAllEntrezCounts: Parse error: Retrieved $tst of $numResults results"
            }
        }

    }
    return $ret
}


#----------------------------------------------------------------------------------------------------
#--- vtkXMLDataParser can't pull out content; use tcl for now.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseAllEntrezCounts {} {

    #--- get the search term 
    set ::QA(entrezQueryString) [ QueryAtlasExtractContentWithTcl "<Term>" "</Term>" ]
    unset -nocomplain ::QA(entrezDatabaseFriendlyNames)
    unset -nocomplain ::QA(entrezDatabaseNames)
    unset -nocomplain ::QA(entrezDatabaseCounts)
    set ::QA(numEntrezDatabases) 0

    #--- get the query results: just go till eof
    set i 0
    if { $::QA(entrezQueryString) != "" } {
        while { ![eof $::QA(entrezCountFile)] } {
           gets $::QA(entrezCountFile) line2
            if { $line2 != "" } {
                set index [ string first "<ResultItem>" $line2 ]
                if { $index >= 0 } {
                    #--- Pull out data
                    incr i

                    set db [QueryAtlasExtractContentWithTcl "<DbName>" "</DbName>" ]
                    puts ".......getting counts for $db"
                    set fname [ QueryAtlasExtractContentWithTcl "<MenuName>" "</MenuName>"]
                    set count [ QueryAtlasExtractContentWithTcl "<Count>" "</Count>" ]
                    set status [ QueryAtlasExtractContentWithTcl "<Status>" "</Status>" ]

                    if { $count > 0 } {
                        lappend ::QA(entrezDatabaseNames) $db
                        lappend ::QA(entrezDatabaseFriendlyNames) $fname
                        lappend ::QA(entrezDatabaseCounts) $count
                        incr ::QA(numEntrezDatabases)
                    }
                }
            }
        }
    }
    return $i
}



#----------------------------------------------------------------------------------------------------
#--- counts how many results reported in the file.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCountNumberOfResults { } {
    
    set i 0
    while { ![eof $::QA(entrezCountFile)] } {
        gets $::QA(entrezCountFile) line
        set index [ string first "<ResultItem>" $line ]
        if { $index >= 0 } {
            incr i
        }
    }
    return $i
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasExtractContentWithTcl { open_tag close_tag  } {

    set ok 0
    while { ![eof $::QA(entrezCountFile)] } {
        gets $::QA(entrezCountFile) line
        set o_taglen [ string len $open_tag ]
        set index [ string first $open_tag $line ]

        if { $index > 0 } {
            #-- strip out the open tag
            set index [ expr $index + $o_taglen ]
            set line [ string range $line $index end ]
            #-- strip out the close tag
            set index [ string first $close_tag $line ]
            set index [ expr $index - 1 ]
            set line [ string range $line 0 $index ]
            set ::QA(entrezSearchTerm) $line
            return $line
            break
        }
    }
    return ""
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTestFan { } {

    if { 0 } {
        #--- get the render widget and set the icon directory
        set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
        set icondir $::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/ImageData

        #--- create  a list of icon names
        set icons [lrange [glob $icondir/*.png] 0 4]

        #-- create a cardfan with some number of cards
        set cardFan [CardFan #auto [llength $icons]]

       #--- for each icon in icons and associated card in the cardfan
       #--- set the card's icon and text (fix the text)                 
       foreach icon $icons card [$cardFan cards] {
           $card configure -icon $icon -text [file tail $icon]
       }

       #--- and configure the cardfan's spacing, scale, startpoint and radius
       $cardFan configure -spacing 15 -anchor [list 0 0 0] -radius 50
       $cardFan configureAll -scale 10 -follow 0
       }
  }


