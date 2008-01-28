#include "vtkSlicerNodeSelectorVolumeRenderingWidget.h"
#include "vtkMRMLVolumeRenderingNode.h"

vtkStandardNewMacro( vtkSlicerNodeSelectorVolumeRenderingWidget );
vtkCxxRevisionMacro (vtkSlicerNodeSelectorVolumeRenderingWidget, "$Revision: 1.0 $");
vtkSlicerNodeSelectorVolumeRenderingWidget::vtkSlicerNodeSelectorVolumeRenderingWidget(void)
{
    //Condition is not set
    this->Condition="";
    this->AdditionalMRMLScene=NULL;
}

vtkSlicerNodeSelectorVolumeRenderingWidget::~vtkSlicerNodeSelectorVolumeRenderingWidget(void)
{
}

bool vtkSlicerNodeSelectorVolumeRenderingWidget::CheckAdditionalConditions(vtkMRMLNode *node)
{
    if(strcmp(this->Condition.c_str(),"")==0)
    {
        return true;
    }
    vtkMRMLVolumeRenderingNode *CurrentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(node);

    bool ret=CurrentNode->HasReference(this->Condition);
    //Enable this back for Labelmaps
    //bool retA=(CurrentNode->GetIsLabelMap()==this->IsLabelmap);
    //if(ModeCondition)
    //{
    //    return (ret&&retA);
    //}
    ////Else we have to take care about the preset widget;
    ////We can have a preset, retA has to be true, ret to be false
    ////or another Node: retA has to be true, ret to be false
    //return (retA&&!ret);

    if(ModeCondition)
    {
        return ret;
    }
    return !ret;

}

int vtkSlicerNodeSelectorVolumeRenderingWidget::AddAditionalNodes()
{
    if ( !this || !this->AdditionalMRMLScene )
    {
        return 0;
    }
    vtkMRMLNode *oldSelectedNode = this->GetSelected();
    this->ClearMenu();

    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();

    //I think i don't need this because Presets cannot be added
    int count = 0;
    vtkMRMLNode *node = NULL;
    vtkMRMLNode *selectedNode = NULL;
    bool selected = false;
    this->AdditionalMRMLScene->InitTraversal();
    while ( (node = this->AdditionalMRMLScene->GetNextNode()) != NULL)
    {
        if (!node->GetSelectable())
        {
            continue;
        }
        if (!this->ShowHidden && node->GetHideFromEditors())
        {
            continue;
        }

        // If there is a Attribute Name-Value  specified, then only include nodes that
        // match both the NodeClass and Attribute
        if (this->CheckAdditionalConditions(node))
        {
            std::stringstream sc;
            sc << "ProcessCommand " << node->GetID();

            this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(node->GetName());
            // do we need a column break?
            if (count != 0 && count % 30 == 0)
            {
                this->GetWidget()->GetWidget()->GetMenu()->SetItemColumnBreak(count, 1);
            }
            this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str());
            if (oldSelectedNode == node)
            {
                selectedNode = node;
                selected = true;
            }
            else if (!selected && !this->NoneEnabled)
            {  
                selectedNode = node;
                selected = true;
            }
        }
    }
    //after all add a seperator
    this->GetWidget()->GetWidget()->GetMenu()->AddSeparator();
    //count++;
    if (selectedNode != NULL)
    {
        this->GetWidget()->GetWidget()->SetValue(selectedNode->GetName());
        this->SelectedID = std::string(selectedNode->GetID());
    }
    else
    {
        char *name = "";
        if (this->NoneEnabled)
        {
            name = "None";
        }
        this->GetWidget()->GetWidget()->SetValue(name);
        this->SelectedID = std::string(name);
    }

    if (oldSelectedNode != selectedNode)
    {
        this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
    return count;
}

vtkMRMLNode* vtkSlicerNodeSelectorVolumeRenderingWidget::GetSelectedInAdditional()
{
    if(this->AdditionalMRMLScene==NULL)
    {
        return NULL;
    }
    return this->AdditionalMRMLScene->GetNodeByID (this->SelectedID.c_str());
}

void vtkSlicerNodeSelectorVolumeRenderingWidget::PrintSelf(std::ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
    os<<indent<<"Condition"<<this->Condition;
    os<<indent<<"IsLabelmap"<<this->IsLabelmap;
}
