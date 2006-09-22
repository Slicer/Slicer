
proc ScriptedModulesCreate {} {

  #
  # process any ScriptedModules
  # - scan for pkgIndex.tcl files 
  # - load the corresponding packages
  # - create scripted logic and scripted gui instances for the module
  # - have the GUI construct itself
  #

  set ::SLICER_PACKAGES(list) {}
  set dirs [glob $::SLICER_BUILD/Modules/Packages/*] 
  foreach d $dirs { 
    if { [file exists $d/pkgIndex.tcl] } {
      lappend ::SLICER_PACKAGES(list) [file tail $d]
      lappend ::auto_path $d
    }
  } 

  foreach package $::SLICER_PACKAGES(list) { 
    package require $package

    # Logic:
    set ::SLICER_PACKAGES($package,logic) [vtkScriptedModuleLogic New]
    set logic $::SLICER_PACKAGES($package,logic)
    $logic SetModuleName $package
    $logic SetAndObserveMRMLScene $::slicer3::MRMLScene
    $logic SetApplicationLogic $::slicer3::ApplicationLogic

    # GUI:
    set ::SLICER_PACKAGES($package,gui) [vtkScriptedModuleGUI New]
    set gui $::SLICER_PACKAGES($package,gui)
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
  foreach package $::SLICER_PACKAGES(list) { 
    set gui $::SLICER_PACKAGES($package,gui)
    $::slicer3::Application RemoveModuleGUI $gui
    $gui SetLogic ""
    $::SLICER_PACKAGES($package,gui) RemoveGUIObservers
    $::SLICER_PACKAGES($package,gui) TearDownGUI
  }
  
  # delete the GUI
  foreach package $::SLICER_PACKAGES(list) { 
    $::SLICER_PACKAGES($package,gui) Delete
  }
  
  # delete the scripted logics
  foreach package $::SLICER_PACKAGES(list) { 
    $::SLICER_PACKAGES($package,logic) SetAndObserveMRMLScene {}
    $::SLICER_PACKAGES($package,logic) Delete
  }
  $::slicer3::ApplicationGUI PopulateModuleChooseList
}
