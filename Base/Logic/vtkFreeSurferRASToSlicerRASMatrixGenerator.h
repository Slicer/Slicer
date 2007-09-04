#ifndef __vtkFreeSurferRASToSlicerRASMatrixGenerator_h
#define __vtkFreeSurferRASToSlicerRASMatrixGenerator_h

#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkSlicerBaseLogicWin32Header.h"

class vtkMRMLVolumeNode;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkFreeSurferRASToSlicerRASMatrixGenerator : public vtkObject
{
public:
  static vtkFreeSurferRASToSlicerRASMatrixGenerator *New();
  vtkTypeRevisionMacro(vtkFreeSurferRASToSlicerRASMatrixGenerator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( RAS2RASMatrix, vtkMatrix4x4);

  // Description:
  // Computes matrix we need to register
  // V1Node to V2Node given the
  // tkregister.dat matrix from FreeSurfer
  void ComputeRAS2RASMatrix( vtkMRMLVolumeNode *V1Node,
                      vtkMRMLVolumeNode *V2Node,
                      vtkMatrix4x4 *FSRegistrationmat );
  void ComputeRAS2RASMatrixWithOrientation( vtkMRMLVolumeNode *V1Node,
                      vtkMRMLVolumeNode *V2Node,
                      vtkMatrix4x4 *FSRegistrationmat );
  void ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                             vtkMatrix4x4 *M );
  void ComputeOrientationMatrix ( vtkMRMLVolumeNode *VNode,
                                  vtkMatrix4x4 *M );

protected:
  vtkFreeSurferRASToSlicerRASMatrixGenerator();
  ~vtkFreeSurferRASToSlicerRASMatrixGenerator();

  vtkMatrix4x4 *RAS2RASMatrix;
  
private:
  vtkFreeSurferRASToSlicerRASMatrixGenerator(const vtkFreeSurferRASToSlicerRASMatrixGenerator&);  // Not implemented.
  void operator=(const vtkFreeSurferRASToSlicerRASMatrixGenerator&);  // Not implemented.
};


#endif


