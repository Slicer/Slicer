#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasPopulateSearchResultsBox { } {

    $::slicer3::ApplicationGUI SelectModule QueryAtlas
    set lb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]    

    lb AppendUnique $::QA(url,Google) 
    lb AppendUnique $::QA(url,Wikipedia)
    lb AppendUnique $::QA(url,IBVD)
    set len  [llength $::QA(url,EntrezLinks) ]
    for { set i 0 } { $i < $len } { incr i } {
        set url [ lindex $::QA(url,EntrezLinks) $i ]
        lb AppendUnique $url
    }

}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasBundleSearchResults { } {
    $::slicer3::ApplicationGUI SelectModule QueryAtlas

    incr ::QA(linkBundleCount) 
    set i $::QA(linkBundleCount)
    set ::QA($i,linkBundle) ""
    
    set clb [[$::slicer3::QueryAtlasGUI GetCurrentResultsList ] GetWidget]    
    set num [ clb GetNumberOfItems ]
    for { set j 0 } { $j < $num } { incr j } {
        set url [ clb GetItem $j ]
        append ::QA($i,linkBundle) $url
    }
    set plb [[$::slicer3::QueryAtlasGUI GetPastResultsList ] GetWidget]        
    plb AppendUnique "$i Reserved Link Bundle"
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
    set species [QueryAtlasGetSpeciesTerms]
    
    set ::QA(url,Google) ""
    set ::QA(url,Wikipedia) ""
    set ::QA(url,IBVD) ""
    set ::QA(url,EntrezLinks) ""
    
    #--- IBVD
    set url "http://www.cma.mgh.harvard.edu/ibvd/how_big.php?"
    # break this down for all demographics that ibvd allow and we offer
    # structure from menu...
    append url "structure=$structure"
    # diagnosis from menu
    append url "\&diagnosis=$diagnosis"
    # gender from menu
    if { $gender != "n/a" } {
        append url "\&gender=$gender"
    }
    set ::QA(url,IBVD) $url
    puts "$::QA(url,IBVD)"

    #--- GOOGLE
    set url "http://www.google.com/search?hl-en&q="
    append url "$structure+$misc+gender $gender"
    if { $diagnosis != "normal" } {
        append url "+$diagnosis"
    }
    append url "&btnG=Google+Search"
    set ::QA(url,Google) $url
    puts "$::QA(url,Google)"

    #--- WIKIPEDIA
    set url "http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search="
    append url "$structure"
    if { $diagnosis != "normal" } {
        append url "+$diagnosis"
    }
    if { $gender != "n/a" } {
        append url "+$gender"
    }
    append url "\&fulltext=Search"
    set ::QA(url,Wikipedia) $url
    puts "$::QA(url,Wikipedia)"

    #--- ENTREZCOUNTS
    set url "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/egquery.fcgi?term="
    append url "$structure+$diagnosis"
    set ::QA(url,EntrezCounts) $url
    puts "$::QA(url,EntrezCounts)"

    QueryAtlasMakeEntrezCountQuery $::QA(url,EntrezCounts)
    set counts [ QueryAtlasGetAllEntrezCounts ]

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
    set ::QA(url,EntrezLinks) ""
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
                if { $gender != "" } {
                    append tmp "+$gender"
                }
                #--- if species is other than 'human', use it
                if { $species != "human" } {
                    append tmp "+$species"
                }
                puts "Link for $dbname: $tmp"
                lappend ::QA(url,EntrezLinks) $tmp
            }
        }
    }
    QueryAtlasPopulateSearchResultsBox 
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



