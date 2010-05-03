#ifndef __vtkEMSegmentStep_h
#define __vtkEMSegmentStep_h

#include "vtkEMSegment.h"
#include "vtkKWWizardStep.h"

class vtkEMSegmentGUI;
class vtkKWMenu;

#define EMSEG_MENU_BUTTON_WIDTH 15
#define EMSEG_WIDGETS_LABEL_WIDTH 25

class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLabelWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
#include <vector>


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
  int SourceTaskFiles(); 
  int SourcePreprocessingTclFiles();

  void SetNextStep(vtkEMSegmentStep *init); 
  virtual void RemoveResults() { } 
  virtual void ShowUserInterface();

  void DefineCheckButton(const char *label, int initState, vtkIdType ID);
  int GetCheckButtonValue(vtkIdType ID); 

  void DefineTextLabel(const char *label, vtkIdType ID);

  void DefineVolumeMenuButton(const char *label, vtkIdType initVolID, vtkIdType buttonID);
  vtkIdType GetVolumeMenuButtonValue(vtkIdType ID); 
  void  VolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID);

  void DefineTextEntry(const char *label, const char *initText, vtkIdType entryID);
  const char* GetTextEntryValue(vtkIdType ID); 

  void SetButtonsFromMRML();

protected:
  vtkEMSegmentStep();
  ~vtkEMSegmentStep();

  vtkEMSegmentGUI *GUI;
  vtkEMSegmentStep *NextStep;

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

 //BTX
  std::vector<vtkKWMenuButtonWithLabel*> volumeMenuButton;
  std::vector<vtkIdType> volumeMenuButtonID;
  std::vector<vtkKWCheckButtonWithLabel*> checkButton;
  std::vector<vtkKWLabelWithLabel*> textLabel;
  std::vector<vtkKWEntryWithLabel*> textEntry;
  //ETX
  vtkKWFrameWithLabel* CheckListFrame;


  void CreateEntryLists();

private:
  vtkEMSegmentStep(const vtkEMSegmentStep&);
  void operator=(const vtkEMSegmentStep&);


};

#endif
