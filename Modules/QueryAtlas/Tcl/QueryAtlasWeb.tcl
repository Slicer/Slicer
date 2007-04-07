

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLForGoogle_Testing { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormURLsForTargets { } {

    #--- get things from GUI:
    set structure [QueryAtlasGetStructureTerms ]
    set diagnosis [QueryAtlasGetDiagnosisTerms ]
    set gender [QueryAtlasGetGenderTerms ]
    set misc [QueryAtlasGetMiscTerms ]
    set species [QueryAtlasGetSpeciesTerms ]
    set cells [QueryAtlasGetHistologyTerms ]
    set genes [ QueryAtlasGetGeneTerms ]
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

    set ::QA(linkBundleFileName) "$::env(SLICER_HOME)/Modules/QueryAtlas/tmp/QA.links"
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
     set ::QA(entrezCountFileName) "$::env(SLICER_HOME)/Modules/QueryAtlas/tmp/QA.counts"
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



