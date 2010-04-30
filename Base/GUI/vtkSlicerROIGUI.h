///  vtkSlicerROIGUI 
/// 
/// Main ROI GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerROIGUI_h
#define __vtkSlicerROIGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerROILogic.h"
#include "vtkMRMLROINode.h"


#include "vtkKWFrame.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkKWRange.h"
#include "vtkKWExtent.h"

class vtkSlicerROIDisplayWidget;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerROIGUI : public vtkSlicerModuleGUI
{
public:
  ///  
  /// Usual vtk class functions 
  static vtkSlicerROIGUI* New (  );
  vtkTypeRevisionMacro ( vtkSlicerROIGUI, vtkSlicerModuleGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );

  void SetModuleLogic ( vtkSlicerROILogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
  //BTX
  using vtkSlicerModuleGUI::SetModuleLogic;
  //ETX
  void SetAndObserveModuleLogic ( vtkSlicerROILogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

  /// 
  /// This method builds the ROI module GUI
  virtual void BuildGUI ( void );
  virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
  virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

  /// 
  /// This method releases references and key-bindings,
  /// and optionally removes observers.
  virtual void TearDownGUI ( );

  /// 
  /// Methods for adding module-specific key bindings and
  /// removing them.
  virtual void CreateModuleEventBindings ( );
  virtual void ReleaseModuleEventBindings ( );

  /// 
  /// Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  /// 
  /// Class's mediator methods for processing events invoked by
  /// either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// Methods describe behavior at module enter and exit.
  virtual void Enter ( );
  //BTX
  using vtkSlicerComponentGUI::Enter;
  //ETX
  virtual void Exit ( );

  /// 
  /// Update the gui from the currently selected ROI node, called on Enter
  void UpdateGUI();


protected:
  vtkSlicerROIGUI ( );
  virtual ~vtkSlicerROIGUI ( );

  /// 
  /// Which ROI  node are we displaying in this gui 
  vtkSlicerNodeSelectorWidget* ROISelectorWidget;

 /// Description:
  /// Module logic and mrml pointers
  vtkSlicerROILogic *Logic;


  /// ROI position and Raidus scale
  vtkSlicerROIDisplayWidget *ROIDisplayWidget;


private:
  vtkSlicerROIGUI ( const vtkSlicerROIGUI& ); /// Not implemented.
  void operator = ( const vtkSlicerROIGUI& ); //Not implemented.
};
#endif
