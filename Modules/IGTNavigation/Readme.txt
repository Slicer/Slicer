Main classes

IGTGUI : derived from SlicerModuleGUI

- Handles the GUI of the module
- Contains BuildGUI method that bascially instantiates the wizard widget and connect the
  steps

- Access the MRMLScene from Logic 
- Has ivars of Logic, MRMLManager and node

IGTMRMLManager : manages the scene...

IGTLogic : contains all the logic ( algorithm related ) nothing GUI
- contains ivar of MRMLManager

Wizard/
-Takes care of all the wizard steps


MRML/
-Takes care of all the MRML nodes

