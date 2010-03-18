#ifndef __vtkKWGuideWidget_h
#define __vtkKWGuideWidget_h

#include "vtkLiverAblation.h"
#include "vtkKWTopLevel.h"
#include "vtkSlicerApplicationGUI.h"

class vtkKWFrame;
class vtkKWPushButton;
class vtkKWLabel;

class VTK_LIVERABLATION_EXPORT vtkKWGuideWidget : public vtkKWTopLevel
{
public:
  static vtkKWGuideWidget* New();
  vtkTypeRevisionMacro(vtkKWGuideWidget, vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  virtual void FiducialButtonCallback();
  virtual void EditorButtonCallback();
  virtual void ExitButtonCallback();

  virtual void Display(int x, int y);

  vtkSetObjectMacro(SlicerAppGUI, vtkSlicerApplicationGUI);


protected:
  vtkKWGuideWidget();
  ~vtkKWGuideWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  vtkKWFrame      *ButtonFrame;
  vtkKWPushButton *FiducialButton;
  vtkKWPushButton *EditorButton;
  vtkKWPushButton *ExitButton;
  vtkSlicerApplicationGUI *SlicerAppGUI;


private:
  vtkKWGuideWidget(const vtkKWGuideWidget&); // Not implemented
  void operator=(const vtkKWGuideWidget&); // Not implemented

  void RaiseModule(const char *moduleName);

};

#endif

