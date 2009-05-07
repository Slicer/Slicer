#ifndef __vtkIGTPlanningStep_h
#define __vtkIGTPlanningStep_h

#include "vtkIGTPlanning.h"
#include "vtkKWWizardStep.h"

class vtkIGTPlanningGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_IGT_EXPORT vtkIGTPlanningStep : public vtkKWWizardStep
{
public:
  static vtkIGTPlanningStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkIGTPlanningGUI);
  virtual void SetGUI(vtkIGTPlanningGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkIGTPlanningStep();
  ~vtkIGTPlanningStep();

  vtkIGTPlanningGUI *GUI;
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
  vtkIGTPlanningStep(const vtkIGTPlanningStep&);
  void operator=(const vtkIGTPlanningStep&);
};

#endif
