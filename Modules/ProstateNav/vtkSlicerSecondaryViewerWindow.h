#ifndef __vtkSlicerSecondaryViewerWindow_h
#define __vtkSlicerSecondaryViewerWindow_h

#include "vtkProstateNavWin32Header.h"

// for getting display device information
#ifdef WIN32
#include "Windows.h"
#endif

#include "vtkSmartPointer.h"

#include "vtkKWTopLevel.h"
#include "vtkSmartPointer.h"

class vtkSlicerViewerWidget;
class vtkSlicerFiducialListWidget;
class vtkSlicerROIViewerWidget;
class vtkSlicerROIViewerWidget;
class vtkMRMLViewNode;

class vtkKWFrame;
class vtkSlicerApplicationGUI;

class VTK_PROSTATENAV_EXPORT  vtkSlicerSecondaryViewerWindow : 
  public vtkKWTopLevel
{
public:
  static vtkSlicerSecondaryViewerWindow *New();  
  vtkTypeRevisionMacro(vtkSlicerSecondaryViewerWindow,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // If a secondary monitor is available, then the window is displayed on that, maximized.
  // If a secondary monitor cannot be detected then the window is displayed as a simple top-level window that the user
  // can move around. Current limitation: monitor detection works only on Windows.
  void DisplayOnSecondaryMonitor();

  vtkKWFrame* GetMainFrame() { return MainFrame; };
  vtkSlicerViewerWidget* GetViewerWidget () { return this->ViewerWidget; };

  void Destroy();

protected:
  
  virtual void CreateWidget();

  void UpdateSecondaryMonitorPosition();  

  vtkSlicerApplicationGUI* GetApplicationGUI();

  vtkSlicerSecondaryViewerWindow();
  ~vtkSlicerSecondaryViewerWindow();  

  vtkKWFrame* MainFrame;

  bool MultipleMonitorsAvailable; 
  int  SecondaryMonitorPosition[2]; // virtual screen position in pixels
  int  SecondaryMonitorSize[2]; // virtual screen size in pixels

  vtkSlicerViewerWidget* ViewerWidget; // 3D viewer widget
  vtkSlicerFiducialListWidget* FiducialListWidget; 
  vtkSlicerROIViewerWidget* ROIViewerWidget;

  vtkSlicerApplicationGUI* ApplicationGUI;

private:
  vtkSlicerSecondaryViewerWindow(const vtkSlicerSecondaryViewerWindow&);
  void operator=(const vtkSlicerSecondaryViewerWindow&);
};

#endif
