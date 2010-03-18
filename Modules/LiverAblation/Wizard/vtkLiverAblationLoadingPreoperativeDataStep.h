#ifndef __vtkLiverAblationLoadingPreoperativeDataStep_h
#define __vtkLiverAblationLoadingPreoperativeDataStep_h

#include "vtkLiverAblationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWGuideWidget;
class vtkKWTopLevel;

class VTK_LIVERABLATION_EXPORT vtkLiverAblationLoadingPreoperativeDataStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationLoadingPreoperativeDataStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationLoadingPreoperativeDataStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void AddVolumeButtonCallback();
  virtual void GuideWidgetButtonCallback();


protected:
  vtkLiverAblationLoadingPreoperativeDataStep();
  ~vtkLiverAblationLoadingPreoperativeDataStep();

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
  vtkLiverAblationLoadingPreoperativeDataStep(const vtkLiverAblationLoadingPreoperativeDataStep&);
  void operator=(const vtkLiverAblationLoadingPreoperativeDataStep&);
};

#endif
