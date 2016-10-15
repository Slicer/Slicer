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
  //vtkSetObjectMacro(OutputVol, vtkImageData);

  vtkSetMacro(InitializationFlag, bool);

  //processing functions
  void Initialization();
  void RunFGC();
  void PrintSelf(ostream &os, vtkIndent indent);

  class vtkInternal;

protected:
  vtkFastGrowCutSeg();
  virtual ~vtkFastGrowCutSeg();

  virtual void ExecuteDataWithInformation(vtkDataObject *outData, vtkInformation *outInfo);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  //vtk image data (from slicer)
  vtkImageData* SourceVol;
  vtkImageData* SeedVol;

  //state variables
  bool InitializationFlag;

  vtkInternal * Internal;
};

#endif
