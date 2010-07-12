#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerModelDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

// for scalars
#include "vtkPointData.h"
#include "vtkCellData.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerModelDisplayWidget, "$Revision$");



//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::vtkSlicerModelDisplayWidget ( )
{

    this->ModelDisplayNode = NULL;
    this->ModelNode = NULL;
    this->ModelHierarchyNode = NULL;
    this->ModelHierarchyLogic = NULL;

    this->SelectedButton = NULL;
    this->VisibilityButton = NULL;
    
    this->ScalarVisibilityButton = NULL;
    this->AutoScalarRangeCheckButton = NULL;
    this->MinScalarRangeEntry = NULL;
    this->MaxScalarRangeEntry = NULL;
    this->ScalarMenu = NULL;
    this->ColorSelectorWidget = NULL;
    
    this->ClippingButton = NULL;
    this->SliceIntersectionVisibilityButton = NULL;
    this->BackfaceCullingButton = NULL;
    this->OpacityScale = NULL;
    this->SurfaceMaterialPropertyWidget = NULL;
    this->ProcessingMRMLEvent = 0;
    this->ProcessingWidgetEvent = 0;

    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;
}


//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::~vtkSlicerModelDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();
  this->SetModelHierarchyLogic(NULL);

  if (this->SelectedButton)
    {
    this->SelectedButton->SetParent(NULL);
    this->SelectedButton->Delete();
    this->SelectedButton = NULL;
    }
  if (this->VisibilityButton)
    {
    this->VisibilityButton->SetParent(NULL);
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }
  if (this->ScalarVisibilityButton)
    {
    this->ScalarVisibilityButton->SetParent(NULL);
    this->ScalarVisibilityButton->Delete();
    this->ScalarVisibilityButton = NULL;
    }
  if (this->AutoScalarRangeCheckButton)
    {
    this->AutoScalarRangeCheckButton->SetParent(NULL);
    this->AutoScalarRangeCheckButton->Delete();
    this->AutoScalarRangeCheckButton = NULL;
    }
  if (this->MinScalarRangeEntry)
    {
    this->MinScalarRangeEntry->SetParent(NULL);
    this->MinScalarRangeEntry->Delete();
    this->MinScalarRangeEntry = NULL;
    }
  if (this->MaxScalarRangeEntry)
    {
    this->MaxScalarRangeEntry->SetParent(NULL);
    this->MaxScalarRangeEntry->Delete();
    this->MaxScalarRangeEntry = NULL;
    }
  if (this->ScalarMenu)
    {
    this->ScalarMenu->SetParent(NULL);
    this->ScalarMenu->Delete();
    this->ScalarMenu = NULL;
    }
   if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  if (this->ClippingButton)
    {
    this->ClippingButton->SetParent(NULL);
    this->ClippingButton->Delete();
    this->ClippingButton = NULL;
    }
  if (this->SliceIntersectionVisibilityButton)
    {
    this->SliceIntersectionVisibilityButton->SetParent(NULL);
    this->SliceIntersectionVisibilityButton->Delete();
    this->SliceIntersectionVisibilityButton = NULL;
    }
   if (this->BackfaceCullingButton)
    {
    this->BackfaceCullingButton->SetParent(NULL);
    this->BackfaceCullingButton->Delete();
    this->BackfaceCullingButton = NULL;
    }
  if (this->OpacityScale)
    {
    this->OpacityScale->SetParent(NULL);
    this->OpacityScale->Delete();
    this->OpacityScale = NULL;
    }
  if (this->SurfaceMaterialPropertyWidget)
    {
    this->SurfaceMaterialPropertyWidget->SetParent(NULL);
    this->SurfaceMaterialPropertyWidget->Delete();
    this->SurfaceMaterialPropertyWidget = NULL;
    }
  if (this->ChangeColorButton)
    {
    this->ChangeColorButton->SetParent(NULL);
    this->ChangeColorButton->Delete();
    this->ChangeColorButton= NULL;
    }
  vtkSetAndObserveMRMLNodeMacro(this->ModelNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->ModelHierarchyNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->ModelDisplayNode, NULL);
  this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerModelDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::SetModelDisplayNode ( vtkMRMLModelDisplayNode *node )
{ 
  // 
  // Set the member variables and do a first process
  //
  vtkSetAndObserveMRMLNodeMacro(this->ModelDisplayNode, node);

  if ( node )
    {
    this->UpdateWidget();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::SetModelNode ( vtkMRMLModelNode *node )
{ 
  // 
  // Set the member variables and do a first process
  //
  vtkSetAndObserveMRMLNodeMacro(this->ModelNode, node);

  if ( node )
    {
    this->UpdateWidget();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::SetModelHierarchyNode ( vtkMRMLModelHierarchyNode *node )
{ 
  // 
  // Set the member variables and do a first process
  //
  vtkSetAndObserveMRMLNodeMacro(this->ModelHierarchyNode, node);

  if ( node )
    {
    this->UpdateWidget();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::ProcessWidgetEvents(vtkObject *caller,
                                                      unsigned long event,
                                                      void *vtkNotUsed(callData))
{
  if (this->ProcessingMRMLEvent != 0 || this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing " << this->ProcessingMRMLEvent);
    return;
    }
  
  this->ProcessingWidgetEvent = event;
 
   if (this->ModelDisplayNode != NULL && 
       (!(vtkKWSurfaceMaterialPropertyWidget::SafeDownCast(caller) == this->SurfaceMaterialPropertyWidget && static_cast<int>(event) == this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent()))
       &&
       (!(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && static_cast<int>(event) == vtkKWScale::ScaleValueChangingEvent))
       &&
       (!(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && static_cast<int>(event) == vtkKWScale::ScaleValueChangedEvent)))
    {
    if (this->MRMLScene->GetNodeByID(this->ModelDisplayNode->GetID()))
      {
      this->MRMLScene->SaveStateForUndo(this->ModelDisplayNode);
      }
    }

   if (event == vtkKWCheckButton::SelectedStateChangedEvent)
     {
     vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast(caller);
     if (cb && cb == this->AutoScalarRangeCheckButton->GetWidget())
       {
       // if it's turned on, reset the scalar min max, it will be passed on to
       // the node in UpdateMRML
       if (this->AutoScalarRangeCheckButton->GetWidget()->GetSelectedState())
         {
         if (this->ModelDisplayNode->GetPolyData() &&
             this->ModelDisplayNode->GetPolyData()->GetScalarRange())
           {
           double *scalarRange = this->ModelDisplayNode->GetPolyData()->GetScalarRange();
           vtkDebugMacro("ProcessWidgetEvents: Scalar range for display node polydata = " << scalarRange[0] << ", " << scalarRange[1]);
           this->MinScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[0]);
           this->MaxScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[1]);
           }
         else
           {
           this->MinScalarRangeEntry->GetWidget()->SetValueAsDouble(0.0);
           this->MaxScalarRangeEntry->GetWidget()->SetValueAsDouble(1.0);
           }
         }
       }
     }

   if (event == vtkKWMenu::MenuItemInvokedEvent)
     {
     vtkKWMenu *menu = vtkKWMenu::SafeDownCast(caller);
     if (menu && menu == this->ScalarMenu->GetWidget()->GetMenu())
       {
       // update the min/max range fields if in auto mode
       if (this->AutoScalarRangeCheckButton->GetWidget()->GetSelectedState())
         {
         if (this->ModelDisplayNode->GetPolyData() &&
             this->ModelDisplayNode->GetPolyData()->GetPointData() &&
             this->ModelDisplayNode->GetPolyData()->GetPointData()->GetScalars(this->ScalarMenu->GetWidget()->GetValue()))
           {
           //double *scalarRange =
           //this->ModelDisplayNode->GetPolyData()->GetScalarRange();
           // it's not set yet as the active one, so reach down and get the range
           double *scalarRange =  this->ModelDisplayNode->GetPolyData()->GetPointData()->GetScalars(this->ScalarMenu->GetWidget()->GetValue())->GetRange();
           vtkDebugMacro("ProcessWidgetEvents: Menu item invoked, scalar range for display node polydata on array " << this->ScalarMenu->GetWidget()->GetValue() << " = " << scalarRange[0] << ", " << scalarRange[1]);
           this->MinScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[0]);
           this->MaxScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[1]);
           }
         }
       }
     }
  this->UpdateMRML();

  this->ProcessingWidgetEvent = 0;

}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::UpdateMRML()
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;

  int selected = this->SelectedButton->GetWidget()->GetSelectedState();
  if (this->ModelNode)
    {
    vtkDebugMacro("UpdateMRML: setting model node's selected state to: " << selected);
    this->ModelNode->SetSelected(selected);
    }
  // update children selected?
  
  if ( this->ModelDisplayNode )
    {
    vtkDebugMacro("UpdateMRML: setting model display node selected state to: " << selected);
    this->ModelDisplayNode->SetSelected(selected);
    
    int visibility = this->VisibilityButton->GetWidget()->GetSelectedState();
    this->ModelDisplayNode->SetVisibility(visibility);
    // update hierarchy children visibility 
    if (this->ModelHierarchyNode != NULL && this->ModelNode == NULL)
      {
      vtkMRMLDisplayNode *dnode = this->ModelHierarchyNode->GetDisplayNode();
      if (dnode)
        {
        dnode->SetVisibility(visibility);
        }
      
      // change children visibility 
      std::vector< vtkMRMLModelHierarchyNode *> childrenNodes;
      this->ModelHierarchyLogic->GetHierarchyChildrenNodes(this->ModelHierarchyNode, childrenNodes);
      for (unsigned int i=0; i<childrenNodes.size(); i++)
        {
        vtkMRMLModelHierarchyNode *cnode = childrenNodes[i];
        vtkMRMLDisplayNode *cdnode = cnode->GetDisplayNode();
        if (cdnode)
          {
          cdnode->SetVisibility(visibility);
          }
        vtkMRMLModelNode *mnode = cnode->GetModelNode();
        if (mnode)
          {
          vtkMRMLDisplayNode *mdnode = mnode->GetDisplayNode();
          if (mdnode)
            {
            mdnode->SetVisibility(visibility);
            }
          }
        } //for
     } //if
    
    
    this->ModelDisplayNode->SetScalarVisibility(this->ScalarVisibilityButton->GetWidget()->GetSelectedState());
    this->ModelDisplayNode->SetAutoScalarRange(this->AutoScalarRangeCheckButton->GetWidget()->GetSelectedState());
    double min = this->MinScalarRangeEntry->GetWidget()->GetValueAsDouble();
    double max = this->MaxScalarRangeEntry->GetWidget()->GetValueAsDouble();
    this->ModelDisplayNode->SetScalarRange(min, max);
    // get the value of the button, it's the selected item in the menu
    this->ModelDisplayNode->SetActiveScalarName(this->ScalarMenu->GetWidget()->GetValue());
    vtkDebugMacro("Set display node active scalar name to " << this->ModelDisplayNode->GetActiveScalarName());
    if (this->ColorSelectorWidget->GetSelected() != NULL)
      {
      vtkMRMLColorNode *color =
        vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
      if ((color != NULL &&
           this->ModelDisplayNode->GetColorNodeID() == NULL) ||
          strcmp(this->ModelDisplayNode->GetColorNodeID(), color->GetID()) != 0)
        {
        // there's a change, set it
        vtkDebugMacro("UpdateMRML: setting the display node's color node to " << color->GetID());
        this->ModelDisplayNode->SetAndObserveColorNodeID(color->GetID());
        }
      }
    this->ModelDisplayNode->SetClipping(this->ClippingButton->GetWidget()->GetSelectedState());
    this->ModelDisplayNode->SetSliceIntersectionVisibility(this->SliceIntersectionVisibilityButton->GetWidget()->GetSelectedState());
    this->ModelDisplayNode->SetBackfaceCulling(this->BackfaceCullingButton->GetWidget()->GetSelectedState());
    this->ModelDisplayNode->SetOpacity(this->OpacityScale->GetWidget()->GetValue());
    if (this->SurfaceMaterialPropertyWidget->GetProperty() == NULL)
      {
      vtkProperty *prop = vtkProperty::New();
      this->SurfaceMaterialPropertyWidget->SetProperty(prop);
      prop->Delete();
      }

    this->ModelDisplayNode->SetAmbient(this->SurfaceMaterialPropertyWidget->GetProperty()->GetAmbient());
    this->ModelDisplayNode->SetDiffuse(this->SurfaceMaterialPropertyWidget->GetProperty()->GetDiffuse());
    this->ModelDisplayNode->SetSpecular(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecular());
    this->ModelDisplayNode->SetPower(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecularPower());
    double *rgb = this->ChangeColorButton->GetColor();
    double *rgb1 = ModelDisplayNode->GetColor();
    if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
        fabs(rgb[1]-rgb1[1]) > 0.001 ||
        fabs(rgb[2]-rgb1[2]) > 0.001)
      {
      this->ModelDisplayNode->SetColor(this->ChangeColorButton->GetColor());
      }
    }
  this->UpdatingMRML = 0;

}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::ProcessMRMLEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData))
{
  if ( !this->ModelDisplayNode )
    {
    return;
    }
  if (this->ProcessingMRMLEvent != 0 || this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing " << this->ProcessingMRMLEvent);
    return;
    }
  
  this->ProcessingMRMLEvent = event;
  

  if (this->ModelDisplayNode == vtkMRMLModelDisplayNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidget();
    }
  this->ProcessingMRMLEvent = 0;
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::UpdateWidget()
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }
  this->UpdatingWidget = 1;
  
  if ( this->ModelDisplayNode == NULL )
    {
    this->UpdatingWidget = 0;
    return;
    }

  // get the model node so can get at it's scalars
  if (this->ModelNode != NULL &&
      this->ModelNode->GetPolyData() != NULL)
    {
    this->ScalarVisibilityButton->SetEnabled(1);
    this->AutoScalarRangeCheckButton->SetEnabled(1);
    this->MinScalarRangeEntry->SetEnabled(1);
    this->MaxScalarRangeEntry->SetEnabled(1);
    this->ScalarMenu->SetEnabled(1);
    this->ColorSelectorWidget->SetEnabled(1);

    // populate the scalars menu from the model node
    int numPointScalars;
    int numCellScalars;
    if (this->ModelNode->GetPolyData() &&
        this->ModelNode->GetPolyData()->GetPointData() != NULL)
      {
      numPointScalars = this->ModelNode->GetPolyData()->GetPointData()->GetNumberOfArrays();
      }
    else
      {
      numPointScalars = 0;
      }
    if (this->ModelNode->GetPolyData() &&
        this->ModelNode->GetPolyData()->GetCellData() != NULL)
      {
      numCellScalars = this->ModelNode->GetPolyData()->GetCellData()->GetNumberOfArrays();
      }
    else
      {
      numCellScalars = 0;
      }
    vtkDebugMacro("numPointScalars = " << numPointScalars << ", numCellScalars = " << numCellScalars);
    this->ScalarMenu->GetWidget()->GetMenu()->DeleteAllItems();
    // clear the button text
    this->ScalarMenu->GetWidget()->SetValue("");
    for (int p = 0; p < numPointScalars; p++)
      {
      vtkDebugMacro("Adding point scalar " << p << " " << this->ModelNode->GetPolyData()->GetPointData()->GetArray(p)->GetName());
      this->ScalarMenu->GetWidget()->GetMenu()->AddRadioButton(this->ModelNode->GetPolyData()->GetPointData()->GetArray(p)->GetName());
      }
    for (int c = 0; c < numCellScalars; c++)
      {
      vtkDebugMacro("Adding cell scalar " << c << " " << this->ModelNode->GetPolyData()->GetCellData()->GetArray(c)->GetName());
      this->ScalarMenu->GetWidget()->GetMenu()->AddRadioButton(this->ModelNode->GetPolyData()->GetCellData()->GetArray(c)->GetName());
      }
    } 
  else 
    { 
    this->ScalarVisibilityButton->SetEnabled(0);
    this->AutoScalarRangeCheckButton->SetEnabled(0);
    this->MinScalarRangeEntry->SetEnabled(0);
    this->MaxScalarRangeEntry->SetEnabled(0);
    this->ScalarMenu->SetEnabled(0);
    this->ColorSelectorWidget->SetEnabled(0);
    vtkDebugMacro("ModelNode is null, can't set up the scalars menu\n"); 
    }

  if (this->ModelNode)
    {
    vtkDebugMacro("UpdateWidget: Updating SelectedButton from model node's selected value: " << this->ModelNode->GetSelected());
    this->SelectedButton->GetWidget()->SetSelectedState(this->ModelNode->GetSelected());
    }
  // for now, let the display node's selected over ride the model node's flag
  this->SelectedButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetSelected());

  this->VisibilityButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetVisibility());
  this->ScalarVisibilityButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetScalarVisibility());
  this->AutoScalarRangeCheckButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetAutoScalarRange());
  double* scalarRange = this->ModelDisplayNode->GetScalarRange();
  if (scalarRange)
    {
    this->MinScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[0]);
    this->MaxScalarRangeEntry->GetWidget()->SetValueAsDouble(scalarRange[1]);
    }
  // set the active one if it's not already set
  this->ScalarMenu->GetWidget()->GetMenu()->SelectItem(this->ModelDisplayNode->GetActiveScalarName());
  // get the color node
  if (this->ModelDisplayNode->GetColorNode() != NULL)
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color == NULL ||
        strcmp(this->ModelDisplayNode->GetColorNodeID(), color->GetID()) != 0)
      {
      this->ColorSelectorWidget->SetSelected(this->ModelDisplayNode->GetColorNode());
      }
    }
  else
    {
    // clear the selection
    this->ColorSelectorWidget->SetSelected(NULL);
    }
  this->ClippingButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetClipping());
  this->SliceIntersectionVisibilityButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetSliceIntersectionVisibility());
  this->BackfaceCullingButton->GetWidget()->SetSelectedState(this->ModelDisplayNode->GetBackfaceCulling());
  this->OpacityScale->GetWidget()->SetValue(this->ModelDisplayNode->GetOpacity());
  if (this->SurfaceMaterialPropertyWidget->GetProperty() == NULL)
    {
    vtkProperty *prop = vtkProperty::New();
    this->SurfaceMaterialPropertyWidget->SetProperty(prop);
    prop->Delete();
    }
  
  this->SurfaceMaterialPropertyWidget->GetProperty()->SetAmbient(this->ModelDisplayNode->GetAmbient());
  this->SurfaceMaterialPropertyWidget->GetProperty()->SetDiffuse(this->ModelDisplayNode->GetDiffuse());
  this->SurfaceMaterialPropertyWidget->GetProperty()->SetSpecular(this->ModelDisplayNode->GetSpecular());
  this->SurfaceMaterialPropertyWidget->GetProperty()->SetSpecularPower(this->ModelDisplayNode->GetPower());
  double *rgb = this->ChangeColorButton->GetColor();
  double *rgb1 = ModelDisplayNode->GetColor();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->ChangeColorButton->SetColor(this->ModelDisplayNode->GetColor());
    }

  this->SurfaceMaterialPropertyWidget->Update();
  this->UpdatingWidget = 0;

}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::RemoveMRMLObservers ( )
{
  if (this->ModelDisplayNode)
    {
    this->ModelDisplayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                                            (vtkCommand *)this->MRMLCallbackCommand );
    }
  
  if (this->ModelNode)
    {
    this->ModelNode->RemoveObservers(vtkCommand::ModifiedEvent,
                                     (vtkCommand *)this->MRMLCallbackCommand );
    }
  
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::RemoveWidgetObservers ( ) {
  this->SelectedButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ScalarVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AutoScalarRangeCheckButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MinScalarRangeEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
  this->MaxScalarRangeEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
  this->ScalarMenu->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ClippingButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceIntersectionVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BackfaceCullingButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ChangeColorButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );
  
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  // ---
  // DISPLAY FRAME            
  vtkKWFrame *modelDisplayFrame = vtkKWFrame::New ( );
  modelDisplayFrame->SetParent ( this->GetParent() );
  modelDisplayFrame->Create ( );
