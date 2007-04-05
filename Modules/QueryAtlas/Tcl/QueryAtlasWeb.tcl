#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormIBVDPlotURL { searchList } {
    
    set url "http://www.cma.mgh.harvard.edu/ibvd/how_big.php?"
    #--- break this down for all demographics that ibvd allow and we offer
    #--- structure from menu...
    set structuretxt "structure=gettermsfromlabel"
    append url $structuretext
    #--- diagnosis from menu
    set diagnosistxt "diagnosis=gettermsfromlabel"
    append url "&$diagnosistxt"
    #--- gender from menu
    set gendertxt "gender=getgenderfromlabel"
    append url "&$gendertxt"
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormGoogleQuery { searchList } {
    set url "http://www.google.com/search?hl-en&q="
    append url $searchList
    append url "&btnG=Google+Search"

}

#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormWikipediQuery { searchList  } {
    #--- need to only use structure here.
    set url "http://en.wikipedia.org/wiki/"
    append url $searchList
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasFormEntrezCountURL { searchList } {
    
    set url "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/egquery.fcgi?term="
    append url $searchList

    set ::QA(entrezCountURL) $url
}



#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasMakeEntrezCountQuery { url } {

    package require http
    
    set ::QA(entrezCountFileName) "C:/cygwin/tmp/QA.counts"
    set ::QA(entrezCountFile) [ open $::QA(entrezCountFileName) w+ ]
    set token [ ::http::geturl $url -channel $::QA(entrezCountFile) -tool "Slicer3" ]
    close $::QA(entrezCountFile)

}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetAllEntrezCounts { } {

    #--- TODO: do some error checking here.
    unset -nocomplain ::QA(entrezDatabaseFriendlyNames)
    unset -nocomplain ::QA(entrezDatabaseNames)
    unset -nocomplain ::QA(entrezDatabaseCounts)
    set ::QA(numEntrezDatabases) 0

    if {  [ info exists ::QA(entrezCountFileName) ] } {
        set ::QA(entrezCountFile) [ open $::QA(entrezCountFileName) r ]
        set numResults [QueryAtlasCountNumberOfResults ]
        if { $numResults > 0 } {
           QueryAtlasParseAllEntrezCounts
        }
        close $::QA(entrezCountFile)
    }

}


#----------------------------------------------------------------------------------------------------
#--- vtkXMLDataParser can't pull out content; use tcl for now.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasParseAllEntrezCounts {} {

    #--- get the search term 
    set ::QA(entrezQueryString) [ QueryAtlasExtractContentWithTcl "<Term>" "</Term>" ]

    #--- get the query results: just go till eof
    if { $::QA(entrezQueryString) != "" } {
        set i 0
        while { ![eof $::QA(entrezCountFile)] } {
            gets $::QA(entrezCountFile) line
            set tag "<ResultItem>"
            set zz [ string first $tag $line ]
            if { $zz > 0 } {
                #--- Pull out data
                gets $::QA(entrezCountFile) line
                set db [QueryAtlasExtractContentWithTcl "<DbName>" "</DbName>" ]
                gets $::QA(entrezCountFile) line
                set fname [ QueryAtlasExtractContentWithTcl "<MenuName>" "</MenuName>"]
                gets $::QA(entrezCountFile) line                        
                set count [ QueryAtlasExtractContentWithTcl "<Count>" "</Count>" ]
                gets $::QA(entrezCountFile) line
                set status [ QueryAtlasExtractContentWithTcl "<Status>" "</Status>" ]
                if { $count > 0 } {
                    lappend ::QA(entrezDatabaseNames) $db
                    lappend ::QA(entrezDatabaseFriendlyNames) $fname
                    lappend ::QA(entrezdatabaseCounts) $count
                    incr ::QA(numEntrezDatabases)
                }
            }
        }
    }
}



#----------------------------------------------------------------------------------------------------
#--- counts how many results reported in the file.
#----------------------------------------------------------------------------------------------------
proc QueryAtlasCountNumberOfResults { } {
    
    set i 0
    while { ![eof $::QA(entrezCountFile)] } {
        gets $::QA(entrezCountFile) line
        set index [ string first "<ResultItem>" $line ]
        if { $index > 0 } {
            incr i
        }
    }

    #--- go back to beginning
    seek $::QA(entrezCountFile) 0 start
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
proc QueryAtlasCreateEntrezDatabaseUrls {searchList } {

    unset -nocomplain ::QA(entrezIndividualURLs)
    for { set i 0 } { $i < $::QA(numEntrezDatabases) } { incr i } {
        set dbname [ lindex $::QA(entrezDatabaseNames) $i  ]
        set url "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?db="
        append url $dbname
        append url "&"
        append url $searchList
        lappend ::QA(entrezIndividualURLs) $url
    }
    
    lappend ::QA(entrezdatabaseCounts) $count    
    
}


#----------------------------------------------------------------------------------------------------
#---
#----------------------------------------------------------------------------------------------------
proc QueryAtlasGetSingleEntrezDatabaseUrls { dbname } {
}
