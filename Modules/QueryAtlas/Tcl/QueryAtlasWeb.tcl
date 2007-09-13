
#----------------------------------------------------------------------------------------------------
proc QueryAtlasWriteFirefoxBookmarkFile { } {
}

#----------------------------------------------------------------------------------------------------
proc QueryAtlasLoadFirefoxBookmarkFile { } {

}

#----------------------------------------------------------------------------------------------------
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSetStructureTerm { } {
    [ $::slicer3::QueryAtlasGUI GetLocalSearchTermEntry ] SetValue $::QA(localLabel) 
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasRemoveStructureTerms {} {

  $::slicer3::ApplicationGUI SelectModule QueryAtlas
  set mcl [[$::slicer3::QueryAtlasGUI GetStructureMultiColumnList] GetWidget]

  $mcl DeleteAllRows
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetStructureTerms {} {

#  $::slicer3::ApplicationGUI SelectModule QueryAtlas
#  set mcl [[$::slicer3::QueryAtlasGUI GetStructureMultiColumnList] GetWidget]

#  set terms ""
#  set n [$mcl GetNumberOfRows]
#  for {set i 0} {$i < $n} {incr i} {
#    # TODO: figure out the mcl checkbutton access
#    if { 1 || [$mcl GetCellTextAsInt $i 0] } {
#      set term [$mcl GetCellText $i 1]
#        if { ![string match "edit*" $term] && ($term != "") } {
#              set terms "$terms+[$mcl GetCellText $i 1]"
#        }
#    }
#  }
#  set terms [ string trimleft $terms "+"]
#  return $terms
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSubStructureTerms {} {

  $::slicer3::ApplicationGUI SelectModule QueryAtlas
  set mcl [[$::slicer3::QueryAtlasGUI GetCellMultiColumnList] GetWidget]

  set terms ""
  set n [$mcl GetNumberOfRows]
  for {set i 0} {$i < $n} {incr i} {
    # TODO: figure out the mcl checkbutton access
    if { 1 || [$mcl GetCellTextAsInt $i 0] } {
      set term [$mcl GetCellText $i 1]
      if { ![string match "edit*" $term] } {
        set terms "$terms+[$mcl GetCellText $i 1]"
      }
    }
  }
  set terms [ string trimleft $terms "+"]
  return $terms
}





#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetDiagnosisTerms { } {
    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set terms ""
    set mb [[$::slicer3::QueryAtlasGUI GetDiagnosisMenuButton] GetWidget]
    set terms [ $mb GetValue ]
    return $terms
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetGenderTerms { } {
    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set terms ""
    set mb [[$::slicer3::QueryAtlasGUI GetGenderMenuButton] GetWidget]
    set terms [ $mb GetValue ]
    return $terms
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSpeciesTerms { } {
    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set terms ""
    set cb_h [$::slicer3::QueryAtlasGUI GetSpeciesHumanButton]
    set cb_ms [$::slicer3::QueryAtlasGUI GetSpeciesMouseButton]
    set cb_mq [$::slicer3::QueryAtlasGUI GetSpeciesMacaqueButton]
    
    set human [ $cb_h GetSelectedState ]
    set mouse [ $cb_ms GetSelectedState ]
    set macaque [ $cb_mq GetSelectedState ]

    if { $human } {
        set terms "human"
    }
    if { $mouse } {
        set terms "$terms+mouse"
    }
    if { $macaque } {
        set terms "$terms+macaque"
    }
    return $terms
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
proc QueryAtlasAddSavedTerms {} {

    #--- add term to listbox.
  $::slicer3::ApplicationGUI SelectModule QueryAtlas
  set mcl [[[$::slicer3::QueryAtlasGUI GetSavedTerms] GetMultiColumnList] GetWidget]

  set i [$mcl GetNumberOfRows]
  $mcl SetCellText [ expr $i - 1 ] 0 $::QA(lastLabels)
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasAddStructureTerms {} {

    #--- add term to listbox.
  $::slicer3::ApplicationGUI SelectModule QueryAtlas
  set mcl [[[$::slicer3::QueryAtlasGUI GetSavedTerms] GetMultiColumnList] GetWidget]

  set i [$mcl GetNumberOfRows]
  $::slicer3::QueryAtlasGUI AddNewStructureSearchTerm $::QA(lastLabels)
  $mcl SetCellTextAsInt $i 0 1
  $mcl SetCellText $i 1 $::QA(lastLabels)

  #--- set hierarchy entry with most recently selected term
  set h_entry [$::slicer3::QueryAtlasGUI GetHierarchySearchTermEntry]
  $h_entry SetValue $::QA(lastLabels)
  
}


#----------------------------------------------------------------------------------------------------
#--- 
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSelectTermSet { termset } {


}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasQuery { site } {

    if { $::QA(lastLabels) == "background" || $::QA(lastLabels) == "Unknown" } {
        set terms ""
    } else {
        set terms $::QA(lastLabels)
    }

    if { $::QA(menu,useTerms) } {
        set terms "$terms+[QueryAtlasGetStructureTerms]"
    }

    regsub -all "/" $terms "+" terms
    regsub -all -- "-" $terms "+" terms
    regsub -all "ctx" $terms "cortex" terms
    regsub -all "rh" $terms "right+hemisphere" terms
    regsub -all "lh" $terms "left+hemisphere" terms

    switch $site {
        "google" {
            $::slicer3::Application OpenLink http://www.google.com/search?q=$terms
        }
        "wikipedia" {
            $::slicer3::Application OpenLink http://www.google.com/search?q=$terms+site:en.wikipedia.org
        }
        "pubmed" {
            $::slicer3::Application OpenLink \
                http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=search&db=PubMed&term=$terms
        }
        "jneurosci" {
            $::slicer3::Application OpenLink \
                http://www.jneurosci.org/cgi/search?volume=&firstpage=&sendit=Search&author1=&author2=&titleabstract=&fulltext=$terms
        }
        "braininfo" {
        }
        "ibvd" {
            regsub -all "Left\+" $terms "" terms ;# TODO ivbd has a different way of handling side
            regsub -all "left\+" $terms "" terms ;# TODO ivbd has a different way of handling side
            regsub -all "Right\+" $terms "" terms ;# TODO ivbd has a different way of handling side
            regsub -all "right\+" $terms "" terms ;# TODO ivbd has a different way of handling side
            regsub -all "\\+" $terms "," commaterms

            if { 0 } {
                set terms "human,normal,$commaterms"
                set url http://www.cma.mgh.harvard.edu/ibvd/search.php?f_submission=true&f_free=$commaterms
            } else {
                #--- this gets us the plot for a diagnosis context.
                #--- set url http://www.cma.mgh.harvard.edu/ibvd/how_big.php?structure=$terms&diagnosis=$dterms
                set url http://www.cma.mgh.harvard.edu/ibvd/how_big.php?structure=$terms
                
            }
            $::slicer3::Application OpenLink $url
        }
        "metasearch" {
            $::slicer3::Application OpenLink \
                https://loci.ucsd.edu/qametasearch/query.do?query=$terms
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForGoogle_Testing { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    unset -nocomplain ::QA(url,Google) 
    set url "http://www.google.com/search?hl-en&q="
    #--- for google, put each term in each list in quotes.

    #--- structure first how many "+" are there?
    set multiTerms [ string first "+" $structure ]
    if { !($multiTerms < 0) } {
        set tmp ""
        set newStructure ""
        set start 0
        set i [ string first "+" $structure ]
        while { $i > 0 } {
            set tmp [ string range $structure $start $i ]
            set newStructure "$newStructure+\"$tmp\""
            set start $i
            set i [ string first "+" $structure ]
        }
        #--- remove the leading "+"
        set newStructure [ string range $newStructure 1 end ]
    } else {
        set newStructure $structure
    }
    #--- cell next
    set multiTerms [ string first "+" $cells ]
    if { !($multiTerms < 0) } {
        set tmp ""
        set newCells ""
        set start 0
        set i [ string first "+" $cells ]
        while { $i > 0 } {
            set tmp [ string range $cells $start $i ]
            set newCells "$newCells+\"$tmp\""
            set start $i
            set i [ string first "+" $cells ]
        }
        #--- remove the leading "+"
        set newCells [ string range $newCells 1 end ]
    } else {
        set newCells $cells
    }
    #--- genes next
    set multiTerms [ string first "+" $genes ]
    if { !($multiTerms < 0) } {
        set tmp ""
        set newGenes ""
        set start 0
        set i [ string first "+" $genes ]
        while { $i > 0 } {
            set tmp [ string range $genes $start $i ]
            set newGenes "$newGenes+\"$tmp\""
            set start $i
            set i [ string first "+" $genes ]
        }
        #--- remove the leading "+"
        set newGenes [ string range $newGenes 1 end ]
    } else {
        set newGenes $genes
    }
    #--- misc next
    set multiTerms [ string first "+" $misc ]
    if { !($multiTerms < 0) } {
        set tmp ""
        set newMisc ""
        set start 0
        set i [ string first "+" $misc ]
        while { $i > 0 } {
            set tmp [ string range $misc $start $i ]
            set newMisc "$newMisc+\"$tmp\""
            set start $i
            set i [ string first "+" $misc ]
        }
        #--- remove the leading "+"
        set newMisc [ string range $newMisc 1 end ]
    } else {
        set newMisc $misc
    }
    #--- make something reasonable for gender
    if {$gender =="M" } {
        set newGender "male"
    } elseif {$gender == "F" } {
        set newGender "female"
    } elseif {$gender == "mixed" } {
        set newGender "\"mixed gender\""
    } elseif {$gender == "n/a" } {
        set newGender ""
    }
    append url "$newStructure+$newMisc+$newCells+$newGenes"
    if { $newGender != "" } {
        append url "+$newGender"
    }

    #--- make something reasonable for diagnosis
    if { $diagnosis != "normal" } {
        append url "+\"$diagnosis\""
    }
    append url "&btnG=Google+Search"
    set ::QA(url,Google) $url
}

proc QueryAtlasFormURLForGoogle { } {
    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    set ::QA(url,Google) ""
    set url "http://www.google.com/search?hl-en&q="
    #--- get only the first structure from the structurelist
    set i [ string first "+" $structure ]
    if { $i > 0 } {
        #--- found a "+", so grab all chars up to it
        set singleStructure [ string range $structure 0 [ expr $i - 1 ] ]
    } else {
        set singleStructure $structure
    }
    append url "$singleStructure"
    append url "&btnG=Google+Search"
    set ::QA(url,Google) $url
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForWikipedia { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    set ::QA(url,Wikipedia) ""
    set url "http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search="
    #--- get only the first structure from the structurelist
    set i [ string first "+" $structure ]
    if { $i > 0 } {
        #--- found a "+", so grab all chars up to it
        set singleStructure [ string range $structure 0 [ expr $i - 1 ] ]
    } else {
        set singleStructure $structure
    }
    append url "$singleStructure"
#    if { $diagnosis != "normal" } {
#        append url "+$diagnosis"
#    }
#    if { $gender != "n/a" } {
#        append url "+$gender"
#    }
    append url "\&fulltext=Search"
    set ::QA(url,Wikipedia) $url
    
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForIBVD { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    unset  -nocomplain ::QA(url,IBVD) 
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

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForPubMed { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    #--- TODO:
    #--- pubmed wants multi word thing + otherthing to look like
    #-- "multi+word+thing"+otherthing
    unset -nocomplain ::QA(url,PubMed)
    set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=search&db=PubMed&term=$structure"
    set ::QA(url,PubMed) $url
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForJNeurosci { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    #--- TODO:
    #--- here, we want to put double quotes around each separate
    #--- structure, cell, gene and misc term.
    #--- jneurosci wants multi word thing + otherthing to look like
    #--- "multi word thing"+otherthing
    unset -nocomplain ::QA(url,JNeurosci) 
    set url "http://www.jneurosci.org/cgi/search?volume=&firstpage=&sendit=Search&author1=&author2=&titleabstract=&fulltext=$structure"
    set ::QA(url,JNeurosci) $url
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForMetasearch { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    unset -nocomplain ::QA(url,Metasearch) 
    set url "https://loci.ucsd.edu/qametasearch/query.do?query=$structure"
    set ::QA(url,Metasearch) $url
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForBraininfo { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    #--- TODO: just take first structure term.
    set i [ string first "+" $structure ]
    if { $i >= 0 } {
        set structure [ string range $structure 0 [expr $i - 1 ] ]
    } 
    unset -nocomplain ::QA(url,Braininfo) 
    set url "http://braininfo.rprc.washington.edu/Scripts/hiercentraldirectory.aspx?ID=$structure"
    set ::QA(url,Braininfo) $url
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLsForEntrez { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]

    unset -nocomplain ::QA(url,EntrezCounts ) 
    unset -nocomplain ::QA(url,EntrezLinks ) ""

    set url "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/egquery.fcgi?term="
    append url "$structure+$diagnosis"
    set ::QA(url,EntrezCounts) $url

    QueryAtlasMakeEntrezCountQuery $::QA(url,EntrezCounts)
    set counts [ QueryAtlasGetAllEntrezCounts ]
    puts "$counts"

    #--- sometimes entrez uses the database name from
    #--- inside one array, and sometimes it uses the other.
    #--- We'll have to just learn these special cases and
    #--- watch for them for now.. Use this to see the difference
    #    puts "$::QA(entrezDatabaseFriendlyNames)"
    #    puts "---"
    #    puts "$::QA(entrezDatabaseNames)"
    #--- ENTREZ
    #--- if counts was good, form urls for all dbs with info
    set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?"
    unset -nocomplain ::QA(url,EntrezLinks)
    if { $counts > 0 } {
        if { [info exists ::QA(entrezDatabaseFriendlyNames) ]  && [info exists ::QA(entrezDatabaseCounts)]} {
            set len [ llength $::QA(entrezDatabaseFriendlyNames) ]
            for { set i 0 } { $i < $len } { incr i } {
                set tmp $url
                set dbname [ lindex $::QA(entrezDatabaseFriendlyNames) $i ]
                #--- fix some inconsistencies with entrez
                switch -glob $dbname {
                    "PMC" {
                        set dbname "pmc"
                    }
                    "OMIA" {
                        set dbname "omia"
                    }
                    "HomoloGene" {
                        set dbname "homologene"
                    }
                }
                append tmp "db=$dbname\&cmd=search\&term=$structure+$diagnosis"
                
                #--- if gender is other than 'n/a'
                #if {$gender =="M" } {
                 #   set newGender "male"
                #} elseif {$gender == "F" } {
                #    set newGender "female"
                #} elseif {$gender == "mixed" } {
                #    set newGender "\"mixed gender\""
                #} elseif {$gender == "n/a" } {
                #    set newGender ""
                #}
                #append tmp "+$newGender"

                #--- if species is other than 'human', use it
                #if { $species != "human" } {
                #    append tmp "+$species"
                #}

                puts "Link for $dbname: $tmp"
                lappend ::QA(url,EntrezLinks) $tmp
            }
        }
    }

    #--- build and display the fan of links in 3DViewer
    set len [ llength $::QA(entrezDatabaseNames) ]
    if { $len > 0 } {
#        QueryAtlasBuildLinkFan
    }
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLsForTargets { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetSubStructureTerms ]
    set target [ QueryAtlasGetSearchTargets ]
    
    #--- Form urls for each Search Target requested.
    #--- PubMed
    if { ($target == "PubMed") } {
        QueryAtlasFormURLForPubMed
        puts "$::QA(url,PubMed)"
    }
    #--- J Neurosci
    if { ($target == "JNeurosci") || ($target == "all") } {
        QueryAtlasFormURLForJNeurosci
        puts "$::QA(url,JNeurosci)"
    }
    #--- metasearch
    if { ($target == "Metasearch") || ($target == "all") } {
        QueryAtlasFormURLForMetasearch
        puts "$::QA(url,Metasearch)"
    }
    #--- IBVD
    if { ($target == "all") || ($target == "IBVD") } {
        QueryAtlasFormURLForIBVD
        puts "$::QA(url,IBVD)"
    }
    #--- GOOGLE
    if { ($target == "all") || ($target == "Google") } {
        QueryAtlasFormURLForGoogle
        puts "$::QA(url,Google)"
    }
    #--- WIKIPEDIA
    if { ($target == "all") || ($target == "Wikipedia") } {
        QueryAtlasFormURLForWikipedia
        puts "$::QA(url,Wikipedia)"
    }
    #--- ENTREZCOUNTS
    if { ($target == "all") || ($target == "Entrez") } {
        QueryAtlasFormURLsForEntrez
        puts "$::QA(url,EntrezLinks)"
    }
    QueryAtlasPopulateSearchResultsBox 
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPopulateSearchResultsBox { } {

    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set lb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]    

    if { [ info exists ::QA(url,Google) ] } {
        if { $::QA(url,Google) != "" } {
            $lb AppendUnique $::QA(url,Google)
        }
    }
    if { [ info exists ::QA(url,Wikipedia) ] } {    
        if { $::QA(url,Wikipedia) != "" } {
            $lb AppendUnique $::QA(url,Wikipedia)
        }
    }
    if { [ info exists ::QA(url,IBVD) ] } {
        if { $::QA(url,IBVD) != "" } {
            $lb AppendUnique $::QA(url,IBVD)
        }
    }
    if { [ info exists ::QA(url,EntrezLinks) ] } {
        if { $::QA(url,EntrezLinks) != "" } {
            set len  [llength $::QA(url,EntrezLinks) ]
            for { set i 0 } { $i < $len } { incr i } {
                set url [ lindex $::QA(url,EntrezLinks) $i ]
                $lb AppendUnique $url
            }
        }
    }
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasOpenLink { } {

    set lb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]
    set index [ $lb GetSelectionIndex ]
    if { $index >= 0 } {
        set name [ $lb GetItem $index ]
        # is this a url?
        set tst [ string first "http" $name ]
        if { $tst >= 0 } {
            $::slicer3::Application OpenLink $name
            $lb SetSelectState $index 0
        }
    }
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasBundleSearchResults { } {

    incr ::QA(linkBundleCount) 
    set i $::QA(linkBundleCount)
    unset -nocomplain ::QA($i,linkBundle)
    
    set clb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]    
    set num [ $clb GetNumberOfItems ]
    for { set j 0 } { $j < $num } { incr j } {
        set url [ $clb GetItem $j ]
        lappend ::QA($i,linkBundle) $url
    }
    set plb [[$::slicer3::QueryAtlasGUI GetPastResultsList ] GetWidget]        
    $plb AppendUnique "$i: Reserved Link Bundle"
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasSaveLinkBundlesToFile { } {

    set ::QA(linkBundleFileName) "$::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/tmp/QA.links"
    puts "saving to $::QA(linkBundleFileName)..."
    set ::QA(linkBundleFile) [ open $::QA(linkBundleFileName) w+ ]
    #--- expand each bundle in the list and write them line by line to file

    set plb [[$::slicer3::QueryAtlasGUI GetPastResultsList ] GetWidget]
    set num [ $plb GetNumberOfItems ]
    for { set j 0 } { $j < $num } { incr j } {    
        #--- get each bundle
        set bundle [ $plb GetItem $j ]
        set index [ string first ":" $bundle ]
        set b [ string range $bundle 0 [ expr $index - 1]]

        #--- find length of each bundle
        set len [ llength $::QA($b,linkBundle) ]
        for { set i 0 } { $i < $len } { incr i } {
            set url [ lindex $::QA($b,linkBundle) $i ]
            puts $::QA(linkBundleFile) $url
        }
    }
    close $::QA(linkBundleFile)
    puts "...done."
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
proc QueryAtlasBuildLinkFan { } {

    #-- get rid of the last round of links
    puts "deleting link fan"
    QueryAtlasDeleteLinkFan
    
    #--- get the render widget and set the icon directory
    set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set renderer [ $renderWidget GetRenderer ]
    puts "got renderer"
    
    set icondir $::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/ImageData
    puts "icondir = $icondir"

    #--- create  a list of icon names
    set iconset [glob $icondir/*.png]
    puts "iconset = $iconset"

    #--- use only the icons for which there is a matching databasename
    set icons ""
    foreach name $::QA(entrezDatabaseNames) {
        #-- search iconset for name
        foreach icon $iconset {
            set index [ string first $name.png $icon]
            if { $index >= 0 } {
                lappend icons $icon
                break
            }
        }
    }
    
    #-- create a cardfan with some number of cards
    if { [ info exists ::QA(entrezDatabaseNames) ] } {
    set len [ llength $::QA(entrezDatabaseNames)] 

        if { $len > 0 } {
            set ::QA(cardFan) [CardFan #auto [llength $::QA(entrezDatabaseNames)]]
            puts "created cardfan"            

            puts "using $::QA(entrezDatabaseNames)"
            #--- for each icon in icons and associated card in the cardfan
            #--- set the card's icon and text (fix the text)                 
            foreach icon $icons card [$::QA(cardFan) cards] name $::QA(entrezDatabaseNames) {
                $card configure -icon $icon -text $name
            }
            puts "configured cardfan"

            #--- get the scene's bounding box...
            set bounds [ $renderer ComputeVisiblePropBounds ]
            #--- get the max dimension of the bbox.
            set xmin [ lindex $bounds 0 ]
            set xmax [ lindex $bounds 1 ]
            set ymin [ lindex $bounds 2 ]
            set ymax [ lindex $bounds 3 ]
            set zmin [ lindex $bounds 4 ]
            set zmax [ lindex $bounds 5 ]
            set diagx [ expr ($xmax - $xmin) ]
            set diagy [ expr ($ymax - $ymin) ]
            set diagz [ expr ($zmax - $zmin) ]

            set diagx [ expr $diagx * $diagx ]
            set diagy [ expr $diagy * $diagy ]
            set diagz [ expr $diagz * $diagz ]

            set radius [ expr $diagx + $diagy + $diagz ]

            #-- what if we have just a single point in scene?
            if {  $radius == 0 } {
                set radius 1.0
            } 
            set radius [ expr (sqrt ($radius) * 0.5 ) ]
            
            #--- add a little extra bump
            set radius [ expr $radius + ( $radius * 0.2) ]
            puts "radius = $radius"
            puts "anchor = $::QA(cardRASAnchor)"
            #--- and configure the cardfan's spacing, scale, startpoint and radius
            $::QA(cardFan) configure -spacing 15 -anchor $::QA(cardRASAnchor) -radius $radius
            $::QA(cardFan) configureAll -scale 10 -follow 0
            puts "done."
        }
    }
}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasDeleteLinkFan { } {

    if { [ info exists ::QA(cardFan) ] } {
        itcl::delete object $::QA(cardFan)
        unset -nocomplain ::QA(cardFan)
    } 
}




#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasTestFan { } {

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


