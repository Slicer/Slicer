#ifndef __vtkVolumeRenderingLogic_h
#define __vtkVolumeRenderingLogic_h
#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeRendering.h"
#include "vtkVolumeMapper.h"
#include "vtkVolume.h"

#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumeRenderingSelectionNode.h"

class vtkSlicerVolumeTextureMapper3D;
class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkSlicerGPURayCastVolumeTextureMapper3D;
class vtkCudaVolumeMapper;

class vtkRenderer;
class vtkTimerLog;

class VTK_SLICERVOLUMERENDERING1_EXPORT vtkVolumeRenderingLogic :public vtkSlicerModuleLogic
{
public:
  static vtkVolumeRenderingLogic *New();
  vtkTypeMacro(vtkVolumeRenderingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};

  
  // Overload the scene setter, this is needed for module creation
  // via LoadableModuleFactory
  virtual void SetMRMLScene(vtkMRMLScene *scene);

  // Register local MRML nodes
  virtual void RegisterNodes();
  
  vtkMRMLVolumeRenderingSelectionNode* GetSelectionNode();

  vtkMRMLVolumeRenderingParametersNode* GetParametersNode();

  // Description:
  // Update MRML events
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ ); 

  double EstimateSampleDistances(void);

protected:
  vtkVolumeRenderingLogic();
  ~vtkVolumeRenderingLogic();
  vtkVolumeRenderingLogic(const vtkVolumeRenderingLogic&);
  void operator=(const vtkVolumeRenderingLogic&);

  static bool First;

  // Description:
  // The hardware accelerated texture mapper.
  vtkSlicerVolumeTextureMapper3D *MapperTexture;    
  
  // Description:
  // The hardware accelerated gpu ray cast mapper.
  vtkCudaVolumeMapper *MapperCUDARaycast;

  // Description:
  // The hardware accelerated gpu ray cast mapper.
  vtkSlicerGPURayCastVolumeTextureMapper3D *MapperGPURaycast;

  // Description:
  // The software accelerated software mapper
  vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

  // Description:
  // The current volume  mapper
  vtkVolumeMapper *CurrentVolumeMapper;

  //BTX
  std::map<std::string, vtkVolumeMapper *> VolumeMappers;
  //ETX

  // Description:
  // Actor used for Volume Rendering
  vtkVolume *Volume;

};

#endif
