#ifndef __vtkIGTNavigationStep_h
#define __vtkIGTNavigationStep_h

#include "vtkIGTNavigation.h"
#include "vtkKWWizardStep.h"

class vtkIGTNavigationGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_IGT_EXPORT vtkIGTNavigationStep : public vtkKWWizardStep
{
public:
  static vtkIGTNavigationStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkIGTNavigationGUI);
  virtual void SetGUI(vtkIGTNavigationGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkIGTNavigationStep();
  ~vtkIGTNavigationStep();

  vtkIGTNavigationGUI *GUI;

  // Description:
  // Initialize a menu with loaded volumes
  virtual void PopulateMenuWithLoadedVolumes(
    vtkKWMenu *menu, vtkObject *object, const char* callback);

  // Description:
  // Set the selected volume for a menu 
  // Return, 1 on success, 0 otherwise;
  virtual int SetMenuButtonSelectedItem(vtkKWMenu* menu, vtkIdType volId);

private:
  vtkIGTNavigationStep(const vtkIGTNavigationStep&);
  void operator=(const vtkIGTNavigationStep&);
};

#endif