/*
  modelDisplayFrame->SetLabelText ("Display");
  modelDisplayFrame->CollapseFrame ( );
*/
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 modelDisplayFrame->GetWidgetName() );

  // Don't select child classes (like FiberBundles)
  //this->ModelSelectorWidget->ChildClassesEnabledOff();

  this->SelectedButton = vtkKWCheckButtonWithLabel::New();
  this->SelectedButton->SetParent ( modelDisplayFrame );
  this->SelectedButton->Create ( );
  this->SelectedButton->SetLabelText("Selected");
  this->SelectedButton->SetBalloonHelpString("set model selected (very much under construction)");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SelectedButton->GetWidgetName() );
  
  this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->VisibilityButton->SetParent ( modelDisplayFrame );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Visibility");
  this->VisibilityButton->SetBalloonHelpString("set model visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );

  // a frame to hold the scalar related widgets
  vtkKWFrameWithLabel *scalarFrame = vtkKWFrameWithLabel::New();
  scalarFrame->SetParent( modelDisplayFrame );
  scalarFrame->Create();
  scalarFrame->SetLabelText("Scalars:");
  this->Script("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 scalarFrame->GetWidgetName(),
                 modelDisplayFrame->GetWidgetName());

  vtkKWFrame *scalarVisibFrame = vtkKWFrame::New();
  scalarVisibFrame->SetParent(scalarFrame->GetFrame());
  scalarVisibFrame->Create();
  this->Script("pack %s -side top -anchor nw -fill x -pady 0",
               scalarVisibFrame->GetWidgetName());
  
  // scalar visibility
  this->ScalarVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->ScalarVisibilityButton->SetParent ( scalarVisibFrame );
  this->ScalarVisibilityButton->Create ( );
  this->ScalarVisibilityButton->SetLabelText("Scalar Visibility");
  this->ScalarVisibilityButton->SetBalloonHelpString("set model scalar visibility.");
  //this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
  //               this->ScalarVisibilityButton->GetWidgetName() );

  
  // a menu of the scalar fields available to be set
  this->ScalarMenu = vtkKWMenuButtonWithLabel::New();
  this->ScalarMenu->SetParent ( scalarVisibFrame );
  this->ScalarMenu->Create();
  this->ScalarMenu->SetLabelText("Set Active Scalar:");
  this->ScalarMenu->SetBalloonHelpString("set which scalar field is displayed on the model");
  this->ScalarMenu->GetWidget()->SetWidth(20);
  // pack the scalars frame
  this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
               this->ScalarVisibilityButton->GetWidgetName(),
               this->ScalarMenu->GetWidgetName());

  // scalar use auto range
  this->AutoScalarRangeCheckButton = vtkKWCheckButtonWithLabel::New();
  this->AutoScalarRangeCheckButton->SetParent ( scalarFrame->GetFrame() );
  this->AutoScalarRangeCheckButton->Create ( );
  this->AutoScalarRangeCheckButton->SetLabelText("Auto Scalar Range");
  this->AutoScalarRangeCheckButton->SetBalloonHelpString("Use automatic scalar range, resets min/max scalar range from the active scalar array. Currently, min/max scalar range is still used when checked, use this to reset the range to the full range.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->AutoScalarRangeCheckButton->GetWidgetName());

  // scalar range
  this->MinScalarRangeEntry = vtkKWEntryWithLabel::New();
  this->MinScalarRangeEntry->SetParent(scalarFrame->GetFrame());
  this->MinScalarRangeEntry->Create();
  this->MinScalarRangeEntry->SetLabelText("Min Scalar Range");
  //this->MinScalarRangeEntry->SetWidth ( 8 );
  this->MinScalarRangeEntry->GetWidget()->SetValueAsDouble(0.0);
  this->MinScalarRangeEntry->SetBalloonHelpString("Set the minimum scalar range to display from the currently active scalar array. Changes here do not work with color transfer function scalar color maps (for example, FreeSurfer color maps)");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
               this->MinScalarRangeEntry->GetWidgetName());

  this->MaxScalarRangeEntry = vtkKWEntryWithLabel::New();
  this->MaxScalarRangeEntry->SetParent( scalarFrame->GetFrame() );
  this->MaxScalarRangeEntry->Create();
  this->MaxScalarRangeEntry->SetLabelText("Max Scalar Range");
  //this->MaxScalarRangeEntry->SetWidth ( 8 );
  this->MaxScalarRangeEntry->GetWidget()->SetValueAsDouble(1.0);
  this->MaxScalarRangeEntry->SetBalloonHelpString("Set the maximum scalar range to display from the currently active scalar array. Changes here do not work with color transfer function scalar color maps (for example, FreeSurfer color maps)");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
               this->MaxScalarRangeEntry->GetWidgetName());
  
  // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( scalarFrame->GetFrame() );
  this->ColorSelectorWidget->Create ( );
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->ColorSelectorWidget->AddExcludedChildClass("vtkMRMLDiffusionTensorDisplayPropertiesNode");
  this->ColorSelectorWidget->ShowHiddenOn();
  this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ColorSelectorWidget->SetBorderWidth(2);
  // this->ColorSelectorWidget->SetReliefToGroove();
  this->ColorSelectorWidget->SetPadX(2);
  this->ColorSelectorWidget->SetPadY(2);
  this->ColorSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ColorSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->ColorSelectorWidget->SetLabelText( "Scalar Color Map Select: ");
  this->ColorSelectorWidget->SetBalloonHelpString("select a color node from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ColorSelectorWidget->GetWidgetName());
  
  this->ClippingButton = vtkKWCheckButtonWithLabel::New();
  this->ClippingButton->SetParent ( modelDisplayFrame );
  this->ClippingButton->Create ( );
  this->ClippingButton->SetLabelText("Clipping");
  this->ClippingButton->SetBalloonHelpString("set model clipping with RGB slice planes.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ClippingButton->GetWidgetName() );

  this->SliceIntersectionVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->SliceIntersectionVisibilityButton->SetParent ( modelDisplayFrame );
  this->SliceIntersectionVisibilityButton->Create ( );
  this->SliceIntersectionVisibilityButton->SetLabelText("Slice Intersections Visible");
  this->SliceIntersectionVisibilityButton->SetBalloonHelpString("Show model intersection on slice planes.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SliceIntersectionVisibilityButton->GetWidgetName() );

  this->BackfaceCullingButton = vtkKWCheckButtonWithLabel::New();
  this->BackfaceCullingButton->SetParent ( modelDisplayFrame );
  this->BackfaceCullingButton->Create ( );
  this->BackfaceCullingButton->SetLabelText("Backface Culling");
  this->BackfaceCullingButton->SetBalloonHelpString("set model back face culling.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->BackfaceCullingButton->GetWidgetName() );
  
  this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent ( modelDisplayFrame );
  this->OpacityScale->Create ( );
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->GetWidget()->SetRange(0,1);
  this->OpacityScale->GetWidget()->SetResolution(0.1);
  this->OpacityScale->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->OpacityScale->GetWidgetName() );

  this->ChangeColorButton = vtkKWChangeColorButton::New();
  this->ChangeColorButton->SetParent ( modelDisplayFrame );
  this->ChangeColorButton->Create ( );
  this->ChangeColorButton->SetColor(0.0, 1.0, 0.0);
  this->ChangeColorButton->LabelOutsideButtonOn();
  this->ChangeColorButton->SetLabelPositionToRight();
  this->ChangeColorButton->SetBalloonHelpString("set model color.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ChangeColorButton->GetWidgetName() );

  this->SurfaceMaterialPropertyWidget = vtkKWSurfaceMaterialPropertyWidget::New();
  this->SurfaceMaterialPropertyWidget->SetParent ( modelDisplayFrame );
  this->SurfaceMaterialPropertyWidget->Create ( );
  this->SurfaceMaterialPropertyWidget->SetBalloonHelpString("set model surface properties.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SurfaceMaterialPropertyWidget->GetWidgetName() );

  // add observers
  
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SelectedButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ScalarVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AutoScalarRangeCheckButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MinScalarRangeEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
  this->MaxScalarRangeEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
    
  this->ScalarMenu->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ClippingButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceIntersectionVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BackfaceCullingButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );

  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
   
  modelDisplayFrame->Delete();
  scalarVisibFrame->Delete();
  scalarFrame->Delete();

}

