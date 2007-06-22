#ifndef __vtkHyperStreamlineTeem_h
#define __vtkHyperStreamlineTeem_h

#include "vtkSlicerTractographySeeding.h"
#include "vtkHyperStreamline.h"
#include "vtkHyperStreamlineDTMRI.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkFloatArray.h"

#include "teem/ten.h"
#include "teem/nrrd.h"

class VTK_SLICERTRACTOGRAPHYSEEDING_EXPORT vtkHyperStreamlineTeem : public vtkHyperStreamlineDTMRI
{

 public:
  vtkTypeRevisionMacro(vtkHyperStreamlineTeem,vtkHyperStreamlineDTMRI);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkHyperStreamlineTeem *New();

 protected:
  vtkHyperStreamlineTeem();
  ~vtkHyperStreamlineTeem();

  void Execute();
  void StartFiberFrom( const vtkFloatingPointType position[3], tenFiberContext *fibercontext );
  void VisualizeFibers( const Nrrd *fibers );

 private:
  tenFiberContext *ProduceFiberContext();
  bool DatasetOrSettingsChanged();
};
#endif
