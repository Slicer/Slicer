// .NAME vtkSlicerLabelmapTree - Extends vtkSlicerBaseTree with labelmap specific methods and behavior 
// .SECTION Description
// vtkSlicerLabelmapTree extends vtkSlicerBaseTree with labelmap specific behavior for Volume Rendering. This means that 
// the tree is especially linked to one vtkMRMLScalerVolumeNode and a vtkMRMLVolumeRenderingNode.
// vtkSlicerLabelmapTree is intended to use it together with vtkSlicerLabelmapElement.

#ifndef __vtkSlicerLabelmapTree_h
#define __vtkSlicerLabelmapTree_h
#include "vtkSlicerBaseTree.h"
#include "vtkVolumeRenderingModule.h"
#include <vector>
#include <string>

class vtkSlicerLabelmapElement;
class vtkMRMLScalarVolumeNode;
class vtkLabelMapPiecewiseFunction;
class vtkMRMLVolumeRenderingNode;



class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapTree : public vtkSlicerBaseTree
{
public:

    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerLabelmapTree *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapTree,vtkSlicerBaseTree);
    virtual void CreateWidget(void);

    // Description:
    // Initialize the Labelmap tree. Use this method directly after creating the widget and 
    // before using any other method. 
    // scalarNode and vrNode: nodes that include information to display the widget. 
    void Init(vtkMRMLScalarVolumeNode *scalarnode,vtkMRMLVolumeRenderingNode *vrnode);

    // Description:
    //UpdateTheGuiElements
    void UpdateGuiElements(void);

    // Description:
    // Change all opacities to the specific stage.
    void ChangeAllOpacities(int stage);

    // Description:
    // Update the saved volume Rendering Node. Use init before calling this method.
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
    // Description:
    // Event invoked everytime a single label is edited
    enum
    {
        SingleLabelEdited = 12000,
    };
    //ETX
        
    // Description:
    // Factor how to get from opacity to stage. E. G. Opacity 0.10=>01.*20= Stage 2
    //BTX
    static const int FACTOR_OPACITY_TO_STAGE=20;
    //ETX


protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerLabelmapTree(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerLabelmapTree(void);

    //BTX
    //Description:
    //What was the RecentColorNodeID, (do we have to change colors and labels and range?!)
    std::string RecentColorNodeID;
    //ETX



    // Description:
    // Flag to know if an update is part of a change of all opacities.
    int InChangeOpacityAll;

    // Description:
    // The VolumeRenderingNode linked to the Labelmap Tree. Used e.g. for scalar opacity
    vtkMRMLVolumeRenderingNode *VolumeRenderingNode;


    // Description:
    // The ScalarVolumeNode linked to the labelmap tree. Used e.g. for ColorNode Information
    vtkMRMLScalarVolumeNode *ScalarVolumeNode;
    //BTX

    // Description:
    // Pointers to all vtkSlicerLabelmapElements used in the tree.
    std::vector<vtkSlicerLabelmapElement*> Elements;
    //ETX

    // Description:
    // labelmap tree specific events.
    virtual void ProcessBaseTreeEvents(vtkObject *caller, unsigned long eid, void *callData);

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerLabelmapTree(const vtkSlicerLabelmapTree&);//not implemented
    void operator=(const vtkSlicerLabelmapTree&);//not implemented
};

#endif 
