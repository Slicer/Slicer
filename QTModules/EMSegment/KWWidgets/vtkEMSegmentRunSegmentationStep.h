#ifndef __vtkEMSegmentRunSegmentationStep_h
#define __vtkEMSegmentRunSegmentationStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWLoadSaveButton;
class vtkKWCheckButtonWithLabel;
class vtkKWMatrixWidgetWithLabel;
class vtkKWMatrixWidget;
class vtkKWFrame;
class vtkSlicerModuleCollapsibleFrame;
class vtkKWPushButton;
class vtkSlicerVisibilityIcons;
class vtkKWLabel;
class vtkKWRange;
class vtkEMSegmentMRMLManager ;
class vtkSlicerSliceGUI;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkMRMLScalarVolumeNode;
class vtkImageRectangularSource;
class vtkMRMLROINode;
class vtkSlicerROIDisplayWidget;
class vtkObserverManager;
class vtkSlicerApplicationGUI;
class vtkKWWidget;
class vtkKWScaleWithEntry;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentRunSegmentationStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentRunSegmentationStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentRunSegmentationStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  // Description:
  // Callbacks.
  virtual void SelectTemplateFileCallback();
  virtual void SelectDirectoryCallback();
  virtual void GenerateSurfaceModelsCallback(int state);
  virtual void SaveIntermediateCallback(int state);
  virtual void SaveAfterSegmentationCallback(int state);
  virtual void MultiThreadingCallback(int state);
  virtual void PostprocessingSubparcellationCallback(int state);
  virtual void PostprocessingMinimumIslandSizeCallback(float value);
  virtual void PostprocessingIsland2DFlagCallback(int state);

  virtual void StartSegmentationCallback();
  virtual void CancelSegmentationCallback();

  // Description:
  // Observers.
  //virtual void AddRunRegistrationOutputGUIObservers();
  // virtual void RemoveRunRegistrationOutputGUIObservers();
  // virtual void ProcessRunRegistrationOutputGUIEvents( vtkObject *caller, unsigned long event, void *callData);

  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData );
  static void ROIMRMLCallback(vtkObject*, unsigned long, void*, void*);
  static void SetRedGreenYellowAllVolumes(vtkSlicerApplicationGUI *applicationGUI, const char* VolID);
  virtual void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkEMSegmentRunSegmentationStep();
  ~vtkEMSegmentRunSegmentationStep();

  vtkKWFrameWithLabel        *RunSegmentationSaveFrame;
  vtkKWFrameWithLabel        *RunSegmentationDirectoryFrame;
  //vtkKWFrameWithLabel        *RunSegmentationOutputFrame;
  //vtkSlicerNodeSelectorWidget* RunSegmentationOutVolumeSelector;
  vtkKWFrame                 *RunSegmentationSaveTemplateFrame;
  vtkKWLabel                 *RunSegmentationSaveTemplateLabel;
  vtkKWLoadSaveButton        *RunSegmentationSaveTemplateButton;

  vtkKWFrame                 *RunSegmentationDirectorySubFrame;
  vtkKWLabel                 *RunSegmentationDirectoryLabel;
  vtkKWLoadSaveButton        *RunSegmentationDirectoryButton;
  vtkKWCheckButtonWithLabel  *RunSegmentationSaveIntermediateCheckButton;
  vtkKWCheckButtonWithLabel  *RunSegmentationGenerateSurfaceCheckButton;
   vtkKWFrameWithLabel        *RunSegmentationMiscFrame;
  vtkKWCheckButtonWithLabel  *RunSegmentationMultiThreadCheckButton;

  vtkKWFrameWithLabel        *PostprocessingFrame;
  vtkKWScaleWithEntry        *PostprocessingMinimumIslandSize;
  vtkKWCheckButtonWithLabel  *PostprocessingSubparcellationCheckButton;
  vtkKWCheckButtonWithLabel  *PostprocessingIsland2DCheckButton;

  // Description:
  // Populate the run segmentation boundary matrix 
  void PopulateSegmentationROIMatrix(
    vtkKWMatrixWidget* matrix, int ijk[3]);

private:
  vtkEMSegmentRunSegmentationStep(const vtkEMSegmentRunSegmentationStep&);
  void operator=(const vtkEMSegmentRunSegmentationStep&);


  vtkKWFrameWithLabel               *DefineVOIFrame;
  vtkKWFrame                        *FrameButtons;
  vtkKWFrame    *FrameROI;
  vtkKWPushButton           *ButtonsShow;
  vtkSlicerVisibilityIcons  *VisibilityIcons;
  vtkKWLabel                *VisibilityLabel;
  vtkKWPushButton           *ButtonsReset;

  vtkMRMLScalarVolumeNode *ROILabelMapNode;
  vtkImageRectangularSource *ROILabelMap;
  int ROIHideFlag; 

  vtkMRMLROINode *roiNode;
  vtkSlicerROIDisplayWidget *roiWidget;
  vtkObserverManager *MRMLObserverManager; // to process events from roi node
  bool roiUpdateGuard;

  vtkCallbackCommand *ROIMRMLCallbackCommand;

  void InitialROIWidget();
  void ResetROIWidget() ;
  //void ROIXChangedCallback(double min, double max) ;
  //void ROIYChangedCallback(double min, double max) ;
  //void ROIZChangedCallback(double min, double max) ;
  //void ROIChangedCallback(int axis, double min, double max) ; 
  void AddGUIObservers() ;
  void AddROISamplingGUIObservers();
  void RemoveGUIObservers() ;
  void RemoveROISamplingGUIObservers();
  void ROIReset() ;
  void ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample);
  void ROIUpdateWithNewSample(double rasSample[3]);
  // void ROIUpdateAxisWithNode(vtkEMSegmentMRMLManager *mrmlManager, vtkKWRange *ROIAxis, int Axis);
  void ROIUpdateWithNode();
  int ROICheck();
  void ROIMapUpdate();
  int ROIMapShow();
  void ROIMapRemove();
  void RetrieveInteractorCoordinates(vtkSlicerSliceGUI *sliceGUI, vtkRenderWindowInteractor *rwi, int coordsIJK[3], double coordsRAS[3]) ;
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData));
  void MRMLUpdateROIFromROINode();
  void MRMLUpdateROINodeFromROI();
  void ROIIntensityMinMaxUpdate(vtkImageData* image, double &intensityMin, double &intensityMax);
  void ResetROICenter(int *center);
  void ShowROIGUI(vtkKWWidget*);

  vtkCallbackCommand *WizardGUICallbackCommand;

  void CenterRYGSliceViews(double pointX, double pointY, double pointZ);
  int ROICheck(int minPoint[3], int maxPoint[3]) ;
};

#endif
