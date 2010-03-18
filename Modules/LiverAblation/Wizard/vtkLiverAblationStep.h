#ifndef __vtkLiverAblationStep_h
#define __vtkLiverAblationStep_h

#include "vtkLiverAblation.h"
#include "vtkKWWizardStep.h"

class vtkLiverAblationGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_LIVERABLATION_EXPORT vtkLiverAblationStep : public vtkKWWizardStep
{
public:
  static vtkLiverAblationStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkLiverAblationGUI);
  virtual void SetGUI(vtkLiverAblationGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkLiverAblationStep();
  ~vtkLiverAblationStep();

  vtkLiverAblationGUI *GUI;
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
  vtkLiverAblationStep(const vtkLiverAblationStep&);
  void operator=(const vtkLiverAblationStep&);
};

#endif
