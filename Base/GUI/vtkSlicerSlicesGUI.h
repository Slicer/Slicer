// .NAME vtkSlicerSlicesGUI 
// .SECTION Description
// Main Slice GUI and mediator functions for slicer3.  
// Manages a collection of individual sliceGUIs, each of which
// contain a SliceViewer a SliceController, a pointer to
// SliceLogic and a pointer to a MRMLSliceNode.

#ifndef __vtkSlicerSlicesGUI_h
#define __vtkSlicerSlicesGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkMRMLSliceNode.h"

//BTX
class SliceGUIMap;
//ETX

class vtkObject;
class vtkKWFrame;

// Description:
// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerSlicesGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSlicesGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

  // Description:
  // Add a Slice GUI with its layoutname to the SliceGUIMap
  virtual void AddSliceGUI(char *layoutName, vtkSlicerSliceGUI *pSliceGUI);
  virtual void AddAndObserveSliceGUI(char *layoutName, vtkSlicerSliceGUI *pSliceGUI);
  
  // Description:
  // Return a pointer to the SliceGUIMap
  // SliceGUIMap* GetSliceGUIMap() { return InternalSliceGUIMap; }

  // Description:
  // Return a particular slice gui by name
  //BTX
  vtkSlicerSliceGUI* GetSliceGUI(char *layoutName);
  //ETX
  
  // Description:
  // The following functions are used to provide an iterator to SliceGUIMap for looping through its items
  int GetNumberOfSliceGUI();
  vtkSlicerSliceGUI* GetFirstSliceGUI ();
  vtkSlicerSliceGUI* GetNextSliceGUI(char *layoutName);
  char* GetFirstSliceGUILayoutName();
  char* GetNextSliceGUILayoutName(char *layoutName);
  
  // Description:
  // Build the SlicesGUI's UIPanel and three main SliceGUIs 
  virtual void BuildGUI ( );

  // Description:
  // Add/Remove Observers on UIPanel widgets and SliceGUIs.
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  virtual void RemoveGUIMapObservers();
    
  // Description:
  // Processes all events raised by the logic
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  // Description:
  // Processes all events raised by the GUI
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  // Description:
  // Processes all events raised by MRML
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // Functions that define and undefine module-specific behaviors.
  virtual void Enter ( );
  virtual void Exit ( );
    
 protected:
  vtkSlicerSlicesGUI ( );
  virtual ~vtkSlicerSlicesGUI ( );

 private:
  // Description:
  // An STL::MAP of the slice GUIs
  //BTX
  SliceGUIMap *InternalSliceGUIMap;
  //ETX

  vtkSlicerSlicesGUI ( const vtkSlicerSlicesGUI& ); // Not implemented.
  void operator = ( const vtkSlicerSlicesGUI& ); //Not implemented.
}; 

#endif
