#ifndef __vtkSlicerLabelmapTree_h
#define __vtkSlicerLabelmapTree_h
#include "vtkSlicerBaseTree.h"
#include "vtkVolumeRenderingModule.h"

class vtkMRMLScalarVolumeNode;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapTree : public vtkSlicerBaseTree
{
public:
        static vtkSlicerLabelmapTree *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapTree,vtkSlicerBaseTree);
    void Init(vtkMRMLScalarVolumeNode *node);
    virtual void CreateWidget(void);

protected:
    vtkMRMLScalarVolumeNode *Node;
    ~vtkSlicerLabelmapTree(void);
        vtkSlicerLabelmapTree(void);
              vtkSlicerLabelmapTree(const vtkSlicerLabelmapTree&);//not implemented
    void operator=(const vtkSlicerLabelmapTree&);//not implemented
};
#endif 
