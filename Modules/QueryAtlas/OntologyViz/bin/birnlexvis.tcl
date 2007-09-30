
proc OntologyVizLaunch {ontdir usesemis} {
    set progbase "birnlexvis"
    set bindir [file join $ontdir "bin"]
    set datadir [file join $ontdir "data"]
    set extjardir [file join $ontdir "extjars"]
    
    set cpath [list "$bindir/$progbase-support.jar" \
                   "$bindir/$progbase.jar" "$extjardir/json.jar" \
                   "$extjardir/prefuse.jar" "$extjardir/jython.jar"]

    if {$usesemis} {
        set cpath [join "$cpath" \;]
    } else {
        set cpath [join "$cpath" :]
    }
    return [exec java -cp "$cpath" "$progbase" "$datadir/*.json"]

}
