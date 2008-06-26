// .NAME vtkSlicerWindow
// .SECTION Description
// Slicer specific window type.  Primary purpose is to implement
// GetApplicationSettingsInterface() to return a Slicer specific
// ApplicationSettingsInterface. 
//

#ifndef _vtkSlicerWindow_h
#define _vtkSlicerWindow_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkKWWindow.h"
#include "vtkKWMenu.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerWindow : public vtkKWWindow
{
public:
  static vtkSlicerWindow* New();
  vtkTypeRevisionMacro(vtkSlicerWindow,vtkKWWindow);

  vtkGetObjectMacro ( FeedbackMenu, vtkKWMenu );
  // Description:
  // Get the Application Settings Interface as well as the Application
  // Settings User Interface Manager.
  virtual vtkKWApplicationSettingsInterface *GetApplicationSettingsInterface();

protected:
  vtkSlicerWindow();
  virtual ~vtkSlicerWindow();

  // Description:
  // Menu with options for users to report feedback
  vtkKWMenu *FeedbackMenu;

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  virtual void UpdateMenuState();

  vtkSlicerWindow(const vtkSlicerWindow&); // Not implemented
  void operator=(const vtkSlicerWindow&); // Not implemented

  
};

#endif
