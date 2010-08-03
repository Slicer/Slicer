 #include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMRMLTreeWidget.h"

#include "vtkMatrix4x4.h"

#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLArrayPlotNode.h"
#include "vtkMRMLXYPlotManagerNode.h"

#include "vtkSlicerXYPlotWidget.h"
#include "vtkSlicerApplication.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLTreeWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLTreeWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::vtkSlicerMRMLTreeWidget ( )
{
  this->TreeWidget = NULL;
  this->ContextMenu = NULL;
  this->NodeID = NULL;
  this->NodeName = NULL;
  this->ShowIDButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::~vtkSlicerMRMLTreeWidget ( )
{
  this->RemoveMRMLObservers();
  if (this->TreeWidget)
    {
    this->TreeWidget->SetParent(NULL);
    this->TreeWidget->Delete();
    this->TreeWidget = NULL;
    }

  if (this->ContextMenu)
    {
    this->ContextMenu->SetParent(NULL);
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }

  if (this->NodeID)
    {
    this->NodeID->Delete();
    this->NodeID = NULL;
    }

  if (this->NodeName)
    {
    this->NodeName->Delete();
    this->NodeName = NULL;
    }

  if (this->ShowIDButton)
    {
    this->ShowIDButton->Delete();
    this->ShowIDButton = NULL;
    }

  this->ClearCutNodes();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerMRMLTreeWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  vtkKWTree *tree = this->TreeWidget->GetWidget();
  vtkKWEntry *entry = this->NodeName->GetWidget();
  if (caller == tree)
    {
    // Selection changed

    vtksys_stl::string selected(
    this->TreeWidget->GetWidget()->GetSelection());

    //std::cout << "Selected = " << selected << "\n";

    if (event == vtkKWTree::SelectionChangedEvent)
      {
      // For example, one could populate the node inspector
      this->SetSelectesLeaves();
      vtkMRMLNode *selectedNode = this->GetSelectedNodeInTree();
      this->UpdateNodeInspector(selectedNode);
      this->UpdateNodeInTree(selectedNode);
      }

    // Right click: context menu

    else if (event == vtkKWTree::RightClickOnNodeEvent)
      {

      if (!this->ContextMenu)
        {
        this->ContextMenu = vtkKWMenu::New();
        }
      if (!this->ContextMenu->IsCreated())
        {
        this->ContextMenu->SetParent(this);
        this->ContextMenu->Create();
        }
      this->ContextMenu->DeleteAllItems();

      int px, py;
      vtkKWTkUtilities::GetMousePointerCoordinates(tree, &px, &py);

      char command[125];
      
      vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID((const char *)callData);

      if (!this->IsLeafSelected((const char *)callData))
        {
        this->TreeWidget->GetWidget()->ClearSelection();
        this->TreeWidget->GetWidget()->SelectNode((const char *)callData);
        this->SelectedLeaves.clear();
        this->SelectedLeaves.push_back((const char *)callData);
        }

      if (this->SelectedLeaves.size() > 1)
        {
        // multiple nodes selected
        sprintf(command, "CutNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Cut Node", this, command);

        sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Delete Node", this, command);
        }
      else {
        // single node selected
        if ( node == NULL || (node != NULL && node->IsA("vtkMRMLTransformNode")) )
          {
          // scene or transform
          sprintf(command, "InsertTransformNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Insert Transform Node", this, command);
          }
        if ((node == NULL || (node != NULL && node->IsA("vtkMRMLTransformNode"))) &&
          this->CutNodes.size() > 0)
          {
          // scene or transform and cut node exists
          sprintf(command, "PasteNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Paste Node", this, command);
          }
        if (node != NULL && node->IsA("vtkMRMLTransformableNode") )
          {
          sprintf(command, "CutNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Cut Node", this, command);
    
          sprintf(command, "SelectNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Edit Properties...", this, command);
          }

        if (node != NULL && (node->IsA("vtkMRMLTransformableNode")))
          {
          vtkMRMLTransformableNode* tbnode = vtkMRMLTransformableNode::SafeDownCast(node);
          vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(tbnode->GetTransformNodeID()));
          if (tbnode->GetParentTransformNode())
            {
            // avoid hardening nonlinear transforms onto volume nodes or linear transform nodes
            if (!(!tnode->CanApplyNonLinearTransforms() && !tnode->IsTransformToWorldLinear()))
              {
              sprintf(command, "HardenTransformCallback {%s}", (const char *)callData);
              this->ContextMenu->AddCommand("Harden Transforms", this, command);
              }
            }
          }
        if (node != NULL && node->IsA("vtkMRMLDisplayableNode") && 
            !node->IsA("vtkMRMLVolumeNode") && 
            !node->IsA("vtkMRMLTransformNode") &&
            vtkMRMLDisplayableNode::SafeDownCast(node)->GetNumberOfDisplayNodes() == 1 &&
            vtkMRMLDisplayableNode::SafeDownCast(node)->GetDisplayNode() != NULL )
          {
          sprintf(command, "ToggleVisibilityCallback {%s}", (const char *)callData);
          int index = this->ContextMenu->AddCheckButton("Visibility", this, command);
          this->ContextMenu->SetItemSelectedState(index, vtkMRMLDisplayableNode::SafeDownCast(node)->GetDisplayNode()->GetVisibility());
          }
        if (node != NULL && node->IsA("vtkMRMLDoubleArrayNode"))
          {
          sprintf(command, "PlotCallback {%s}", (const char *)callData);
          /*int index = */ this->ContextMenu->AddCommand("Plot", this, command);
          }
        if (node != NULL)
          {
          sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Delete Node", this, command);
          }
        }
      this->ContextMenu->PopUp(px, py);
      }
    }
  else if (caller == entry) 
    {
    if (event == vtkKWEntry::EntryValueChangedEvent) 
      {
      //
      // value changed events are generated for a wide variety of reasons - 
      // so confirm that the selected node is the same one that was displayed
      // in the node inspector and confirm that the name has actually changed before
      // triggering events
      //
      char *inspectedNodeID = this->NodeID->GetWidget()->GetText();
      const char *inspectedNodeNewName = entry->GetValue();
      vtkMRMLNode *node = this->GetSelectedNodeInTree();

      if ( node && 
            !strcmp( node->GetID(), inspectedNodeID ) && 
            strcmp( node->GetName(), inspectedNodeNewName ) )
        {
        node->SetName(entry->GetValue());
        this->UpdateTreeFromMRML();
        this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
        }
      }

    }
  else if (event == vtkKWCheckButton::SelectedStateChangedEvent && 
          this->ShowIDButton->GetWidget() == vtkKWCheckButton::SafeDownCast(caller) )
    {
    this->UpdateTreeFromMRML();
    }

} 

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::DeleteNodeCallback(const char * vtkNotUsed(id))
{
  // delete node, then repopulate tree
  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(this->SelectedLeaves[i].c_str());
    if (node != NULL)
      {
      vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
      std::string message = std::string("Delete ") + std::string(node->GetName()) + std::string("?"); 
      if ( vtkKWMessageDialog::PopupYesNo( 
            app, this->ContextMenu->GetParentTopLevel(), 
            "Delete Node", message.c_str(),
            vtkKWMessageDialog::WarningIcon | 
            vtkKWMessageDialog::InvokeAtPointer) )
        {
        this->GetMRMLScene()->RemoveNode(node);
        }
      }
    }
  this->TreeWidget->GetWidget()->ClearSelection();
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PasteNodeCallback(const char *id)
{
  vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  for (unsigned int i=0; i< this->CutNodes.size(); i++)
    {
    vtkMRMLTransformableNode *node = this->CutNodes[i];
    if (node != NULL)
      {
      if (tnode != NULL)
        {
        node->SetAndObserveTransformNodeID(tnode->GetID());
        }
      this->GetMRMLScene()->AddNode(node);
      }
    }
  this->ClearCutNodes();
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::InsertTransformNodeCallback(const char *id)
{
  vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  vtkSmartPointer<vtkMRMLLinearTransformNode> node = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
  this->GetMRMLScene()->AddNodeNoNotify(node);
  if (tnode != NULL)
    {
    node->SetAndObserveTransformNodeID(tnode->GetID());
    }
  node->SetName(node->GetID());
  this->UpdateTreeFromMRML();
  this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CutNodeCallback(const char *vtkNotUsed(id))
{
  //cout << "I want to delete MRML node " << id << endl;
  this->ClearCutNodes();

  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID((const char *)this->SelectedLeaves[i].c_str());
    vtkMRMLTransformableNode *tnode = vtkMRMLTransformableNode::SafeDownCast(node);
    tnode->Register(this);
    this->CutNodes.push_back(tnode );
    this->GetMRMLScene()->RemoveNode(node);
    tnode->SetAndObserveTransformNodeID(NULL);
    }
  this->TreeWidget->GetWidget()->ClearSelection();
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::SelectNodeCallback(const char *id)
{
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(id);
  if (node != NULL)
    {
    this->InvokeEvent(vtkSlicerMRMLTreeWidget::SelectedEvent, node);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PlotCallback(const char * vtkNotUsed(id))
{
  // plot the node
  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(this->SelectedLeaves[i].c_str());
    if (node != NULL && node->IsA("vtkMRMLDoubleArrayNode"))
      {
        vtkMRMLDoubleArrayNode *dnode = vtkMRMLDoubleArrayNode::SafeDownCast(node);

        vtkSmartPointer<vtkMRMLArrayPlotNode> plot = vtkSmartPointer<vtkMRMLArrayPlotNode>::New();
        this->GetMRMLScene()->AddNode(plot);
        plot->SetAndObserveArray(dnode);
        plot->SetColor(1, 0, 0);
        
        
        vtkSmartPointer<vtkMRMLXYPlotManagerNode> manager = vtkSmartPointer<vtkMRMLXYPlotManagerNode>::New();
        this->GetMRMLScene()->AddNode(manager);
        manager->AddPlotNode(plot);
        vtkMRMLDoubleArrayNode::LabelsVectorType labels = dnode->GetLabels();
        manager->SetXLabel(labels.at(0).c_str());
        manager->SetYLabel(labels.at(1).c_str());
        manager->SetErrorBarAll(1);
        manager->SetBackgroundColor(0.8,0.8,1);
        
        vtkKWTopLevel *top = vtkKWTopLevel::New();
        top->SetApplication(this->GetApplication());
        top->Create();
        
        vtkSmartPointer<vtkSlicerXYPlotWidget> widget = vtkSmartPointer<vtkSlicerXYPlotWidget>::New();
        widget->SetParent(top);
        widget->SetAndObservePlotManagerNode(manager);
        widget->Create();
        
        this->Script ( "pack %s -fill both -expand true",
                   widget->GetWidgetName() );

        widget->UpdateGraph();
       
        top->SetSize(400, 200);
        top->Display();

        // top->Delete(); do I need to keep this around to have it display?

        // what about the widget? when do I delete that? Or will the toplevel delete it?
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::NodeParentChangedCallback(
  const char *nodeID, const char *parentID, const char*)
{
  if (!strcmp(parentID, "Scene"))
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
    if (node != NULL)
      {
      vtkMRMLTransformNode *tnode = node->GetParentTransformNode();
      if (tnode != NULL)
        {
        node->SetAndObserveTransformNodeID(NULL);
        node->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
        }
      }
    }
  else 
    {
    vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(parentID));
    if (tnode != NULL)
      {
      vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
      if (node != NULL)
        {
        if (tnode != NULL && tnode != node)
          {
          node->SetAndObserveTransformNodeID(tnode->GetID());
          node->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
          }
        }
      }
    }
  this->UpdateTreeFromMRML();
}
 
//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ToggleVisibilityCallback(const char *id)
{
  vtkMRMLDisplayableNode *dnode = 
    vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  if (dnode)
    {
    vtkMRMLDisplayNode *dnode_node = dnode->GetDisplayNode();
    if (dnode_node)
      {
      this->VisibilityCallback(id, dnode_node->GetVisibility() ? 0 : 1);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::VisibilityCallback(const char *id, int vis)
{
  vtkMRMLDisplayableNode *dnode = 
    vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  vtkMRMLDisplayNode *displayNode;
  if (dnode)
    {
    for (int i = 0; i < dnode->GetNumberOfDisplayNodes(); i++)
      {
      displayNode = dnode->GetNthDisplayNode(i);
      if ( displayNode )
        {
        dnode->GetNthDisplayNode(i)->SetVisibility(vis);
        }
      else
        {
        vtkErrorMacro("GetNthDisplayNode returns NULL, even for value of i less than GetNumberOfDisplayNodes");
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::HardenTransformCallback(const char *id)
{
  vtkMRMLTransformableNode* tbnode = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!tbnode)
    {
    return;
    }

  vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(tbnode->GetTransformNodeID()));

  if (!tnode)
    {
    return;
    }

  if (!tnode->IsTransformToWorldLinear() && !tbnode->CanApplyNonLinearTransforms())
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (  this->GetParent() );
    dialog->SetStyleToMessage();
    std::string msg = std::string("Can't apply non linear transform to the node: ") +
      std::string(tbnode->GetName()) +
      std::string(", of type: ") + 
      std::string(tbnode->GetClassName());
    dialog->SetText(msg.c_str());
    dialog->Create( );
    vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
    dialog->SetMasterWindow( app->GetApplicationGUI()->GetMainSlicerWindow() );

    dialog->ModalOn();
    dialog->Invoke();
    dialog->Delete();

    //vtkErrorMacro(<<"Can't apply non linear transform to this node.");
    return;
    }

  this->GetMRMLScene()->SaveStateForUndo();

  if (tnode->IsTransformToWorldLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> hardeningMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    tnode->GetMatrixTransformToWorld(hardeningMatrix);
    tbnode->ApplyTransform(hardeningMatrix);
    }
  else
    {
    vtkSmartPointer<vtkGeneralTransform> hardeningTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    tnode->GetTransformToWorld(hardeningTransform);
    tbnode->ApplyTransform(hardeningTransform);
    }

  tbnode->SetAndObserveTransformNodeID(NULL);
  tbnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
  tbnode->SetModifiedSinceRead(1);
  this->UpdateTreeFromMRML();
}
 
//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::AddMRMLObservers ( )
{
  if (this->MRMLScene == NULL)
    {
    return;
    }
  if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::SceneEditedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
  else
    {
    vtkDebugMacro("MRML scene already has the node removed event being watched by the SceneSnapshotWidget");
    }
}


//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::RemoveMRMLObservers ()
{
  if ( this->MRMLScene == NULL )
    {
    return;
    }
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneEditedEvent, (vtkCommand *)this->MRMLCallbackCommand);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  vtkMRMLNode *caller_node = reinterpret_cast<vtkMRMLNode *>(caller);
  vtkMRMLNode *calldata_node = reinterpret_cast<vtkMRMLNode *>(callData);
  vtkMRMLTransformableNode *calldata_tnode = 
    vtkMRMLTransformableNode::SafeDownCast(calldata_node);
  vtkMRMLCameraNode *camera_node = vtkMRMLCameraNode::SafeDownCast(calldata_node);
  vtkMRMLViewNode *view_node = vtkMRMLViewNode::SafeDownCast(calldata_node);
  if (this->MRMLScene &&
      ((event == vtkMRMLScene::SceneClosedEvent) || 
       (vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene && 
        (calldata_tnode || view_node || camera_node))))
    {
    this->UpdateTreeFromMRML();
    }
  else if (this->MRMLScene && (event == vtkMRMLScene::NodeAddedEvent) && 
     calldata_tnode)
    {
    this->UpdateTreeFromMRML();
    }
  else if ( this->MRMLScene && (event == vtkMRMLScene::SceneEditedEvent) )
    {
    this->UpdateTreeFromMRML();
    }
  else if (caller_node  && event == vtkMRMLDisplayableNode::DisplayModifiedEvent) 
    {
    this->UpdateNodeInTree(caller_node);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::RemoveWidgetObservers ( ) 
{
  if (this->TreeWidget) 
    {
    this->TreeWidget->GetWidget()->RemoveObservers(
      vtkKWTree::SelectionChangedEvent, 
      (vtkCommand *)this->GUICallbackCommand);  
 
    this->TreeWidget->GetWidget()->RemoveObservers(
      vtkKWTree::RightClickOnNodeEvent, 
      (vtkCommand *)this->GUICallbackCommand);  
    }

  if (this->ShowIDButton)
    {
    this->ShowIDButton->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->NodeName)
    {
    this->NodeName->GetWidget()->RemoveObservers(  vtkKWEntry::EntryValueChangedEvent,
                                                 (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CreateWidget ( )
{
  // Check if already created
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // MRML Tree

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("MRML Tree");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  this->TreeWidget = vtkKWTreeWithScrollbars::New() ;
  this->TreeWidget->SetParent ( frame->GetFrame() );
  this->TreeWidget->VerticalScrollbarVisibilityOn();
  this->TreeWidget->HorizontalScrollbarVisibilityOff();
  //this->TreeWidget->ResizeButtonsVisibilityOn();
  
  this->TreeWidget->Create ( );
  this->TreeWidget->SetBalloonHelpString("MRML Tree");
  ///  this->TreeWidget->SetBorderWidth(2);
  //this->TreeWidget->SetReliefToGroove();
  this->Script ( "pack %s -side top -anchor nw -expand y -fill both -padx 2 -pady 2",
                 this->TreeWidget->GetWidgetName());

  vtkKWTree *tree = this->TreeWidget->GetWidget();
  tree->SelectionFillOn();
  tree->SetSelectionModeToMultiple ();
  tree->SetNodeParentChangedCommand(this, "NodeParentChangedCallback");
  tree->EnableReparentingOn();
  //tree->SetSelectionModeToSingle();
  tree->SetHeight(12);

  tree->AddObserver(
    vtkKWTree::SelectionChangedEvent, 
    (vtkCommand *)this->GUICallbackCommand );

  tree->AddObserver(
    vtkKWTree::RightClickOnNodeEvent, 
    (vtkCommand *)this->GUICallbackCommand);


  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( frame->GetFrame() );
  f->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 f->GetWidgetName() );
                 
  this->ShowIDButton = vtkKWCheckButtonWithLabel::New();
  this->ShowIDButton->SetParent ( f );
  this->ShowIDButton->Create ( );
  this->ShowIDButton->SetLabelPositionToRight();
  this->ShowIDButton->ExpandWidgetOff();
  this->ShowIDButton->GetLabel()->SetAnchorToCenter();
  this->ShowIDButton->SetLabelText(" Display MRML ID's");
  this->ShowIDButton->SetBalloonHelpString("Display MRML ID's.");
  this->Script ( "pack %s -side top -anchor w -expand n -padx 0 -pady 0",
                 this->ShowIDButton->GetWidgetName());
  this->ShowIDButton->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->UpdateTreeFromMRML();
  f->Delete();
  frame->Delete();

  // MRML Node Inspector

  frame = vtkKWFrameWithLabel::New();
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("MRML Node Inspector");
  frame->CollapseFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  int label_width = 10;

  this->NodeID = vtkKWLabelWithLabel::New() ;
  this->NodeID->SetParent(frame->GetFrame());
  this->NodeID->SetLabelText("ID:");
  this->NodeID->Create();
  this->NodeID->GetWidget()->SetAnchorToWest();
  this->NodeID->SetLabelWidth(label_width);
  this->NodeID->SetBalloonHelpString("MRML Node ID");
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->NodeID->GetWidgetName());

  this->NodeName = vtkKWEntryWithLabel::New() ;
  this->NodeName->SetParent(frame->GetFrame());
  this->NodeName->SetLabelText("Name:");
  this->NodeName->Create();
  this->NodeName->SetLabelWidth(label_width);
  this->NodeName->SetBalloonHelpString("MRML Node Name");
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->NodeName->GetWidgetName());

  this->NodeName->GetWidget()->AddObserver(  vtkKWEntry::EntryValueChangedEvent,
                                            (vtkCommand *)this->GUICallbackCommand );

  frame->Delete();

  this->AddMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateTreeFromMRML()
  
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());
  this->TreeWidget->GetWidget()->DeleteAllNodeWindows("root");
  this->TreeWidget->GetWidget()->DeleteAllNodes();

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;

  // create Root node
  this->TreeWidget->GetWidget()->AddNode(NULL, "Scene", "Scene");
  int nnodes = scene->GetNumberOfNodes();
  for (int n=0; n<nnodes; n++)
    {
    node = scene->GetNthNode(n);
    this->AddNodeToTree(node);
    }

  // check that the selected node is still in the tree
  if (this->TreeWidget->GetWidget()->HasNode(selected_node.c_str()))
    {
    this->TreeWidget->GetWidget()->SelectNode(selected_node.c_str());
    }
  else
    {
    if (selected_node != "")
      {
      vtkWarningMacro("Selected node no longer in tree: " << selected_node.c_str());
      }
    }
  // At this point you probably want to reset the MRML node inspector fields
  // in case nothing in the tree is selected anymore (here, we delete all nodes
  // each time, so nothing will be selected, but it's not a bad thing to 
  // try to save the old selection, or just update the tree in a smarter
  // way).

  this->TreeWidget->GetWidget()->OpenFirstNode ();

  this->UpdateNodeInspector(this->GetSelectedNodeInTree());
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::AddNodeToTree(vtkMRMLNode *node)
{
  if (!node || node->GetHideFromEditors())
   {
    return;
   }

  vtkKWTree *tree = this->TreeWidget->GetWidget();
  char *ID = node->GetID();
  if (tree->HasNode(ID))
    {
    return;
    }
    
  vtksys_stl::string node_text(node->GetName());
  if (node_text.size())
    {
    if (this->ShowIDButton && 
        this->ShowIDButton->GetWidget()->GetSelectedState())
      {
      node_text += " (";
      node_text += ID;
      node_text += ")";
      }
    }
  else
    {
    node_text = ID;
    }
  /*
  node_text += ": ";
  node_text += node->GetClassName();
  */
  
  const char *parent_node = "Scene";
  int isTransformable = node->IsA("vtkMRMLTransformableNode");
  if (isTransformable)
    {
    vtkMRMLTransformableNode *transformableNode = 
      vtkMRMLTransformableNode::SafeDownCast(node);
    vtkMRMLTransformableNode *parent = 
      transformableNode->GetParentTransformNode();
    if (parent)
      {
      parent_node = parent->GetID();
      this->AddNodeToTree(parent);
      }
    }
  tree->AddNode(parent_node, ID, node_text.c_str());

  this->UpdateNodeInTree(node);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateNodeInTree(vtkMRMLNode *node)
{
  if (!node || node->GetHideFromEditors())
   {
    return;
   }

  vtkKWTree *tree = this->TreeWidget->GetWidget();
  char *ID = node->GetID();
  if (!tree->HasNode(ID))
    {
    return;
    }

  // This is how you can set icons for each node. You can either use
  // predefined icons, or go through the usual resources framework, just
  // like the toolbar icons are constructed (see SetNodeImage...)

  if (node->IsA("vtkMRMLDisplayableNode") && 
      !node->IsA("vtkMRMLVolumeNode") && 
      !node->IsA("vtkMRMLTransformNode")  &&
      vtkMRMLDisplayableNode::SafeDownCast(node)->GetNumberOfDisplayNodes() == 1 )
    {
    vtkMRMLDisplayNode *display_node = 
      vtkMRMLDisplayableNode::SafeDownCast(node)->GetDisplayNode();
    if (display_node != NULL)
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(
        this->GetApplication());
      vtkKWCheckButton *vis_button = vtkKWCheckButton::SafeDownCast(
        tree->GetNodeWindow(ID));
      if (!vis_button) 
        {
        node->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent, 
                          (vtkCommand *)this->MRMLCallbackCommand);
        char command[255];
        vis_button = vtkKWCheckButton::New();
        vis_button->SetParent(tree);
        vis_button->Create();
        vis_button->SetBorderWidth(0);
        vis_button->SetIndicatorVisibility(0);
        vtkSlicerFoundationIcons *icons = 
          app->GetApplicationGUI()->GetSlicerFoundationIcons();
        vis_button->SetImageToIcon(
          icons->GetSlicerInvisibleNoFrameIcon());
        vis_button->SetSelectImageToIcon(
          icons->GetSlicerVisibleNoFrameIcon());
        sprintf(command, "VisibilityCallback {%s}", (const char*) ID);
        vis_button->SetCommand(this, command);
        tree->SetNodeWindow(ID, vis_button);
        tree->SetNodePadX(ID, 20);
        vis_button->Delete();
        }

      vis_button->SetBackgroundColor(tree->GetBackgroundColor());
      vis_button->SetActiveBackgroundColor(tree->GetBackgroundColor());

      int vis = display_node->GetVisibility();
      vis_button->SetSelectedState(vis);
      }
    }
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerMRMLTreeWidget::GetSelectedNodeInTree()
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL, *first_selected_node = NULL;
  int nnodes = scene->GetNumberOfNodes();
  for (int n=0; n<nnodes; n++)
    {
    node = scene->GetNthNode(n);
    if (!strcmp(node->GetID(), selected_node.c_str()))
      {
      first_selected_node = node;
      break;
      }
    }
  return first_selected_node;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateNodeInspector(vtkMRMLNode *node)
{
  if (this->NodeID)
    {
    this->NodeID->GetWidget()->SetText(
      node ? node->GetID() : "");
    }

  if (this->NodeName)
    {
    this->NodeName->GetWidget()->SetValue(
      node ? node->GetName() : "");
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::SetSelectesLeaves()
{
  this->SelectedLeaves.clear();

  vtksys_stl::string selectedLeaves(
    this->TreeWidget->GetWidget()->GetSelection());

  vtksys_stl::string::size_type locStart = 0;
  vtksys_stl::string::size_type locEnd = 0;
  do 
    {
    locEnd = selectedLeaves.find( " ", locStart );
    if (locEnd != vtksys_stl::string::npos)
      {
      vtksys_stl::string selectedLeaf = selectedLeaves.substr(locStart, locEnd-locStart);
      this->SelectedLeaves.push_back(selectedLeaf);
      locStart = locEnd+1;
      }
    }
  while (locEnd != vtksys_stl::string::npos) ;
  vtksys_stl::string selectedLeaf = selectedLeaves.substr(locStart);
  this->SelectedLeaves.push_back(selectedLeaf);
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLTreeWidget::IsLeafSelected(const char *leaf)
{
  vtksys_stl::string sleaf(leaf);

  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    if (this->SelectedLeaves[i].compare(sleaf) == 0)
      {
      return 1;
      }
    }
  return 0;
}

void vtkSlicerMRMLTreeWidget::ClearCutNodes()
{
  for (unsigned int i=0; i<this->CutNodes.size(); i++)
    {
    this->CutNodes[i]->UnRegister(this);
    }
  this->CutNodes.clear();
}
