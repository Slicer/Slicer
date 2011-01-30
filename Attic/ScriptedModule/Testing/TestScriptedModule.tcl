
proc ScriptedModulesCreate {} {

  #
  # process any ScriptedModules
  # - scan for pkgIndex.tcl files 
  # - load the corresponding packages
  # - create scripted logic and scripted gui instances for the module
  # - have the GUI construct itself
  #

  # TODO: this code should be updated to match what's in Slicer3.cxx, i.e.
  # not just look inside $::Slicer_HOME/lib/Slicer3/Modules/, but
  # iterate over all the vtkSlicerApplication::GetModulePaths (since
  # $::Slicer_HOME/lib/Slicer3/Modules/ is already there, alongside any
  # user modules)

  set ::Slicer_PACKAGES(list) {}
  set dirs [glob $::Slicer_HOME/lib/Slicer3/Modules/*] 
  foreach d $dirs { 
    if { [file exists $d/Tcl/pkgIndex.tcl] } {
      lappend ::Slicer_PACKAGES(list) [file tail $d]
      lappend ::auto_path $d
    }
  } 

  foreach package $::Slicer_PACKAGES(list) { 
    package require $package

    # Logic:
    set ::Slicer_PACKAGES($package,logic) [vtkScriptedModuleLogic New]
    set logic $::Slicer_PACKAGES($package,logic)
    $logic SetModuleName $package
    $logic SetAndObserveMRMLScene $::slicer3::MRMLScene
    $logic SetApplicationLogic $::slicer3::ApplicationLogic

    # GUI:
    set ::Slicer_PACKAGES($package,gui) [vtkScriptedModuleGUI New]
    set gui $::Slicer_PACKAGES($package,gui)
    $gui SetModuleName $package
    $gui SetLogic $logic
    $gui SetApplication $::slicer3::Application
    $gui SetGUIName $package
    [$gui GetUIPanel] SetName $package
    [$gui GetUIPanel] SetUserInterfaceManager [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetMainUserInterfaceManager]
    [$gui GetUIPanel] Create
    $::slicer3::Application AddModuleGUI $gui
    $gui BuildGUI
    $gui AddGUIObservers
  
  }
  $::slicer3::ApplicationGUI PopulateModuleChooseList
}

proc ScriptedModulesRemove {} {

  # remove the observers from the scripted modules
  foreach package $::Slicer_PACKAGES(list) { 
    set gui $::Slicer_PACKAGES($package,gui)
    $::slicer3::Application RemoveModuleGUI $gui
    $gui SetLogic ""
    $::Slicer_PACKAGES($package,gui) RemoveGUIObservers
    $::Slicer_PACKAGES($package,gui) TearDownGUI
  }
  
  # delete the GUI
  foreach package $::Slicer_PACKAGES(list) { 
    $::Slicer_PACKAGES($package,gui) Delete
  }
  
  # delete the scripted logics
  foreach package $::Slicer_PACKAGES(list) { 
    $::Slicer_PACKAGES($package,logic) SetAndObserveMRMLScene {}
    $::Slicer_PACKAGES($package,logic) Delete
  }
  $::slicer3::ApplicationGUI PopulateModuleChooseList
}
