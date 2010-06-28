///  vtkSlicerSlicesGUI 
/// 
/// Main Slice GUI and mediator functions for slicer3.  
/// Manages a collection of individual sliceGUIs, each of which
/// contain a SliceViewer a SliceController, a pointer to
/// SliceLogic and a pointer to a MRMLSliceNode.

#ifndef __vtkSlicerSlicesGUI_h
#define __vtkSlicerSlicesGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSmartPointer.h"

//BTX
class SliceGUIMap;
class ParameterWidgetMap;
//ETX

class vtkObject;
class vtkKWFrame;
class vtkSlicerVisibilityIcons;
class vtkSlicerSliceControllerWidget;

/// Description:
/// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerSlicesGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSlicesGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

  /// 
  /// Add a Slice GUI with its layoutname to the SliceGUIMap
  virtual void AddSliceGUI(const char *layoutName, vtkSlicerSliceGUI *pSliceGUI);
  virtual void AddAndObserveSliceGUI(const char *layoutName, vtkSlicerSliceGUI *pSliceGUI);
  
  /// 
  /// Return a pointer to the SliceGUIMap
  /// SliceGUIMap* GetSliceGUIMap() { return InternalSliceGUIMap; }

  /// 
  /// Return a particular slice gui by name
  vtkSlicerSliceGUI* GetSliceGUI(const char *layoutName);

  /// 
  /// The current slice node being operated on
  vtkGetObjectMacro(SliceNode, vtkMRMLSliceNode);
  vtkSetObjectMacro(SliceNode, vtkMRMLSliceNode);
  
  /// 
  /// The following functions are used to provide an iterator to SliceGUIMap for looping through its items
  int GetNumberOfSliceGUI();
  vtkSlicerSliceGUI* GetFirstSliceGUI ();
  vtkSlicerSliceGUI* GetNextSliceGUI(const char *previouslayoutName);
  const char* GetFirstSliceGUILayoutName();
  const char* GetNextSliceGUILayoutName(const char *previouslayoutName);

  /// 
  /// More traditional VTK style access methods.  Uses a linear time
  /// walk of the data structures. Use the GetFirst/GetNext methods if
  /// there are a lot of slice guis
  vtkSlicerSliceGUI* GetNthSliceGUI(int n);
  const char *GetNthSliceGUILayoutName(int n);
  
  /// 
  /// Build the SlicesGUI's UIPanel and three main SliceGUIs 
  virtual void BuildGUI ( void );
  virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
  virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

  /// 
  /// Add/Remove Observers on UIPanel widgets and SliceGUIs.
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  virtual void RemoveGUIMapObservers();
    
  /// 
  /// Processes all events raised by the logic
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  /// 
  /// Processes all events raised by the GUI
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  /// 
  /// Processes all events raised by MRML
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// Functions that define and undefine module-specific behaviors.
  virtual void Enter ( );
  virtual void Enter ( vtkMRMLNode* node ) { this->Superclass::Enter(node); }
  virtual void Exit ( );
    
 protected:
  vtkSlicerSlicesGUI ( );
  virtual ~vtkSlicerSlicesGUI ( );

  void UpdateGUI();

  /// Set up a slice controller and pack it in the panel
  vtkSlicerSliceControllerWidget* BuildSliceController(vtkSlicerSliceGUI *);
  
 private:
  /// 
  /// An STL::MAP of the slice GUIs
  //BTX
  SliceGUIMap *InternalSliceGUIMap;
  ParameterWidgetMap *InternalParameterWidgetMap;
  //vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
  //ETX
  vtkMRMLSliceNode *SliceNode;

  vtkKWLabel *NACLabel;
  vtkKWLabel *NAMICLabel;
  vtkKWLabel *NCIGTLabel;
  vtkKWLabel *BIRNLabel;
  
  vtkSlicerVisibilityIcons *VisibilityIcons;
  
  vtkSlicerSlicesGUI ( const vtkSlicerSlicesGUI& ); /// Not implemented.
  void operator = ( const vtkSlicerSlicesGUI& ); //Not implemented.
}; 

#endif
