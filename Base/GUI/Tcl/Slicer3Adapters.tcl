
#
# this is a file of adapter tcl classes that fill the role of 
# slicer3 classes.
# These provide access to vtk
# instances and related helper functionality so that legacy slicer3 code
# can operate within the slicer4 context.  Code using this includes the SWidget
# classes and the editer module 
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

    # create instances of the application level adapter classes
    set ::slicer3::Application [namespace current]::[::Slicer3Adapters::Application #auto]
    set ::slicer3::ApplicationGUI [namespace current]::[::Slicer3Adapters::ApplicationGUI #auto]
    $::slicer3::ApplicationGUI SetApplication $::slicer3::Application

  }
}

package require Itcl
package require vtk


#
# The partent class definition - define if needed (not when re-sourcing)
#
namespace eval Slicer3Adapters {

  itcl::class SliceGUI {
    # this is a replacement for a vtkSlicerSliceGUI from slicer3

    constructor {} {
      set _sliceViewer [namespace current]::[::Slicer3Adapters::SliceViewer #auto]
    }

    destructor {
    }

    # parts of the sliceGUI 
    variable _logic ""
    variable _sliceViewer ""
    variable _interactorStyle ""
    variable _slicerWindow ""
    variable _grabID ""
    variable _activeLeftButtonTool ""

    # simple container methods
    method SetLogic {arg} {
      set _logic $arg
      $this SetMRMLScene [$_logic GetMRMLScene]
      $_sliceViewer SetSliceLogic $_logic
    }
    method GetLogic {} {return $_logic}
    method SetGrabID {arg} {set _grabID $arg}
    method GetGrabID {} {return $_grabID}
    method SetMRMLScene {arg} { set ::slicer3::MRMLScene $arg}
    method GetMRMLScene {} { return $::slicer3::MRMLScene }
    method GetSliceNode {} { return [$_logic GetSliceNode]}

    #TODO: DeleteEvent Observers on the sliceGUI are /probably/ not 
    # needed since other parts of the infrastructure, like slice nodes,
    # are already being observed and will also go away.  Also, most (all?)
    # SWidgets self destruct cleanly if the sliceGUI instance is gone
    method AddObserver {event script} {
      # nothing
      return "0"
    }
    method RemoveObserver {tag} {
      # nothing
    }

    #TODO: will need to adapt this to slicer4's version of mouse 'ownership'
    method GetActiveLeftButtonTool {} { 
      return $_activeLeftButtonTool
    }
    method SetActiveLeftButtonTool {arg} { 
      set _activeLeftButtonTool $arg 
    }

    method IsA {arg} {
      # only admit to being a slice gui (not a vtk object)
      if { $arg == "vtkSlicerSliceGUI" } {
        return 1
      } else {
        return 0
      }
    }

    # stub out this call - not needed if grabID is set correctly
    method SetGUICommandAbortFlag {arg} {}

    # access to locally instanced classes
    method GetSliceViewer {} {
      return $_sliceViewer
    }

    # these methods push state down to internal class instances
    method SetInteractorStyle {arg} {
      set _interactorStyle $arg
      [$_sliceViewer GetRenderWidget] SetRenderWindowInteractor [$_interactorStyle GetInteractor]
    }
    method GetInteractorStyle {} {return $_interactorStyle}

    method SetCornerAnnotation {arg} {
      [$_sliceViewer GetRenderWidget] SetCornerAnnotation $arg
    }
    method GetCornerAnnotation {} {
      return [$_sliceViewer GetCornerAnnotation]
    }

    method AddObservation {event script} {
      # mimic the behavior of slicer3 where sliceGUI is a vtk object
      # - observe the interactor style
      # -- need to give AddObservation the python name of the interactor style
      set layoutName [[$this GetSliceNode] GetLayoutName]
      set pyname [format "slicer.sliceWidget%s_interactorStyle" $layoutName]
      #set script "$this SetGrabID {}; $script"
      py_eval [format "slicer.broker.AddObservation(%s,'%s','%s')" $pyname $event $script]
    }

    method GetCurrentGUIEvent {} {
      puts "TODO: GetCurrentGUIEvent is deprecated - include event in observer command directly"
    }
    method SetCurrentGUIEvent {args} {
      # do nothing
    }
  }

  itcl::class Application {
    # this is a replacement for a vtkSlicerApplication from slicer3

    constructor {} {
    }

    destructor {
    }

    # parts of the application

    # methods
    method GetModuleGUIByName {args} {
      #TODO: will scripted modules be able to access other modules in slicer4
      return ""
    }
    method ProcessPendingEvents {args} {
      py_eval "slicer.app.processEvents()"
    }
  }

  itcl::class ApplicationGUI {
    # this is a replacement for a vtkSlicerApplicationGUI from slicer3

    constructor {} {
      set _slicerWindow [namespace current]::[::Slicer3Adapters::SlicerWindow #auto]
      set _viewerWidget [namespace current]::[::Slicer3Adapters::ViewerWidget #auto]
    }

    destructor {
    }

    # parts of the application
    variable _slicerWindow ""
    variable _viewerWidget ""
    variable _application ""

    # methods
    # access to locally instanced classes
    method SetApplication {arg} {set _application $arg}
    method GetApplication {} {return $_application}
    method GetMainSlicerWindow {} {
      return $_slicerWindow
    }
    method GetActiveViewerWidget {} {
      if { [py_eval "slicer.app.layoutManager() == None"] == "True" } {
        return ""
      }
      return $_viewerWidget
    }

    method GetGUILayoutNode {} {
      return [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLLayoutNode"]
    }
  }

  itcl::class ViewerWidget {
    # this is a replacement for a vtkSlicerViewerWidget from slicer3

    constructor {} {
      set _renderWidget [namespace current]::[::Slicer3Adapters::RenderWidget #auto]
    }

    destructor {
    }

    # place holder variable
    variable _renderWidget ""

    # methods
    method GetMainViewer {} {
      if { [py_eval "slicer.app.layoutManager()"] != "None" } {
        if { [py_eval "slicer.app.layoutManager().threeDWidget(0)"] != "None" } {
          $_renderWidget SetRenderWindowInteractor [py_eval "slicer.app.layoutManager().threeDWidget(0).threeDView().renderWindow().GetInteractor()"]
        }
      }
      return $_renderWidget
    }
    method RequestRender {} {
      if { [$this GetMainViewer] != "" } {
        $_renderWidget RequestRender
      }
    }
  }

  itcl::class SliceViewer {
    # this is a replacement for a vtkSlicerSliceViewer from slicer3

    constructor {} {
      set _renderWidget [namespace current]::[::Slicer3Adapters::RenderWidget #auto]
    }

    destructor {
    }

    variable _renderWidget ""
    variable _sliceLogic ""

    # methods
    method GetRenderWidget {} {return $_renderWidget}
    method SetSliceLogic {arg} {set _sliceLogic $arg}
    method GetSliceLogic {} {return $_sliceLogic}
    method RequestRender {} {
      if { [py_eval "slicer.app.layoutManager() == None"] == "True" } {
        return ""
      }
      set layoutName [[$_sliceLogic GetSliceNode] GetLayoutName]
      if { [py_eval "slicer.app.layoutManager().sliceWidget('$layoutName') == None"] == "True" } {
        return ""
      }
      py_eval "slicer.app.layoutManager().sliceWidget('$layoutName').sliceView().scheduleRender()"
    }
    method Render {} {
      if { [py_eval "slicer.app.layoutManager() == None"] == "True" } {
        return ""
      }
      set layoutName [[$_sliceLogic GetSliceNode] GetLayoutName]
      if { [py_eval "slicer.app.layoutManager().sliceWidget('$layoutName') == None"] == "True" } {
        return ""
      }
      py_eval "slicer.app.layoutManager().sliceWidget('$layoutName').sliceView().forceRender()"
    }
    method GetHighlightColor {} {
      # TODO: need to figure out how these resources map to Qt
      return "1 1 0"
    }
  }

  itcl::class SlicerWindow {
    # this is a replacement for a vtkSlicerWindow from slicer3
    #
    variable _progressGauge ""

    constructor {} {
      set _progressGauge [namespace current]::[::Slicer3Adapters::ProgressGauge #auto]
    }

    destructor {
    }

    # methods
    method SetStatusText {arg} {
      py_eval [format "slicer.util.showStatusMessage('%s')" $arg]
    }
    method GetProgressGauge {} {
      return $_progressGauge
    }
  }

  # special purpose replacement for Tk winfo command
  proc ::winfo {option mocktkwidget} {
    switch $option {
      "width" {
        return [lindex [$mocktkwidget GetSize] 0]
      }
      "height" {
        return [lindex [$mocktkwidget GetSize] 1]
      }
      default {
        error "unknown option $option - should be width or height"
      }
    }
  }

  itcl::class RenderWidget {
    # this is a replacement for a vtkKWRenderWidget from kwwidgets

    constructor {} {
      set _cornerAnnotation [namespace current]::[::Slicer3Adapters::CornerAnnotation #auto]
      set _vtkWidget [namespace current]::[::Slicer3Adapters::VTKWidget #auto]

    }

    destructor {
    }

    # parts of the application
    variable _interactor ""
    variable _cornerAnnotation ""
    variable _vtkWidget ""

    # methods
    method GetWidgetName {} {
      # return mock tk widget for use with ::winfo proc 
      return $this
    }
    method GetSize {} {
      return [[$_interactor GetRenderWindow] GetSize]
    }
    method SetRenderWindowInteractor {arg} {set _interactor $arg}
    method GetRenderWindowInteractor {} {return $_interactor}
    method SetCornerAnnotation {arg} {set _cornerAnnotation $arg}
    method GetCornerAnnotation {} {return $_cornerAnnotation}
    method CornerAnnotationVisibilityOff {} {
      $_cornerAnnotation VisibilityOff
    }
    method CornerAnnotationVisibilityOn {} {
      $_cornerAnnotation VisibilityOn
    }
    method GetVTKWidget {} {return $_vtkWidget}
    method GetRenderWindow {} {
      return [$_interactor GetRenderWindow]
    }
    method RequestRender {} {
      # TODO: make this a request rather than a force render
      [$this GetRenderWindow] Render
    }
    method GetRenderer {} {
      return [[[$_interactor GetRenderWindow] GetRenderers] GetFirstRenderer]
    }
    method GetNumberOfRenderers {} {
      return [[[$_interactor GetRenderWindow] GetRenderers] GetNumberOfItems]
    }
    method GetNthRenderer {n} {
      return [[[$_interactor GetRenderWindow] GetRenderers] GetItemAsObject $n]
    }
    method RemoveAllRenderers {} {
      [[$_interactor GetRenderWindow] GetRenderers] RemoveAllItems
    }
  }

  itcl::class CornerAnnotation {
    # this is a stand-in for a vtkCornerAnnotation from vtk
    #  - the class still exists, but is not accessible from wrapping currently

    constructor {} {
      set _textProperty [vtkTextProperty New]
    }

    destructor {
    }

    # parts of the application
    variable _visibility 1
    variable _textProperty ""

    # methods
    method VisibilityOn {} {set _visibility 1}
    method VisibilityOff {} {set _visibility 0}
    method SetVisibility {arg} {set _visibility $arg}
    method GetVisibility {} {return $_visibility}
    method GetTextProperty {} {return $_textProperty}
    method SetText {corner text} {
      #puts "TODO: SetText $corner $text"
      #puts -nonewline "t";flush stdout
    }
  }

  itcl::class ProgressGauge {
    # this is a stand-in for a vtkKWProgressDialog from kwwidgets

    constructor {} {
    }

    destructor {
    }

    # parts of the application
    variable _value 0

    # methods
    method SetValue {value} {
      set _value $value
      puts "Progress: $_value"
    }
  }

  itcl::class vtkKWMessageDialog {
    # this is a stand-in for a vtkKWMessageDialog from kwwidgets

    constructor {} {
    }

    destructor {
    }

    # parts of the application
    variable _value 0

    # methods
    method SetParent {value} {}
    method SetMasterWindow {value} {}
    method SetStyleToMessage {} {}
    method SetText {value} {
      puts "Message: $value"
    }
    method Create {} {}
    method Invoke {} {}
    method Delete {} { itcl::delete $this }
  }

  itcl::class VTKWidget {
    # this is a stand-in for a kw wrapped tk widget

    constructor {} {
    }

    destructor {
    }

    # parts of the application

    # methods
    method AddBinding {args} {
      # silently ignore
      #puts "TODO: ignoring AddBinding $args"
    }
  }

  itcl::class vtkMRMLScriptedModuleNode {
    # this is a stand-in for a mrml node

    constructor {} {
    }

    destructor {
    }

    # parts of the application

    # methods
    method AddBinding {args} {puts "TODO: ignoring AddBinding $args"}
  }
}

# a handy utility, copied from tkcon.tcl
## lremove - remove items from a list
# OPTS:
#   -all        remove all instances of each item
#   -glob        remove all instances matching glob pattern
#   -regexp        remove all instances matching regexp pattern
# ARGS:        l        a list to remove items from
#        args        items to remove (these are 'join'ed together)
##
proc lremove {args} {
    array set opts {-all 0 pattern -exact}
    while {[string match -* [lindex $args 0]]} {
        switch -glob -- [lindex $args 0] {
            -a*        { set opts(-all) 1 }
            -g*        { set opts(pattern) -glob }
            -r*        { set opts(pattern) -regexp }
            --        { set args [lreplace $args 0 0]; break }
            default {return -code error "unknown option \"[lindex $args 0]\""}
        }
        set args [lreplace $args 0 0]
    }
    set l [lindex $args 0]
    foreach i [join [lreplace $args 0 0]] {
        if {[set ix [lsearch $opts(pattern) $l $i]] == -1} continue
        set l [lreplace $l $ix $ix]
        if {$opts(-all)} {
            while {[set ix [lsearch $opts(pattern) $l $i]] != -1} {
                set l [lreplace $l $ix $ix]
            }
        }
    }
    return $l
}

