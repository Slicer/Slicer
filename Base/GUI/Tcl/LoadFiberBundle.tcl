
#------------------------------------------------------------------------------
# utility to load fiber bundles thru the data module.
#------------------------------------------------------------------------------
proc LoadFiberBundle { fileName  } {

    if { $fileName == "" } {
        return
    }

    set app $::slicer3::Application
    set fiberBundleGUI [$app GetModuleGUIByName "FiberBundles" ]

    set dataGUI [ $app GetModuleGUIByName "Data" ]
    
    if { $fiberBundleGUI != "" } {
        set fiberBundleLogic [ $fiberBundleGUI GetLogic ]
        if { $fiberBundleLogic != "" } {
            set fiberBundleNode [ $fiberBundleLogic AddFiberBundle $fileName ]
            if {$fiberBundleNode == "" } {
                set dialog [vtkKWMessageDialog New]
                $dialog SetParent [ $dataGUI GetUIPanel [ GetPageWidget "Data"] ]
                $dialog SetStyleToMessage
                $dialog SetText "Unable to read DTI fiber bundle model file $fileName"
                $dialog Create
                $dialog Invoke
                $dialog Delete
            }
        }
    }
}






