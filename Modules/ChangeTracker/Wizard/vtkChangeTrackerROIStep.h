#ifndef __vtkChangeTrackerROIStep_h
#define __vtkChangeTrackerROIStep_h

#include "vtkChangeTrackerStep.h"

class vtkKWFrame;
class vtkKWRange;
class vtkKWPushButton;
class vtkSlicerModuleCollapsibleFrame;
class vtkSlicerSliceGUI;
class vtkRenderWindowInteractor;
class vtkMRMLScalarVolumeNode;
class vtkMRMLChangeTrackerNode;
class vtkKWLabel;
class vtkImageRectangularSource;
class vtkMRMLROINode;
class vtkObserverManager;
class vtkSlicerROIDisplayWidget;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerROIStep : public vtkChangeTrackerStep
{
public:
  static vtkChangeTrackerROIStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerROIStep,vtkChangeTrackerStep);
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
  // Process MRML events to get changes from ROI widget
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);
  void AddGUIObservers();
  void RemoveGUIObservers();

protected:
  vtkChangeTrackerROIStep();
  ~vtkChangeTrackerROIStep();

  vtkKWFrame                        *FrameButtons;
  vtkKWFrame                        *FrameBlank;
  vtkSlicerModuleCollapsibleFrame   *FrameROI;
  // keep this for backward compatibility
  vtkSlicerModuleCollapsibleFrame   *FrameROIIJK;

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
  vtkChangeTrackerROIStep(const vtkChangeTrackerROIStep&);
  void operator=(const vtkChangeTrackerROIStep&);
 
  void ROIReset();
  void ROIUpdateWithNewSample(int ijkSample[3]);
  void ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample);
  void ROIUpdateWithNode();
  void ROIUpdateAxisWithNode(vtkMRMLChangeTrackerNode* Node, vtkKWRange *ROIAxis, int Axis);

  int  ROICheck();

  int  ROIMapShow();
  void ROIMapRemove();
  void ROIMapUpdate();

  void AddROISamplingGUIObservers();
  void RemoveROISamplingGUIObservers();
 
  void RetrieveInteractorIJKCoordinates(vtkSlicerSliceGUI *sliceGUI, vtkRenderWindowInteractor *rwi,int coords[3]);

  void ROIChangedCallback(int axis, double min, double max); 

  void DeleteSuperSampleNode();

  void MRMLUpdateROINodeFromROI();
  void MRMLUpdateROIFromROINode();
  void ResetROICenter(int *ijkCoords);

  void ROIIntensityMinMaxUpdate(vtkImageData* image, double &intensityMin, double &intensityMax);
  void InitROIRender();
  void UpdateROIRender();
  void ResetROIRender();

  vtkMRMLScalarVolumeNode *ROILabelMapNode;
  vtkImageRectangularSource *ROILabelMap;
  int ROIHideFlag; 

  vtkMRMLROINode *roiNode;
  vtkSlicerROIDisplayWidget *roiWidget;
  vtkObserverManager *MRMLObserverManager; // to process events from roi node
  bool roiUpdateGuard;
};

#endif
