#ifndef __vtkEMSegmentStep_h
#define __vtkEMSegmentStep_h

#include "vtkEMSegment.h"
#include "vtkKWWizardStep.h"

class vtkEMSegmentGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class VTK_EMSEGMENT_EXPORT vtkEMSegmentStep : public vtkKWWizardStep
{
public:
  static vtkEMSegmentStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkEMSegmentGUI);
  virtual void SetGUI(vtkEMSegmentGUI*);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

protected:
  vtkEMSegmentStep();
  ~vtkEMSegmentStep();

  vtkEMSegmentGUI *GUI;

  // Description:
  // Initialize a menu with loaded volumes
  virtual void PopulateMenuWithLoadedVolumes(
    vtkKWMenu *menu, vtkObject *object, const char* callback);

  // Description:
  // Initialize a menu with selected target volumes
  virtual void PopulateMenuWithTargetVolumes(
    vtkKWMenu *menu, vtkObject *object, const char* callback);

  // Description:
  // Set the selected volume for a menu 
  // Return, 1 on success, 0 otherwise;
  virtual int SetMenuButtonSelectedItem(vtkKWMenu* menu, vtkIdType volId);

private:
  vtkEMSegmentStep(const vtkEMSegmentStep&);
  void operator=(const vtkEMSegmentStep&);
};

#endif
