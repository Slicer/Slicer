#ifndef __vtkProstateNavFiducialCalibrationStep_h
#define __vtkProstateNavFiducialCalibrationStep_h

#include "vtkProstateNavStep.h"

// Include algo class to get CALIB_MARKER_COUNT
#include "vtkTransRectalFiducialCalibrationAlgo.h"

#include "vtkstd/vector"

class vtkKWPushButton;
class vtkKWSpinBoxWithLabel;
class vtkKWCheckButton;
class vtkKWScaleWithEntry;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWText;
class vtkKWTextWithScrollbars;
class vtkKWEntrySet;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkSlicerNodeSelectorWidget;

class vtkSlicerInteractorStyle;
class vtkMRMLVolumeNode;
class vtkActor;
class vtkSlicerViewerWidget;

class vtkMRMLScalarVolumeNode;
class vtkMRMLTRProstateBiopsyModuleNode;

class vtkImageData;
class vtkVolumeMapper;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolumeProperty;
class vtkVolume;
class vtkMatrix4x4; 
class vtkKWRenderWidget;
class vtkMRMLTransRectalProstateRobotNode;


#include "vtkSmartPointer.h"

class VTK_PROSTATENAV_EXPORT vtkProstateNavFiducialCalibrationStep :
  public vtkProstateNavStep
{
public:
  static vtkProstateNavFiducialCalibrationStep *New();
  vtkTypeRevisionMacro(vtkProstateNavFiducialCalibrationStep,
                       vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  virtual void UpdateMRML();
  virtual void UpdateGUI();  

  // Description:
  // The Enter and Exit functions are not part of vtkKWWizardStep.
  // They have been added here sot that they can be called by
  // ShowUserInterface and HideUserInterface.
  virtual void Enter();
  virtual void Exit();

  virtual void TearDownGUI();

  // Description:
  // Process GUI events
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event,
                                void *callData);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  // Description
  // Reset
  virtual void Reset();

protected:
  vtkProstateNavFiducialCalibrationStep();
  ~vtkProstateNavFiducialCalibrationStep();

  
  void ShowLoadResetControls();
  void ShowLoadVolumeControls();
  void ShowFiducialSegmentParamsControls();
  void ShowFiducialSegmentationResultsControls();
  void ShowExportImportControls();

  void PopulateCalibrationResults();

  void AddGUIObservers();
  void RemoveGUIObservers();

  void AddMRMLObservers();
  void RemoveMRMLObservers();

  void SegmentRegisterMarkers();
  void SegmentAxis(int nAxis);
  void Resegment();

  void ShowAxesIn3DView(bool show);
  void ShowMarkerVolumesIn3DView(bool show);

  void EnableMarkerPositionEdit(bool enable);

  void UpdateCalibration();

  void JumpToFiducial(unsigned int i);

  // Description:
  // GUI callback  
  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData);

  vtkMRMLTransRectalProstateRobotNode* GetRobot();

  vtkSlicerViewerWidget* GetMainViewerWidget();

  //BTX

  vtkSmartPointer<vtkKWFrame> LoadVolumeDialogFrame;
  vtkSmartPointer<vtkKWPushButton> LoadCalibrationVolumeButton;
  vtkSmartPointer<vtkSlicerNodeSelectorWidget> VolumeSelectorWidget;

  // 3) reset push button, this will require segmentation from start i.e.
  // new specification of 4 clicks on fiducials
  vtkSmartPointer<vtkKWPushButton> ResetCalibrationButton;
  // 4) Re-segment push button, this will not require new specification of 4 clicks, but this will
  // be used when Fiducials thresholds, initial angle, radius etc parameters are changed
  vtkSmartPointer<vtkKWPushButton> ResegmentButton;
  vtkSmartPointer<vtkKWCheckButton> EditMarkerPositionButton;
  // also associated frames
  vtkSmartPointer<vtkKWFrame> LoadResetFrame;
  vtkSmartPointer<vtkKWFrame> ExportImportFrame;

  vtkSmartPointer<vtkKWFrameWithLabel> FiducialPropertiesFrame;
  vtkSmartPointer<vtkKWLabel> FiducialSizeLabel;
  vtkSmartPointer<vtkKWPushButton> JumpToFiducialButton[CALIB_MARKER_COUNT];
  vtkSmartPointer<vtkKWSpinBoxWithLabel> FiducialWidthSpinBox;
  vtkSmartPointer<vtkKWSpinBoxWithLabel> FiducialHeightSpinBox;
  vtkSmartPointer<vtkKWSpinBoxWithLabel> FiducialDepthSpinBox;
  vtkSmartPointer<vtkKWScaleWithEntry> FiducialThresholdScale[CALIB_MARKER_COUNT];
  vtkSmartPointer<vtkKWCheckButton> AutomaticCenterpointAdjustmentCheckButton;
  vtkSmartPointer<vtkKWSpinBoxWithLabel> RadiusSpinBox;
  vtkSmartPointer<vtkKWCheckButton> RadiusCheckButton;
  vtkSmartPointer<vtkKWSpinBoxWithLabel> InitialAngleSpinBox;

  vtkSmartPointer<vtkKWFrameWithLabel> SegmentationResultsFrame;
  vtkSmartPointer<vtkKWTextWithScrollbars> CalibrationResultsBox;

  // Description:
  // CalibrationListNode is used for displaying 4 fiducial points that defines the calibration marker initial positions
  vtkSmartPointer<vtkMRMLFiducialListNode> CalibrationPointListNode;

  //ETX

  bool ProcessingCallback;

  vtkMRMLTransRectalProstateRobotNode* ObservedRobot;
  bool EnableAutomaticCenterpointAdjustment;

private:  
  vtkProstateNavFiducialCalibrationStep(const vtkProstateNavFiducialCalibrationStep&);
  void operator=(const vtkProstateNavFiducialCalibrationStep&);
};

#endif
