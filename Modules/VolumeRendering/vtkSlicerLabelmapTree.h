#ifndef __vtkSlicerLabelmapTree_h
#define __vtkSlicerLabelmapTree_h
#include "vtkSlicerBaseTree.h"
#include "vtkVolumeRenderingModule.h"
#include <vector>

class vtkSlicerLabelmapElement;
class vtkMRMLScalarVolumeNode;
class vtkLabelMapPiecewiseFunction;
 
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapTree : public vtkSlicerBaseTree
{
public:
    static vtkSlicerLabelmapTree *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapTree,vtkSlicerBaseTree);
    void Init(vtkMRMLScalarVolumeNode *node,vtkLabelMapPiecewiseFunction *piecewiseFunction);
    virtual void CreateWidget(void);
    void ChangeAllOpacities(int stage);

protected:
    vtkLabelMapPiecewiseFunction *PiecewiseFunction;
    vtkMRMLScalarVolumeNode *Node;
    //BTX
    std::vector<vtkSlicerLabelmapElement*> Elements;
    //ETX
    ~vtkSlicerLabelmapTree(void);
    vtkSlicerLabelmapTree(void);
    vtkSlicerLabelmapTree(const vtkSlicerLabelmapTree&);//not implemented
    void operator=(const vtkSlicerLabelmapTree&);//not implemented
    virtual void ProcessBaseTreeEvents(vtkObject *caller, unsigned long eid, void *callData);


};
#endif 
