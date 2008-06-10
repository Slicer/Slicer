#ifndef __vtkTumorGrowthROIStep_h
#define __vtkTumorGrowthROIStep_h

#include "vtkTumorGrowthStep.h"

class vtkKWFrame;
class vtkKWRange;
class vtkKWPushButton;
class vtkSlicerModuleCollapsibleFrame;
class vtkSlicerSliceGUI;
class vtkRenderWindowInteractor;
class vtkMRMLScalarVolumeNode;
class vtkMRMLTumorGrowthNode;
class vtkKWLabel;
class vtkImageRectangularSource;

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthROIStep : public vtkTumorGrowthStep
{
public:
  static vtkTumorGrowthROIStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthROIStep,vtkTumorGrowthStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  // Description:
  // Callbacks.
  void ROIXChangedCallback(double min, double max); 
  void ROIYChangedCallback(double min, double max); 
  void ROIZChangedCallback(double min, double max); 

  virtual void TransitionCallback(); 
  // We call this function in order to remove nodes when going backwards 
  virtual void RemoveResults()  { 
    this->ROIMapRemove();
  }

  // Description:
  // Callback functions for buttons
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);
  void AddGUIObservers();
  void RemoveGUIObservers();

protected:
  vtkTumorGrowthROIStep();
  ~vtkTumorGrowthROIStep();

  vtkKWFrame                        *FrameButtons;
  vtkKWFrame                        *FrameBlank;
  vtkSlicerModuleCollapsibleFrame   *FrameROI;

  vtkKWFrame                        *FrameROIX;
  vtkKWFrame                        *FrameROIY;
  vtkKWFrame                        *FrameROIZ;

  vtkKWPushButton           *ButtonsShow;
  vtkKWPushButton           *ButtonsReset;

  vtkKWRange *ROIX;
  vtkKWRange *ROIY;
  vtkKWRange *ROIZ;

  vtkKWLabel *LabelROIX;
  vtkKWLabel *LabelROIY;
  vtkKWLabel *LabelROIZ;

  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData );
 
private:
  vtkTumorGrowthROIStep(const vtkTumorGrowthROIStep&);
  void operator=(const vtkTumorGrowthROIStep&);
 
  void ROIReset();
  void ROIUpdateWithNewSample(int ijkSample[3]);
  void ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample);
  void ROIUpdateWithNode();
  void ROIUpdateAxisWithNode(vtkMRMLTumorGrowthNode* Node, vtkKWRange *ROIAxis, int Axis);

  int  ROICheck();

  int  ROIMapShow();
  void ROIMapRemove();
  void ROIMapUpdate();

  void AddROISamplingGUIObservers();
  void RemoveROISamplingGUIObservers();
 
  void RetrieveInteractorIJKCoordinates(vtkSlicerSliceGUI *sliceGUI, vtkRenderWindowInteractor *rwi,int coords[3]);

  void ROIChangedCallback(int axis, double min, double max); 

  void DeleteSuperSampleNode();

  vtkMRMLScalarVolumeNode *ROILabelMapNode;
  vtkImageRectangularSource *ROILabelMap;

};

#endif
