#include "vtkSlicerNodeSelectorVolumeRenderingWidget.h"
#include "vtkMRMLVolumeRenderingNode.h"

vtkStandardNewMacro( vtkSlicerNodeSelectorVolumeRenderingWidget );
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
    vtkMRMLVolumeRenderingNode *currentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(node);
    if(ModeCondition)
    {
            return currentNode->HasReference(this->Condition);
    }
    return !currentNode->HasReference(this->Condition);

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

    //I think i don't need this because presets cannot be added
    int count = 0;
    //int c=0;

    //if (this->NewNodeEnabled)
    //{
    //  for (c=0; c < this->GetNumberOfNodeClasses(); c++)
    //  {
    //    const char *name = this->GetNodeName(c);
    //    if (name == NULL || !strcmp(name, "") )
    //      {
    //      name = this->MRMLScene->GetTagByClassName(this->GetNodeClass(c));
    //      }

    //    std::stringstream ss;
    //    ss << "Create New " << this->MRMLScene->GetTagByClassName(this->GetNodeClass(c));

    //    // Build the command.  Since node name can contain spaces, we
    //    // need to quote the node name in the constructed Tcl command
    //    std::stringstream sc;
    //    sc << "ProcessNewNodeCommand " << this->GetNodeClass(c) << " \"" << name << "\"";

    //    this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(ss.str().c_str());
    //    this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str() );
    //    this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
    //  }
    //}

  /*  if (this->NoneEnabled) 
      {
      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("None");
      this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, "ProcessCommand None");
      }*/

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
        //Whats this?!
        //if (!this->GetChildClassesEnabled() && strcmp(node->GetClassName(), className) != 0)
        //  {
        //  continue;
        //  }

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
