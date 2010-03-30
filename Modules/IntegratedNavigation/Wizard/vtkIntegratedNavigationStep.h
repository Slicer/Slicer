#ifndef __vtkIntegratedNavigationStep_h
#define __vtkIntegratedNavigationStep_h

#include "vtkIntegratedNavigation.h"
#include "vtkKWWizardStep.h"

class vtkIntegratedNavigationGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_IGT_EXPORT vtkIntegratedNavigationStep : public vtkKWWizardStep
{
public:
  static vtkIntegratedNavigationStep *New();
  vtkTypeRevisionMacro(vtkIntegratedNavigationStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkIntegratedNavigationGUI);
  virtual void SetGUI(vtkIntegratedNavigationGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkIntegratedNavigationStep();
  ~vtkIntegratedNavigationStep();

  vtkIntegratedNavigationGUI *GUI;
  vtkCallbackCommand *GUICallbackCommand;

  // Description:
  // Initialize a menu with loaded volumes
  virtual void PopulateMenuWithLoadedVolumes(
    vtkKWMenu *menu, vtkObject *object, const char* callback);

  // Description:
  // Set the selected volume for a menu 
  // Return, 1 on success, 0 otherwise;
  virtual int SetMenuButtonSelectedItem(vtkKWMenu* menu, vtkIdType volId);

  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {};
  static void GUICallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData );

private:
  vtkIntegratedNavigationStep(const vtkIntegratedNavigationStep&);
  void operator=(const vtkIntegratedNavigationStep&);
};

#endif
