#ifndef __vtkSlicerLabelmapTree_h
#define __vtkSlicerLabelmapTree_h
#include "vtkSlicerBaseTree.h"
#include "vtkVolumeRenderingModule.h"
#include <vector>

class vtkSlicerLabelmapElement;
class vtkMRMLScalarVolumeNode;
class vtkLabelMapPiecewiseFunction;
class vtkMRMLVolumeRenderingNode;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapTree : public vtkSlicerBaseTree
{
public:
    static vtkSlicerLabelmapTree *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapTree,vtkSlicerBaseTree);
    void Init(vtkMRMLScalarVolumeNode *scalarnode,vtkMRMLVolumeRenderingNode *vrnode);
    void UpdateGuiElements(void);
    virtual void CreateWidget(void);
    void ChangeAllOpacities(int stage);
    void UpdateVolumeRenderingNode(vtkMRMLVolumeRenderingNode *vrNode)
      {
          if(this->VolumeRenderingNode!=NULL)
          {
              this->VolumeRenderingNode=vrNode;
          }
          else
          {
              vtkErrorMacro("Init has to be used before Update is called");
          }
      }
    //BTX
    enum
    {
        SingleLabelEdited = 12000,
    };
    //ETX

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerLabelmapTree(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerLabelmapTree(void);


    int StepSize;
    int InChangeOpacityAll;
    vtkMRMLVolumeRenderingNode *VolumeRenderingNode;
    vtkMRMLScalarVolumeNode *ScalarVolumeNode;
    //BTX
    std::vector<vtkSlicerLabelmapElement*> Elements;
    //ETX


    virtual void ProcessBaseTreeEvents(vtkObject *caller, unsigned long eid, void *callData);

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerLabelmapTree(const vtkSlicerLabelmapTree&);//not implemented
    void operator=(const vtkSlicerLabelmapTree&);//not implemented
};
#endif 
