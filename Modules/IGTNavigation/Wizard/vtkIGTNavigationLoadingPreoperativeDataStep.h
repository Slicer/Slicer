#ifndef __vtkIGTNavigationLoadingPreoperativeDataStep_h
#define __vtkIGTNavigationLoadingPreoperativeDataStep_h

#include "vtkIGTNavigationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_IGT_EXPORT vtkIGTNavigationLoadingPreoperativeDataStep : public vtkIGTNavigationStep
{
public:
  static vtkIGTNavigationLoadingPreoperativeDataStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationLoadingPreoperativeDataStep,vtkIGTNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTNavigationLoadingPreoperativeDataStep();
  ~vtkIGTNavigationLoadingPreoperativeDataStep();

  virtual void PopulatePreoperativeImageDataSelector();
  virtual void PopulateToolModelSelector();
 
  vtkKWMenuButtonWithLabel   *PreoperativeImageDataMenuButton; 
  vtkKWMenuButtonWithLabel   *ToolModelMenuButton; 

private:
  vtkIGTNavigationLoadingPreoperativeDataStep(const vtkIGTNavigationLoadingPreoperativeDataStep&);
  void operator=(const vtkIGTNavigationLoadingPreoperativeDataStep&);
};

#endif
