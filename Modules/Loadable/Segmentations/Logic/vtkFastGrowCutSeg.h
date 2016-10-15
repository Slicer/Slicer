#ifndef FASTGROWCUT_H
#define FASTGROWCUT_H

#include "vtkSlicerSegmentationsModuleLogicExport.h"

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>

class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkFastGrowCutSeg : public vtkImageAlgorithm
{
public:
  static vtkFastGrowCutSeg* New();
  vtkTypeMacro(vtkFastGrowCutSeg, vtkImageAlgorithm);

  //set parameters of grow cut
  void SetSourceVol(vtkImageData* image) { this->SetInputData(0, image); }
  void SetSeedVol(vtkImageData* image) { this->SetInputData(1, image); }

  // Reset to initial state.
  // This has to be called if the source image is changed or there were deletions in
  // the seed volume.
  void Reset();
  void PrintSelf(ostream &os, vtkIndent indent);

  class vtkInternal;

protected:
  vtkFastGrowCutSeg();
  virtual ~vtkFastGrowCutSeg();

  virtual void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:

  vtkInternal * Internal;
};

#endif
