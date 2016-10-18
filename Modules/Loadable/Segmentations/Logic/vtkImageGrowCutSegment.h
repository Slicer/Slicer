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
  void PrintSelf(ostream &os, vtkIndent indent);

  // Set input grayscale volume (input 0)
  void SetIntensityVolume(vtkImageData* grayscaleImage) { this->SetInputData(0, grayscaleImage); }

  // Set input seed label volume (input 1)
  void SetSeedLabelVolume(vtkImageData* labelImage) { this->SetInputData(1, labelImage); }

  // Reset to initial state. This forces full recomputation of the result label volume.
  // This method has to be called if intensity volume changes or if seeds are deleted after initial computation.
  void Reset();

protected:
  vtkImageGrowCutSegment();
  virtual ~vtkImageGrowCutSegment();

  virtual void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  class vtkInternal;
  vtkInternal * Internal;
};

#endif
