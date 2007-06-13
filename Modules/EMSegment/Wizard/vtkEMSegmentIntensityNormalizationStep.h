#ifndef __vtkEMSegmentIntensityNormalizationStep_h
#define __vtkEMSegmentIntensityNormalizationStep_h

#include "vtkEMSegmentStep.h"

class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntry;
class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;

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

private:
  vtkEMSegmentIntensityNormalizationStep(const vtkEMSegmentIntensityNormalizationStep&);
  void operator=(const vtkEMSegmentIntensityNormalizationStep&);
};

#endif
