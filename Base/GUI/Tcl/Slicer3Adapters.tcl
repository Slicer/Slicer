
#
# this is a file of adapter tcl classes that fill the role of 
# slicer3 classes.
# These provide access to vtk
# instances and related helper functionality so that legacy slicer3 code
# can operate within the slicer4 context. Code using is EMSegment.
#
# code in this file is mostly just a place to organize access to 
# underlying slicer vtk instances (logic and mrml) but it also
# is allowed to be 'aware' of the python interpreter and make direct
# calls using the 'py' command that is added with to the tcl
# interp as part of the python-in-tcl (tpycl) layer that underlies
# this interpreter
#

# 
# initialize the ::slicer3 namespace with pre-defined classes
#
#
namespace eval Slicer3Adapters {
  proc Initialize {} {
    if { [info exists ::slicer3::Broker] } {
      return
    }

    # global variable used in some scripts and tests
    set ::SlicerHome $::env(SLICER_HOME)

    # bring python wrapped slicer vtk classes into the tcl namespace
    py_package slicer

    # expose the event Broker singleton as a tcl proc
    # and set the variable to point to that proc
    namespace eval ::slicer3 {}
    proc ::slicer3::BrokerProc {args} {
      set method [lindex $args 0]
      switch $method {
        "AddObservation" {
          set subject [lindex $args 1]
          if { [catch "$subject GetMTime"] } {
            # call AddObservation directly on subject
            # (rely on only wrapped vtkObjects having a GetMTime method)
            set event [lindex $args 2]
            set script [lindex $args 3]
            $subject AddObservation $event $script
            return
          }
          set event [lindex $args 2]
          if { [string is int $event] } {
            # TODO: this makes python treat this as a string but AddObservation will understand it as an int
            set event "${event}XXX"
          }
          set script [lindex $args 3]
          ::tpycl::methodCaller slicer.broker slicer.broker [list $method $subject $event $script]
        }
        default {
          ::tpycl::methodCaller slicer.broker slicer.broker $args
        }
      }
    }
    set ::slicer3::Broker ::slicer3::BrokerProc

    # expose the application logic singleton as a tcl proc
    # and set the variable to point to that proc
    proc ::slicer3::ApplicationLogicProc {args} {
      ::tpycl::methodCaller ::slicer3::ApplicationLogicProc slicer.app.applicationLogic() $args
    }
    set ::slicer3::ApplicationLogic ::slicer3::ApplicationLogicProc

    # expose the volumes logic as a tcl proc
    # and set the variable to point to that proc
    proc ::slicer3::VolumesLogicProc {args} {
      ::tpycl::methodCaller ::slicer3::VolumesLogicProc slicer.modules.volumes.logic() $args
    }
    set ::slicer3::VolumesLogic ::slicer3::VolumesLogicProc
  }
}
