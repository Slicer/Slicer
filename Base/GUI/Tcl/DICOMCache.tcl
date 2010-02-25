package require Itcl

#########################################################
#
if {0} { ;# comment

  DICOMCache is a helper class for LoadVolume.  It manages
  a cache of parsed directory information for easy reload
  and selection.

# TODO : 

}
#
#########################################################

# 
# utilities 
# - to get/set contents of a file
# - generate a simple hash for a string
#
namespace eval DICOMCache {

  proc cat { fileName {contents ""} } {
    # no passed data, so return current contents
    if { $contents == "" } {
      if { ![file exists $fileName] } {
        return ""
      }
      set fp [open $fileName "r"]
      set contents [read $fp]
      close $fp
      return $contents
    }
    # with passed data, write it to the file
    set fp [open $fileName "w"]
    puts -nonewline $fp $contents
    close $fp
  }

  # a "good enough" hash function for generating unique file names"
  proc hash { s } {
    set hash 0
    set len [string length $s]
    for {set i 0} {$i < $len} {incr i} {
      set c [string index $s $i]
      scan $c "%c" ci
      set hash [expr ($hash + ($i * $ci)) % 999999]
    }
    return $hash
  }

}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class DICOMCache] == "" } {

  itcl::class DICOMCache {

    constructor  {} {
    }

    destructor {
    }

    public variable cacheDirectory "" ;# where the catalog array file is stored

    variable _catalog ;# the mapping between directory names and dicom tree information

    # methods
    method fillCatalog {} {}
    method saveCatalog {} {}
    method getTreeFileName { dirName } {}
    method getTreeForDirectory { dirName treeName {findAncestor 0} } {}
    method setTreeForDirectory { dirName treeName } {}
    method getDirectories {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body DICOMCache::constructor {} {

  array unset _catalog
  array set _catalog ""

  if { [info exists ::slicer3::Application] } {
    $this configure -cacheDirectory [$::slicer3::Application GetTemporaryDirectory]
  }
}


itcl::body DICOMCache::destructor {} {
}


#
# get the current catalog array from cache
#
itcl::body DICOMCache::fillCatalog {} {
  array unset _catalog
  array set _catalog [DICOMCache::cat $cacheDirectory/DICOMCatalog]
}

#
# save catalog to cache file
#
itcl::body DICOMCache::saveCatalog {} {
  DICOMCache::cat $cacheDirectory/DICOMCatalog [array get _catalog]
}

#
# get a filename that can be used to encode the directory name
# - needs to be unique and small enough to be a legal 
#   file name on any systems we use.
# - make it using the last element of the path together
#   with a hash of the full directory name
# - the hash isn't assured to be unique, but since 
#   the full path is stored in the file, it can be doublechecked
#
itcl::body DICOMCache::getTreeFileName { dirName } {
  return "$cacheDirectory/[file tail $dirName]-[DICOMCache::hash $dirName]"
}

#
# save the tree for a given directory in the cache and 
# add a reference to it in the catalog
#
itcl::body DICOMCache::setTreeForDirectory { dirName treeName } {

  upvar $treeName tree
  $this fillCatalog
  set fileName [$this getTreeFileName $dirName]
  set _catalog($dirName) $fileName
  DICOMCache::cat $fileName [array get tree]
  $this saveCatalog
}

#
# given a directory name, fill in the tree information from the 
# cache if it exists
# - look for an ancestor directory that exists in the cache
#   and use it (but only if it includes this dirName in it's parse info)
#
itcl::body DICOMCache::getTreeForDirectory { dirName treeName {findAncestor 0} } {

  upvar $treeName tree
  array unset tree
  array set tree ""
  $this fillCatalog

  if { $findAncestor } {
    set ancestorDir ""
    foreach dirPart [file split $dirName] {
      set ancestorDir [file join $ancestorDir $dirPart]
      set testFileName [$this getTreeFileName $ancestorDir]
      if { [info exists _catalog($ancestorDir)] } {
        array set testTree [DICOMCache::cat $_catalog($ancestorDir)]
        foreach fileList [array names testTree *files] {
          set filedir [file dirname [lindex $testTree($fileList) 0]]
          if { $filedir == $dirName } {
            # here we found an ancestor directory that has been parsed and includes
            # information about the directory that is requested - so return this one
            set dirName $ancestorDir
            break
          }
        }
      }
      if { $dirName == $ancestorDir } {
        break
      }
    }
  }
  
  set fileName [$this getTreeFileName $dirName]
  if { [info exists _catalog($dirName)] } {
    array set tree [DICOMCache::cat $_catalog($dirName)]
  }
  if { [info exists tree(directoryName)] && $tree(directoryName) != $dirName } {
    puts "warning: ignoring duplicate directory entry for $dirName and $tree(directoryName)"
    array unset tree
    array set tree ""
  }
}

#
# return a list of directories currently in the cache
#
itcl::body DICOMCache::getDirectories {} {

  $this fillCatalog
  return [array names _catalog]
}
