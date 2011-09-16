#ifndef __vtkEMSegmentInputChannelsStep_h
#define __vtkEMSegmentInputChannelsStep_h

#include "vtkEMSegmentStep.h"
#include <vector>

class vtkKWFrameWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWFrame;
class vtkKWCheckButtonWithLabel;
class vtkKWPushButton;
class vtkMRMLVolumeNode;
class vtkEMSegmentKWDynamicFrame;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentInputChannelsStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentInputChannelsStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentInputChannelsStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  //  virtual void IntensityImagesTargetSelectionChangedCallback();
  virtual void AlignTargetImagesCallback(int state);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();

  int AddInputChannel();
  int RemoveInputChannel();

  int GetNumberOfInputChannels();

  vtkGetObjectMacro(CheckListFrame,vtkEMSegmentKWDynamicFrame);
protected:
  vtkEMSegmentInputChannelsStep();
  ~vtkEMSegmentInputChannelsStep();

  // virtual void PopulateIntensityImagesTargetVolumeSelector();

  vtkKWFrameWithLabel        *InputChannelDefineFrame;

  int NumberOfInputChannelLines;

  vtkKWFrame *InputAddRemoveChannelFrame;
  vtkKWPushButton *AddInputChannelButton;
  vtkKWPushButton *RemoveInputChannelButton;

  vtkKWFrameWithLabel        *TargetToTargetRegistrationFrame;
  vtkKWCheckButtonWithLabel  *IntensityImagesAlignTargetImagesCheckButton;
 vtkEMSegmentKWDynamicFrame* CheckListFrame;

  //BTX
  std::vector<vtkKWFrameWithLabel*> InputChannelDefineLineFrame;
  std::vector<vtkKWEntryWithLabel*> InputChannelDefineLineName;
  std::vector<vtkSlicerNodeSelectorWidget*> InputChannelDefineLineVolume; 
  //ETX
private:
  vtkEMSegmentInputChannelsStep(const vtkEMSegmentInputChannelsStep&);
  void operator=(const vtkEMSegmentInputChannelsStep&);
  void DeleteAllInputChannelFrames();
  void DeleteInputChannelFrame(vtkIdType lineNum);
  void UpdateInputChannelsfromMRML();
  void ChangeNumberOfInputChannel(int newLines) ;
  void CreateInputChannelFrame(int i, const char* newName, vtkMRMLVolumeNode* newVolumeNode); 
};

#endif
