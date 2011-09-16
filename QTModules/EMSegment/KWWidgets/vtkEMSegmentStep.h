#ifndef __vtkEMSegmentStep_h
#define __vtkEMSegmentStep_h

#include "vtkEMSegment.h"
#include "vtkKWWizardStep.h"

class vtkEMSegmentGUI;
class vtkKWMenu;

class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLabelWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkSlicerApplication;

#include <vtkstd/vector>
#include <vtkstd/string>


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

  // Have to do it bc for some reason TCL Wrapping ignores this flag !
  int SourceTclFile(const char *tclFile);

  void SetNextStep(vtkEMSegmentStep *init); 
  virtual void RemoveResults() { } 
  virtual void ShowUserInterface();

  vtkSlicerApplication* GetSlicerApplication();

protected:
  vtkEMSegmentStep();
  ~vtkEMSegmentStep();

  vtkEMSegmentGUI *GUI;
  vtkEMSegmentStep *NextStep;

  // Description:
  // Initialize a menu with selected target volumes
  virtual void PopulateMenuWithTargetVolumes(
    vtkKWMenu *menu, vtkObject *object, const char* callback);

  // Description:
  // Set the selected volume for a menu 
  // Return, 1 on success, 0 otherwise;
  virtual int SetMenuButtonSelectedItem(vtkKWMenu* menu, vtkIdType volId);

  void CreateEntryLists();

private:
  vtkEMSegmentStep(const vtkEMSegmentStep&);
  void operator=(const vtkEMSegmentStep&);


};

#endif
