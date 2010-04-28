#ifndef __vtkEMSegmentPreProcessingStep_h
#define __vtkEMSegmentPreProcessingStep_h

#include "vtkEMSegmentStep.h"
#include <vector>
class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLabelWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentPreProcessingStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentPreProcessingStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentPreProcessingStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  void Validate();
  
  void CreateCheckButton(const char *label, int initState, vtkIdType ID);
  int GetCheckButtonValue(vtkIdType ID); 

  void CreateTextLabel(const char *label, vtkIdType ID);

  void CreateVolumeMenuButton(const char *label, vtkIdType initVolID, vtkIdType buttonID);
  vtkIdType GetVolumeMenuButtonValue(vtkIdType ID); 
  void  PreProcessingVolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID);

  void CreateTextEntry(const char *label, const char *initText, vtkIdType entryID);
  const char* GetTextEntryValue(vtkIdType ID); 

  void SetButtonsFromMRML();
  void SetTaskPreprocessingSetting();

protected:
  vtkEMSegmentPreProcessingStep();
  ~vtkEMSegmentPreProcessingStep();

private:
  vtkEMSegmentPreProcessingStep(const vtkEMSegmentPreProcessingStep&);
  void operator=(const vtkEMSegmentPreProcessingStep&);
 
  vtkKWFrameWithLabel* CheckListFrame;
  //BTX
  std::vector<vtkKWMenuButtonWithLabel*> volumeMenuButton;
  std::vector<vtkIdType> volumeMenuButtonID;
  std::vector<vtkKWCheckButtonWithLabel*> checkButton;
  std::vector<vtkKWLabelWithLabel*> textLabel;
  std::vector<vtkKWEntryWithLabel*> textEntry;
  //ETX
};

#endif
