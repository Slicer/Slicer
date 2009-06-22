#ifndef __vtkEMSegmentIntensityNormalizationStep_h
#define __vtkEMSegmentIntensityNormalizationStep_h

#include "vtkEMSegmentStep.h"

class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntry;
class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWHistogram;
class vtkKWPiecewiseFunctionEditor;

class vtkImageMeanIntensityPreNormalization;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityNormalizationStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityNormalizationStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityNormalizationStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void NormalizationTargetSelectionChangedCallback(
    vtkIdType VolId);
  virtual void NormalizationEnableCallback(
    vtkIdType VolId, int state);
  virtual void NormalizationNormTypeCallback(
    vtkIdType VolId, int enumType);
  virtual void NormalizationPrintInfoCallback(
    vtkIdType VolId, int checked);
  virtual void NormalizationNormValueCallback(
    vtkIdType VolId, double dValue);
  virtual void NormalizationSmoothingWidthCallback(
    vtkIdType VolId, int iValue);
  virtual void NormalizationMaxSmoothingWidthCallback(
    vtkIdType VolId, int iValue);
  virtual void NormalizationRelativeMaxVoxelNumCallback(
    vtkIdType VolId, double dValue);
  //virtual void ParameterCursorMoveCallback(int x);

  virtual void NormalizationHistogramChangedCallback(
  vtkIdType VolId);
  //Get histogram Value
  virtual void GetValueeee ();

  // Description:
  // Observers
  virtual void AddCursorMovingGUIEvents();
  virtual void RemoveCursorMovingGUIEvents();
  virtual void ProcessCursorMovingGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);




  //BTX
  enum
    {
    NormalizationDefaultT1SPGR = 0,
    NormalizationDefaultT2
    };
  //ETX

protected:
  vtkEMSegmentIntensityNormalizationStep();
  ~vtkEMSegmentIntensityNormalizationStep();

  virtual void PopulateNormalizationTargetVolumeSelector();
  virtual void PopulateNormalizationHistogramSelector();
  
  virtual void ResetDefaultParameters(vtkIdType target_vol_id);

  vtkKWMenuButtonWithLabel  *NormalizationTargetVolumeMenuButton;
  vtkKWFrameWithLabel       *NormalizationParametersFrame;
  vtkKWCheckButtonWithLabel *NormalizationEnableCheckButton;
  vtkKWMenuButton  *NormalizationDefaultsMenuButton;
  vtkKWCheckButtonWithLabel *NormalizationPrintCheckButton;
  vtkKWEntryWithLabel       *NormalizationNormValueEntry;
  vtkKWEntryWithLabel       *NormalizationSmoothingWidthEntry;
  vtkKWEntryWithLabel       *NormalizationMaxSmoothingWidthEntry;
  vtkKWScaleWithEntry       *NormalizationRelativeMaxVoxelScale;
  
  vtkKWMenuButtonWithLabel  *NormalizationHistogramMenuButton;
  vtkKWHistogram            *NormalizationHistogram;
  vtkKWFrameWithLabel       *NormalizationHistogramFrame;
  vtkKWPiecewiseFunctionEditor *VisuHisto;
  
  vtkKWEntryWithLabel       *NormalizationValueRecommandedEntry;
  vtkKWFrameWithLabel       *RecommandationFrame;
  
  vtkImageMeanIntensityPreNormalization* ImageMeanIntensityPreNormalization;
  
  vtkIdType id_event;

private:
  vtkEMSegmentIntensityNormalizationStep(const vtkEMSegmentIntensityNormalizationStep&);
  void operator=(const vtkEMSegmentIntensityNormalizationStep&);
};

#endif
