// .NAME vtkSlicerLabelMapWidget - Widget that holds all GUI elements used for volume rendering and labelmaps 
// .SECTION Description
// Widget that holds all GUI elements used for volume rendering and labelmaps 
#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerLabelmapTree.h"
#include "vtkKWVolumeMaterialPropertyWidget.h"
#include "vtkMRMLVolumeRenderingNode.h"

class vtkKWTreeWithScrollbars;
class vtkMRMLScalarVolumeNode;
class vtkSlicerLabelmapElement;
class vtkLabelMapPiecewiseFunction;



class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkSlicerWidget
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    void UpdateGuiElements(void);
    static vtkSlicerLabelMapWidget *New();
    vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);

    // Description:
    // Initialize the Labelmap widget. Use this method directly after creating the widget and 
    // before using any other method. 
    // scalarNode and vrNode: nodes that include information to display the widget. 
    void Init(vtkMRMLScalarVolumeNode *scalarNode,vtkMRMLVolumeRenderingNode *vrnode)
    {
        if(this->VolumeRenderingNode==vrnode&&this->Node==scalarNode)
        {
            vtkErrorMacro("Init already called, call UpdateGUIElements instead");
            return;
        }
        if(scalarNode==NULL||vrnode==NULL)
        {
            vtkErrorMacro("NULL is no allowed");
        }
        this->VolumeRenderingNode=vrnode;
        this->Node=scalarNode;
        if(this->VMPW_Shading!=NULL)
        {
            this->VMPW_Shading->SetVolumeProperty(this->VolumeRenderingNode->GetVolumeProperty());
        }
        if(this->Tree!=NULL)
        {
            this->Tree->Init(this->Node,this->VolumeRenderingNode);
        }


    }

    // Description:
    // Create the Widget.
    virtual void CreateWidget();

    // Description:
    // Update the VolumeRenderingNode. Attention: Init has to be used, before using this method
    // Note: An update of the scalarNode is not possible, one must create and init a new widget.
    void UpdateVolumeRenderingNode(vtkMRMLVolumeRenderingNode *vrNode)
    {
        if(this->Tree!=NULL)
        {
            this->Tree->UpdateVolumeRenderingNode(vrNode);
        }
        else
        {
            vtkErrorMacro("Init has to be used before Update is called");
        }
    }
    // Description:
    // Get the vtkMRMLVolumeRenderingNode
    vtkGetObjectMacro(VolumeRenderingNode,vtkMRMLVolumeRenderingNode);
    // Description:
    // Get the vtkMRMLScalarVolumeNode
    vtkGetObjectMacro(Node,vtkMRMLScalarVolumeNode);

    //BTX
    // Description:
    // Enum for Events
    enum
    {
        // Description:
        // A call of Render is necessary
        NeedForRenderEvent=30000,
    };
    //ETX
protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerLabelMapWidget(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerLabelMapWidget(void);

    // Description:
    // Associated VolumeRenderingNode, used for opacities and colormapping
    vtkMRMLVolumeRenderingNode *VolumeRenderingNode;

    // Description:
    // Associated ScalarVolumeNode, used to keep GUI small as only relevant labels are shown.
    // The ScalarVolumeNode has to be a labelmap.
    vtkMRMLScalarVolumeNode *Node;

    //GUI ELEMENTS

    // Description:
    // labelmap tree which allows to change the opacity of every label
    vtkSlicerLabelmapTree *Tree;

    // Description:
    // labelmaptreeElement which allows to change all opacities all at once
    vtkSlicerLabelmapElement *ChangeAll;

    // Description:
    // change the shading
    vtkKWVolumeMaterialPropertyWidget *VMPW_Shading;

    //METHODS
    // Description:
    // Process the GUI events from the widget
    virtual void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);


private:
    // Description:
    // Caution: Not implemented
    vtkSlicerLabelMapWidget(const vtkSlicerLabelMapWidget&);
    void operator=(const vtkSlicerLabelMapWidget&);
};
#endif
