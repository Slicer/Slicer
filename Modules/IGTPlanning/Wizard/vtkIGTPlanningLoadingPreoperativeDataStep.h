#ifndef __vtkIGTPlanningLoadingPreoperativeDataStep_h
#define __vtkIGTPlanningLoadingPreoperativeDataStep_h

#include "vtkIGTPlanningStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWGuideWidget;
class vtkKWTopLevel;

class VTK_IGT_EXPORT vtkIGTPlanningLoadingPreoperativeDataStep : public vtkIGTPlanningStep
{
public:
  static vtkIGTPlanningLoadingPreoperativeDataStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningLoadingPreoperativeDataStep,vtkIGTPlanningStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void AddVolumeButtonCallback();
  virtual void GuideWidgetButtonCallback();


protected:
  vtkIGTPlanningLoadingPreoperativeDataStep();
  ~vtkIGTPlanningLoadingPreoperativeDataStep();

  virtual void PopulatePreoperativeImageDataSelector();
  virtual void PopulateToolModelSelector();
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

  vtkKWMenuButtonWithLabel   *PreoperativeImageDataMenuButton; 
  vtkKWMenuButtonWithLabel   *ToolModelMenuButton; 

  vtkKWPushButton            *AddVolumeButton; 
  vtkKWPushButton            *GuideWidgetButton; 

  // Description:
  // guide widget 
  vtkKWGuideWidget *GuideWidget;
  virtual int CreateGuideWidget();

  // Description:
  // Display the warning/error/information/debug message log dialog.
  // Optionally provide a master window this dialog should be the slave of.
  virtual void DisplayGuideWidget(vtkKWTopLevel *master);
  virtual vtkKWGuideWidget* GetGuideWidget();

private:
  vtkIGTPlanningLoadingPreoperativeDataStep(const vtkIGTPlanningLoadingPreoperativeDataStep&);
  void operator=(const vtkIGTPlanningLoadingPreoperativeDataStep&);
};

#endif
