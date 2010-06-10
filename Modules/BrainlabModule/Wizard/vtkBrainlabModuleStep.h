#ifndef __vtkBrainlabModuleStep_h
#define __vtkBrainlabModuleStep_h

#include "vtkBrainlabModule.h"
#include "vtkKWWizardStep.h"

class vtkBrainlabModuleGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleStep : public vtkKWWizardStep
{
public:
  static vtkBrainlabModuleStep *New();
  vtkTypeRevisionMacro(vtkBrainlabModuleStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkBrainlabModuleGUI);
  virtual void SetGUI(vtkBrainlabModuleGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkBrainlabModuleStep();
  ~vtkBrainlabModuleStep();

  vtkBrainlabModuleGUI *GUI;
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

  void RaiseModule(const char *moduleName);

private:
  vtkBrainlabModuleStep(const vtkBrainlabModuleStep&);
  void operator=(const vtkBrainlabModuleStep&);
};

#endif
