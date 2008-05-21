// .NAME vtkSlicerNodeSelectorVolumeRenderingWidget - Extends vtkSlicerNodeSelector for Volume Rendering issues
// .SECTION Description
// Allows the filtering of the NodeSelectorWidget. Concrete this means, adding of an additional MRML Scene for Presets and
// checking for conditions like referenced ID labelmap or not etc.
#ifndef __vtkSlicerNodeSelectorVolumeRenderingWidget_h
#define __vtkSlicerNodeSelectorVolumeRenderingWidget_h

#include "vtkSlicerModuleGUI.h"
#include "vtkVolumeRendering.h"
#include "vtkSlicerNodeSelectorWidget.h"

class VTK_SLICERVOLUMERENDERING_EXPORT vtkSlicerNodeSelectorVolumeRenderingWidget :public vtkSlicerNodeSelectorWidget
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerNodeSelectorVolumeRenderingWidget* New();
    vtkTypeRevisionMacro(vtkSlicerNodeSelectorVolumeRenderingWidget,vtkSlicerNodeSelectorWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    //BTX
    // Description:
    // Set condition under which the nodes from the additional mrml scene will be added or not.
    // con: id of the referenced node
    // isLm: Show only labelmaps or non labelmaps
    // modeCon: Check if the conditions are true or false.
    void SetCondition(std::string con,int isLm, bool modeCon)
    {
        this->IsLabelmap=isLm;
        this->Condition=con;
        this->ModeCondition=modeCon;
    }

    // Description:
    // Get the condition string (which node is referenced.
    std::string GetCondition(void)
    {
        return this->Condition;
    }
    //ETX

    // Description:
    // Set an additional MRML Scene which will also be shown in vtkSlicerNodeSelectorWidget.
    void SetAdditionalMRMLScene(vtkMRMLScene *scene)
    {
        this->AdditionalMRMLScene=scene;
    }

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerNodeSelectorVolumeRenderingWidget();

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerNodeSelectorVolumeRenderingWidget();


    //BTX
    // Description:
    // Referenced id of vtkMRMLVolumeRenderingNode that is to be checked.
    std::string Condition;
    //ETX

    //Description:
    //if 1 use ==for condition if 0 use != for condition
    bool ModeCondition;

    // Description:
    // Does the node have to be a labelmap or not
    int IsLabelmap;

    // Description:
    // Additional MRML scene which nodes will be added in addition to the MRML scene
    // of the vtkSlicerNodeSelectorWidget
    vtkMRMLScene *AdditionalMRMLScene;

    // Description:
    // Check for additional volume rendering specific issues.
    virtual  bool CheckAdditionalConditions(vtkMRMLNode *node);

    // Description:
    // Add additional nodes for volume rendering.
    virtual int AddAditionalNodes();

    // Description:
    // Check if the selected node is part of the additional nodes.
    virtual vtkMRMLNode* GetSelectedInAdditional();

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerNodeSelectorVolumeRenderingWidget(const vtkSlicerNodeSelectorVolumeRenderingWidget&);//not implemented
    void operator=(const vtkSlicerNodeSelectorVolumeRenderingWidget&);//not implemented

};
#endif
