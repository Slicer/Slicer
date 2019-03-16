#ifndef FASTGROWCUT_H
#define FASTGROWCUT_H

#include "vtkSlicerSegmentationsModuleLogicExport.h"

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>

class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkImageGrowCutSegment : public vtkImageAlgorithm
{
public:
  static vtkImageGrowCutSegment* New();
  vtkTypeMacro(vtkImageGrowCutSegment, vtkImageAlgorithm);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Set input grayscale volume (input 0)
  void SetIntensityVolume(vtkImageData* grayscaleImage) { this->SetInputData(0, grayscaleImage); }

  // Set input seed label volume (input 1)
  void SetSeedLabelVolume(vtkImageData* labelImage) { this->SetInputData(1, labelImage); }

  // Set mask volume (input 2). Optional.
  // If this volume is specified then only those regions outside the mask (where mask has zero value)
  // will be included in the segmentation result. Regions outside the mask will not be used
  // for region growing either (growing will not start from or cross through masked region).
  void SetMaskVolume(vtkImageData* labelImage) { this->SetInputData(2, labelImage); }

  // Reset to initial state. This forces full recomputation of the result label volume.
  // This method has to be called if intensity volume changes or if seeds are deleted after initial computation.
  void Reset();

protected:
  vtkImageGrowCutSegment();
  ~vtkImageGrowCutSegment() override;

  void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo) override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  class vtkInternal;
  vtkInternal * Internal;
};

#endif
