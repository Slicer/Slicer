// .NAME vtkSlicerViewerGUI
// .SECTION Description
// Main Data GUI and mediator methods for slicer3.

#ifndef __vtkSlicerViewerGUI_h
#define __vtkSlicerViewerGUI_h

#include "vtkKWRenderWidget.h"

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerWindow.h"

//#include "vtkSlicerViewerLogic.h"

// Description:
// This class implements Slicer's Viewer GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewerGUI : public vtkSlicerComponentGUI
{
public:
  // Description:
  // Usual vtk class functions
  static vtkSlicerViewerGUI* New (  );
  vtkTypeRevisionMacro ( vtkSlicerViewerGUI, vtkSlicerComponentGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  vtkGetObjectMacro(MainViewer, vtkKWRenderWidget);
  vtkSetObjectMacro(MainViewer, vtkKWRenderWidget);
  
  vtkGetObjectMacro(MainSlicerWindow, vtkSlicerWindow);
  vtkSetObjectMacro(MainSlicerWindow, vtkSlicerWindow);
  
  // Description:
  // This method builds the Viewer module's GUI
  virtual void BuildGUI ( ) ;
  
  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  
  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // Describe the behavior at module enter and exit.
  virtual void Enter ( );
  virtual void Exit ( );
  
protected:
  vtkSlicerViewerGUI ( );
  virtual ~vtkSlicerViewerGUI ( );
  
  void UpdateFromMRML();
  
  vtkSlicerWindow *MainSlicerWindow;
  vtkKWRenderWidget *MainViewer;
  
private:
  vtkSlicerViewerGUI ( const vtkSlicerViewerGUI& ); // Not implemented.
  void operator = ( const vtkSlicerViewerGUI& ); //Not implemented.
};


#endif
