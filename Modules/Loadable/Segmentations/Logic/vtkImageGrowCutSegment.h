#ifndef FASTGROWCUT_H
#define FASTGROWCUT_H

#include "vtkSlicerSegmentationsModuleLogicExport.h"

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkInformation.h>

class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkImageGrowCutSegment : public vtkImageAlgorithm
{
public:
  static vtkImageGrowCutSegment* New();
  vtkTypeMacro(vtkImageGrowCutSegment, vtkImageAlgorithm);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /// Set input grayscale volume (input 0)
  void SetIntensityVolume(vtkImageData* grayscaleImage) { this->SetInputData(0, grayscaleImage); }

  /// Set input seed label volume (input 1)
  void SetSeedLabelVolume(vtkImageData* labelImage) { this->SetInputData(1, labelImage); }

  /// Set mask volume (input 2). Optional.
  /// If this volume is specified then only those regions outside the mask (where mask has zero value)
  /// will be included in the segmentation result. Regions outside the mask will not be used
  /// for region growing either (growing will not start from or cross through masked region).
  void SetMaskVolume(vtkImageData* labelImage) { this->SetInputData(2, labelImage); }

  /// Reset to initial state. This forces full recomputation of the result label volume.
  /// This method has to be called if intensity volume changes or if seeds are deleted after initial computation.
  void Reset();

  /// Spatial regularization factor, which can force growing in nearby regions.
  /// For each physical unit distance, this much intensity level difference is simulated.
  /// By default = 0, which means spatial distance does not play a role in the region growing, only intensity value similarity.
  vtkGetMacro(DistancePenalty, double);
  vtkSetMacro(DistancePenalty, double);

protected:
  vtkImageGrowCutSegment();
  ~vtkImageGrowCutSegment() override;

  void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo) override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  int FillInputPortInformation(int port, vtkInformation * info) override;

private:
  vtkImageGrowCutSegment(const vtkImageGrowCutSegment&) = delete;
  void operator=(const vtkImageGrowCutSegment&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  double DistancePenalty;
};

#endif
