#ifndef __vtkSlicerVRLabelmapHelper_h
#define __vtkSlicerVRLabelmapHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerVRHelper.h"

class vtkSlicerLabelMapWidget;
class vtkFixedPointVolumeRayCastMapper;
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVRLabelmapHelper :public vtkSlicerVRHelper
{
public:
    static vtkSlicerVRLabelmapHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVRLabelmapHelper,vtkSlicerVRHelper);
    virtual void InitializePipelineNewCurrentNode();
    virtual void Rendering(void);
    virtual void UpdateRendering(void);
protected:
    vtkSlicerVRLabelmapHelper(void);
    ~vtkSlicerVRLabelmapHelper(void);
    vtkSlicerVRLabelmapHelper(const vtkSlicerVRLabelmapHelper&);//not implemented
    void operator=(const vtkSlicerVRLabelmapHelper&);//not implemented


    vtkSlicerLabelMapWidget *LM_OptionTree;
    vtkFixedPointVolumeRayCastMapper *MapperRaycast;

    void UpdateLM();
};
#endif
