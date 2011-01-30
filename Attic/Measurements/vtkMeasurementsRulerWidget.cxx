#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkMeasurementsRulerWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWCheckButton.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"
#include "vtkBoundingBox.h"

#include "vtkMeasurementsDistanceWidgetClass.h"

#include "vtkMRMLMeasurementsRulerNode.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"


#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

class vtkMeasurementsRulerWidgetCallback : public vtkCommand
{
public:
  static vtkMeasurementsRulerWidgetCallback *New()
  { return new vtkMeasurementsRulerWidgetCallback; }
  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    // save node for undo if it's the start of an interaction event
    if (event == vtkCommand::StartInteractionEvent)
      {
      if (this->RulerNode && this->RulerNode->GetScene())
        {
        this->RulerNode->GetScene()->SaveStateForUndo(this->RulerNode);
        }
      }
    else if (event == vtkCommand::InteractionEvent)
      {
      vtkLineWidget2 *distanceWidget = reinterpret_cast<vtkLineWidget2*>(caller);
      if (distanceWidget)
        {
        if (distanceWidget->GetRepresentation())
          {
          vtkLineRepresentation *rep = vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation());
          if (rep)
            {
            double p1[3], p2[3];
            rep->GetPoint1WorldPosition(p1);
            rep->GetPoint2WorldPosition(p2);
            if (this->RulerNode)
              {
              // does the ruler node have a transform?
              vtkMRMLTransformNode* tnode = this->RulerNode->GetParentTransformNode();
              vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
              transformToWorld->Identity();
              if (tnode != NULL && tnode->IsLinear())
                {
                vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
                lnode->GetMatrixTransformToWorld(transformToWorld);
                }
              // convert by the inverted parent transform
              double  xyzw[4];
              xyzw[0] = p1[0];
              xyzw[1] = p1[1];
              xyzw[2] = p1[2];
              xyzw[3] = 1.0;
              double worldxyz[4], *worldp = &worldxyz[0];
              transformToWorld->Invert();
              transformToWorld->MultiplyPoint(xyzw, worldp);
              this->RulerNode->SetPosition1(worldxyz[0], worldxyz[1], worldxyz[2]);
              // second point
              xyzw[0] = p2[0];
              xyzw[1] = p2[1];
              xyzw[2] = p2[2];
              xyzw[3] = 1.0;
              transformToWorld->MultiplyPoint(xyzw, worldp);
              this->RulerNode->SetPosition2(worldxyz[0], worldxyz[1], worldxyz[2]);
              transformToWorld->Delete();
              transformToWorld = NULL;
              tnode = NULL;
              }
            }
          }
        }
      }
  }
  //,DistanceRepresentation(0)
  vtkMeasurementsRulerWidgetCallback():RulerNode(0) {}
  vtkMRMLMeasurementsRulerNode *RulerNode;
//  std::string RulerID;
//  vtkLineRepresentation *DistanceRepresentation;
};

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMeasurementsRulerWidget );
vtkCxxRevisionMacro ( vtkMeasurementsRulerWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkMeasurementsRulerWidget::vtkMeasurementsRulerWidget ( )
{

  // gui elements
  this->RulerNodeID = NULL;
  this->RulerSelectorWidget = NULL;
  this->VisibilityButton = NULL;
  this->RulerFromFiducialsButton = NULL;
  this->RulerModel1SelectorWidget = NULL;
  this->RulerModel2SelectorWidget = NULL;
  this->PointColourButton = NULL;
  this->Point2ColourButton = NULL;
  this->LineColourButton = NULL;
  this->TextColourButton = NULL;

  this->DistanceLabel = NULL;

  this->Position1Label = NULL;
  this->Position1XEntry = NULL;
  this->Position1YEntry = NULL;
  this->Position1ZEntry = NULL;

  this->Position2Label = NULL;
  this->Position2XEntry = NULL;
  this->Position2YEntry = NULL;
  this->Position2ZEntry = NULL;

  this->DistanceAnnotationFormatEntry = NULL;
  this->DistanceAnnotationScaleEntry = NULL;
  this->DistanceAnnotationVisibilityButton = NULL;

  this->ResolutionEntry = NULL;

  this->AllVisibilityMenuButton = NULL;
  this->RemoveAllRulersButton = NULL;
  this->ReportButton = NULL;

  this->AnnotationFormatMenuButton = NULL;

  // 3d elements
  this->ViewerWidget = NULL;

//  this->DistanceWidget = vtkMeasurementsDistanceWidgetClass::New();
  
  
 
  
  this->SetRulerNodeID(NULL);
  
  this->Updating3DWidget = 0;

//  this->DebugOn();
}


//---------------------------------------------------------------------------
vtkMeasurementsRulerWidget::~vtkMeasurementsRulerWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  // gui elements
  if ( this->AllVisibilityMenuButton )
    {
    this->AllVisibilityMenuButton->SetParent ( NULL );
    this->AllVisibilityMenuButton->Delete();
    this->AllVisibilityMenuButton = NULL;
    }
  if (this->RemoveAllRulersButton )
    {
    this->RemoveAllRulersButton->SetParent (NULL );
    this->RemoveAllRulersButton->Delete ( );
    this->RemoveAllRulersButton = NULL;
    }
  if (this->ReportButton )
    {
    this->ReportButton->SetParent (NULL );
    this->ReportButton->Delete ( );
    this->ReportButton = NULL;
    }
  if ( this->AnnotationFormatMenuButton )
    {
    this->AnnotationFormatMenuButton->SetParent ( NULL );
    this->AnnotationFormatMenuButton->Delete();
    this->AnnotationFormatMenuButton = NULL;
    }
  if (this->RulerSelectorWidget)
    {
    this->RulerSelectorWidget->SetParent(NULL);
    this->RulerSelectorWidget->SetMRMLScene(NULL);
    this->RulerSelectorWidget->Delete();
    this->RulerSelectorWidget = NULL;
    }
  if (this->RulerFromFiducialsButton)
    {
    this->RulerFromFiducialsButton->SetParent(NULL);
    this->RulerFromFiducialsButton->Delete();
    this->RulerFromFiducialsButton = NULL;
    }
 
  if (this->VisibilityButton)
    {
    this->VisibilityButton->SetParent(NULL);
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->SetParent(NULL);
    this->RulerModel1SelectorWidget->SetMRMLScene(NULL);
    this->RulerModel1SelectorWidget->Delete();
    this->RulerModel1SelectorWidget = NULL;
    }
  if (this->RulerModel2SelectorWidget)
    {
    this->RulerModel2SelectorWidget->SetParent(NULL);
    this->RulerModel2SelectorWidget->SetMRMLScene(NULL);
    this->RulerModel2SelectorWidget->Delete();
    this->RulerModel2SelectorWidget = NULL;
    }
  if (this->PointColourButton)
    {
    this->PointColourButton->SetParent(NULL);
    this->PointColourButton->Delete();
    this->PointColourButton= NULL;
    }
  if (this->Point2ColourButton)
    {
    this->Point2ColourButton->SetParent(NULL);
    this->Point2ColourButton->Delete();
    this->Point2ColourButton= NULL;
    }
  if (this->LineColourButton)
    {
    this->LineColourButton->SetParent(NULL);
    this->LineColourButton->Delete();
    this->LineColourButton= NULL;
    }
  if (this->TextColourButton)
    {
    this->TextColourButton->SetParent(NULL);
    this->TextColourButton->Delete();
    this->TextColourButton= NULL;
    }
  if (this->DistanceLabel)
    {
    this->DistanceLabel->SetParent(NULL);
    this->DistanceLabel->Delete();
    this->DistanceLabel = NULL;
    }
  if (this->Position1Label)
    {
    this->Position1Label->SetParent(NULL);
    this->Position1Label->Delete();
    this->Position1Label = NULL;
    }
  if (this->Position1XEntry)
    {
    this->Position1XEntry->SetParent(NULL);
    this->Position1XEntry->Delete();
    this->Position1XEntry = NULL;
    }
  if (this->Position1YEntry)
    {
    this->Position1YEntry->SetParent(NULL);
    this->Position1YEntry->Delete();
    this->Position1YEntry = NULL;
    }
  if (this->Position1ZEntry)
    {
    this->Position1ZEntry->SetParent(NULL);
    this->Position1ZEntry->Delete();
    this->Position1ZEntry = NULL;
    }

  if (this->Position2Label)
    {
    this->Position2Label->SetParent(NULL);
    this->Position2Label->Delete();
    this->Position2Label = NULL;
    }

  if (this->Position2XEntry)
    {
    this->Position2XEntry->SetParent(NULL);
    this->Position2XEntry->Delete();
    this->Position2XEntry = NULL;
    }
  if (this->Position2YEntry)
    {
    this->Position2YEntry->SetParent(NULL);
    this->Position2YEntry->Delete();
    this->Position2YEntry = NULL;
    }
  if (this->Position2ZEntry)
    {
    this->Position2ZEntry->SetParent(NULL);
    this->Position2ZEntry->Delete();
    this->Position2ZEntry = NULL;
    }

  if (this->DistanceAnnotationFormatEntry)
    {
    this->DistanceAnnotationFormatEntry->SetParent(NULL);
    this->DistanceAnnotationFormatEntry->Delete();
    this->DistanceAnnotationFormatEntry = NULL;
    }
  if (this->DistanceAnnotationScaleEntry)
    {
    this->DistanceAnnotationScaleEntry->SetParent(NULL);
    this->DistanceAnnotationScaleEntry->Delete();
    this->DistanceAnnotationScaleEntry = NULL;
    }
  if (this->DistanceAnnotationVisibilityButton)
    {
    this->DistanceAnnotationVisibilityButton->SetParent(NULL);
    this->DistanceAnnotationVisibilityButton->Delete();
    this->DistanceAnnotationVisibilityButton = NULL;
    }

  if (this->ResolutionEntry)
    {
    this->ResolutionEntry->SetParent(NULL);
    this->ResolutionEntry->Delete();
    this->ResolutionEntry = NULL;
    }

  // 3d widgets
  std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
  for (iter = this->DistanceWidgets.begin();
       iter != this->DistanceWidgets.end();
       iter++)
    {
    iter->second->Delete();
    }
  this->DistanceWidgets.clear(); 

  this->SetRulerNodeID(NULL);
  this->SetViewerWidget(NULL);
  this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkMeasurementsRulerWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
    std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
    for (iter = this->DistanceWidgets.begin(); iter !=  this->DistanceWidgets.end(); iter++)
      {
      os << indent << "Distance Widget: " << iter->first.c_str() << "\n";
      iter->second->GetWidget()->PrintSelf(os, indent);
      }
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::SetRulerNodeID ( char *id )
{
  if (this->GetRulerNodeID() != NULL &&
      id != NULL &&
      strcmp(id,this->GetRulerNodeID()) == 0)
    {
    vtkDebugMacro("SetRulerNodeID: no change in id, not doing anything for now: " << id << endl);
    return;
    }
  
  // get the old node - needed to remove events from it
//  vtkMRMLMeasurementsRulerNode *oldRuler = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetRulerNodeID()));
 
  // set the id properly - see the vtkSetStringMacro
  this->RulerNodeID = id;
  
  if (id == NULL)
    {
    vtkDebugMacro("SetRulerNodeID: NULL input id, clearing GUI and returning.\n");
    this->UpdateWidget(NULL);
    return;
    }
  
  // get the new node
  vtkMRMLMeasurementsRulerNode *newRuler = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetRulerNodeID()));
  // set up observers on the new node - now done in adddistancewidget
  if (newRuler != NULL)
    {
//    vtkIntArray *events = vtkIntArray::New();
//    events->InsertNextValue(vtkCommand::ModifiedEvent);
//    events->InsertNextValue(vtkMRMLMeasurementsRulerNode::DisplayModifiedEvent);
//    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
//    events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
//    vtkSetAndObserveMRMLNodeEventsMacro(oldRuler, newRuler, events);
//    events->Delete();

    // set up the GUI
    this->UpdateWidget(newRuler);
    }
  else
    {
    vtkDebugMacro ("ERROR: unable to get the mrml ruler node to observe!\n");
    }
}


//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtStartInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkMeasurementsRulerWidget::UpdateInteractionModeAtStartInteraction()
{

  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }
  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
    return;
    }

  //--- check to see if we're already in "place" mode, and
  //--- if so, allow 1 click to create an angle widget with its
  //--- angle vertex at the click-location.
  //--- If we are not in place mode, then use the mouse-click
  //--- to pick the widget.
  //--- TODO: In the future, may want to modify for THREE 
  //--- clicks to place individual vertices of an angle widget.
  int currentMode = interactionNode->GetCurrentInteractionMode();
  int pickPersistence = interactionNode->GetPickModePersistence();
  if ( currentMode != vtkMRMLInteractionNode::Place && pickPersistence == 0 )
    {
    interactionNode->SetLastInteractionMode ( currentMode );
    }
  interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::PickManipulate);

}



//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtEndInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkMeasurementsRulerWidget::UpdateInteractionModeAtEndInteraction()
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }

  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
    return;
    }

  int pickPersistence = interactionNode->GetPickModePersistence();
  int placePersistence = interactionNode->GetPlaceModePersistence ();
  if ( pickPersistence == 0 && placePersistence == 0 )
    {
    interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ProcessWidgetEvents(vtkObject *caller,
                                                     unsigned long event,
                                                     void *vtkNotUsed(callData))
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "ProcessWidgetEvents: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "ProcessWidgetEvents: got Null SlicerApplicationGUI" );
    return;
    }

 //---
    if (event == vtkCommand::StartInteractionEvent)
    {
    this->UpdateInteractionModeAtStartInteraction();
    }
  else if (event == vtkCommand::EndInteractionEvent)
    {
    this->UpdateInteractionModeAtEndInteraction();
    }

  // process events that apply to all lists
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
  if (menu != NULL)
    {
    if ( menu == this->AllVisibilityMenuButton->GetMenu() )
      {
      // set visibility on all rulers
      if ( menu->GetItemSelectedState ( "All Rulers Visible" ) == 1 )
        {
        this->ModifyAllRulerVisibility (1 );
        }
      else if ( menu->GetItemSelectedState ( "All Rulers Invisible" ) == 1 )
        {
        this->ModifyAllRulerVisibility (0 );
        }
      // call the update here as modifying the mrml nodes will bounce on the
      // check in process mrml events for process this widget event
      this->Update3DWidgetsFromMRML();
      }
    }

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->RemoveAllRulersButton)
    {
    int numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLMeasurementsRulerNode" );
    if ( numnodes > 0 )
      {
      std::string message;
      if ( numnodes > 1 )
        {
        message = "Are you sure you want to delete all Rulers?";
        }
      else
        {
        message = "Are you sure you want to delete the Ruler?";        
        }
      //--- ask user to confirm.
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent (  this->RemoveAllRulersButton );
      dialog->SetStyleToOkCancel();
      dialog->SetText(message.c_str());
      dialog->Create ( );
      dialog->SetMasterWindow( this->RemoveAllRulersButton );
      dialog->ModalOn();
      int doit = dialog->Invoke();
      dialog->Delete();
      
      if ( doit )
        {
        vtkDebugMacro("ProcessWidgetEvents: Remove Rulers Button event: " << event << ".\n");
        // save state for undo
        this->MRMLScene->SaveStateForUndo();

        //--- now delete all nodes... 
        for (int nn=0; nn<numnodes; nn++ )
          {
          vtkMRMLMeasurementsRulerNode *aNode = vtkMRMLMeasurementsRulerNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( 0, "vtkMRMLMeasurementsRulerNode" ));
          if ( aNode )
            {
            this->GetMRMLScene()->RemoveNode(aNode);
            
            this->SetRulerNodeID(NULL);
            }
          }
        }
      }
    return;
    }
  vtkKWLoadSaveDialog *d = vtkKWLoadSaveDialog::SafeDownCast ( caller );
  if (d != NULL &&  event == vtkKWTopLevel::WithdrawEvent &&
      d == this->ReportButton->GetLoadSaveDialog())
    {
    const char *fileName = this->ReportButton->GetFileName();
    if (fileName)
      {
      this->GenerateReport(fileName);
      this->ReportButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      if ( this->ReportButton->GetText())
        {
        this->ReportButton->SetText ("Generate a Report");
        }
      }
    }
  
  vtkKWPushButton *fromFids = vtkKWPushButton::SafeDownCast ( caller );
  if (fromFids && event == vtkKWPushButton::InvokedEvent)
    {
    if (fromFids == this->RulerFromFiducialsButton)
      {
      if (appGUI)
        {
        appGUI->ProcessAddRulerCommand();
        }
      else
        {
        vtkWarningMacro("Can't get at the application gui to make a ruler from fiducials");
        }
      }
    }
  
  // process ruler node selector events
  if (this->RulerSelectorWidget ==  vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
      {
      vtkDebugMacro("ProcessWidgetEvents Node Selector Event: " << event << ".\n");
      vtkMRMLMeasurementsRulerNode *rulerNode =
        vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected());
      if (rulerNode != NULL)
        {
        this->SetRulerNodeID(rulerNode->GetID());
        }
      else
        {
        vtkDebugMacro("ProcessWidgetEvent: the selected node is null!");
        }
      return;
    }

  // get the currently displayed list
  
  // is there one list?
  vtkMRMLMeasurementsRulerNode *activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
  if (activeRulerNode == NULL)
    {
    vtkDebugMacro("No selected ruler");
    return;
    /*
      not allowing spontaneous creation of a ruler
    vtkDebugMacro ("ERROR: No ruler node, adding one first!\n");
      vtkMRMLMeasurementsRulerNode *newList = this->GetLogic()->AddRuler();      
      if (newList != NULL)
        {
        this->SetRulerNodeID(newList->GetID());
        newList->Delete();
        }
      else
        {
        vtkErrorMacro("Unable to add a new ruler via the logic\n");
        }
      // now get the newly active node 
      activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
      if (activeRulerNode == NULL)
        {
        vtkErrorMacro ("ERROR adding a new ruler list for the point...\n");
        return;
        }
    */
    }

  // GUI elements
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWChangeColorButton *ccbutton = vtkKWChangeColorButton::SafeDownCast(caller);
  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);
 
  if (b && event == vtkKWPushButton::InvokedEvent)
    {
    if (b == this->VisibilityButton)
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetVisibility(!(activeRulerNode->GetVisibility()));
      this->Update3DWidget(activeRulerNode);
      }
    else if (b == this->DistanceAnnotationVisibilityButton)
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetDistanceAnnotationVisibility(!(activeRulerNode->GetDistanceAnnotationVisibility()));
      this->Update3DWidget(activeRulerNode);
      }
    }
  else if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->RulerModel1SelectorWidget &&
           event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent
           && this->GetViewerWidget())
    {
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->RulerModel1SelectorWidget->GetSelected());
    if (model != NULL  && model->GetDisplayNode() != NULL)
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetModelID1(model->GetID());
      this->Update3DWidget(activeRulerNode);
      }
    else
      {
      // is it a slice node?
      vtkMRMLSliceNode *slice = vtkMRMLSliceNode::SafeDownCast(this->RulerModel1SelectorWidget->GetSelected());
      if (slice != NULL && slice->GetID())
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetModelID1(slice->GetID());
        this->Update3DWidget(activeRulerNode);
        }
      else
        {
        // remove the constraint by setting it to null
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetModelID1(NULL);
        this->Update3DWidget(activeRulerNode);
        }
      }
    }
  else if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->RulerModel2SelectorWidget &&
           event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent
           && this->GetViewerWidget())
    {
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->RulerModel2SelectorWidget->GetSelected());
    if (model != NULL  && model->GetDisplayNode() != NULL)
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetModelID2(model->GetID());
      this->Update3DWidget(activeRulerNode);
      }
    else
      {
      // is it a slice node?
      vtkMRMLSliceNode *slice = vtkMRMLSliceNode::SafeDownCast(this->RulerModel2SelectorWidget->GetSelected());
      if (slice != NULL && slice->GetID())
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetModelID2(slice->GetID());
        this->Update3DWidget(activeRulerNode);
        }
      else
        {
        // remove the constraint by setting it to null
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetModelID2(NULL);
        this->Update3DWidget(activeRulerNode);
        }
      }
    }
  else if (activeRulerNode == vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected()) &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("\tmodified event on the ruler selected node.\n");
    if (activeRulerNode !=  vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected()))
      {
      // select it first off
      this->SetRulerNodeID(vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected())->GetID());
      }
    vtkDebugMacro("Setting gui from ruler node");
    this->UpdateWidget(activeRulerNode);
    return;
    }
  else if (ccbutton && event == vtkKWChangeColorButton::ColorChangedEvent)
    {
    if (ccbutton == this->PointColourButton)
      {
      double *guiRGB = this->PointColourButton->GetColor();
      double *nodeRGB = activeRulerNode->GetPointColour();
      if (nodeRGB == NULL ||
          (fabs(guiRGB[0]-nodeRGB[0]) > 0.001 ||
           fabs(guiRGB[1]-nodeRGB[1]) > 0.001 ||
           fabs(guiRGB[2]-nodeRGB[2]) > 0.001))
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetPointColour(this->PointColourButton->GetColor());
        }
      }
    else if (ccbutton == this->Point2ColourButton)
      {
      double *guiRGB = this->Point2ColourButton->GetColor();
      double *nodeRGB = activeRulerNode->GetPoint2Colour();
      if (nodeRGB == NULL ||
          (fabs(guiRGB[0]-nodeRGB[0]) > 0.001 ||
           fabs(guiRGB[1]-nodeRGB[1]) > 0.001 ||
           fabs(guiRGB[2]-nodeRGB[2]) > 0.001))
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetPoint2Colour(this->Point2ColourButton->GetColor());
        }
      }
    else if (ccbutton == this->LineColourButton)
      {
      double *guiRGB = this->LineColourButton->GetColor();
      double *nodeRGB = activeRulerNode->GetLineColour();
      if (nodeRGB == NULL ||
          (fabs(guiRGB[0]-nodeRGB[0]) > 0.001 ||
           fabs(guiRGB[1]-nodeRGB[1]) > 0.001 ||
           fabs(guiRGB[2]-nodeRGB[2]) > 0.001))
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetLineColour(this->LineColourButton->GetColor());
        }
      }
    else if (ccbutton == this->TextColourButton)
      {
      double *guiRGB = this->TextColourButton->GetColor();
      double *nodeRGB = activeRulerNode->GetDistanceAnnotationTextColour();
      if (nodeRGB == NULL ||
          (fabs(guiRGB[0]-nodeRGB[0]) > 0.001 ||
           fabs(guiRGB[1]-nodeRGB[1]) > 0.001 ||
           fabs(guiRGB[2]-nodeRGB[2]) > 0.001))
        {
        if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
        activeRulerNode->SetDistanceAnnotationTextColour(this->TextColourButton->GetColor());
        }
      }
    this->Update3DWidget(activeRulerNode);
    }
  else if ( menu != NULL &&
            menu == this->AnnotationFormatMenuButton->GetWidget()->GetMenu() )
    {
    // set visibility on all rulers
    if ( menu->GetItemSelectedState ( "1 decimal" ) == 1 )
      {
      activeRulerNode->SetDistanceAnnotationFormat("%.1f mm");
      }
    else if ( menu->GetItemSelectedState ( "0 decimals" ) == 1 )
      {
      activeRulerNode->SetDistanceAnnotationFormat("%.0f mm");
      }
    else if ( menu->GetItemSelectedState ( "2 decimals" ) == 1 )
      {
      activeRulerNode->SetDistanceAnnotationFormat("%.2f mm");
      }
    else if ( menu->GetItemSelectedState ( "Scientific Notation" ) == 1 )
      {
      activeRulerNode->SetDistanceAnnotationFormat("%.2e mm");
      }
    }
  else if (entry && event == vtkKWEntry::EntryValueChangedEvent)
    {
    if (entry == this->Position1XEntry ||
        entry == this->Position1YEntry ||
        entry == this->Position1ZEntry)
      {
      double x, y, z;
      double *position = activeRulerNode->GetPosition1();
      // make sure don't undo the changes in the node by getting the gui
      // elements since that would trigger an update to the gui with the old
      // values
      if (entry == this->Position1XEntry)
        {
        x = this->Position1XEntry->GetValueAsDouble();
        }
      else
        {
        // grab the value from the node
        x = position[0];
        }
      if (entry ==  this->Position1YEntry)
        {
        y = this->Position1YEntry->GetValueAsDouble();
        }
      else
        {
        y = position[1];
        }
      if (entry == this->Position1ZEntry)
        {
        z = this->Position1ZEntry->GetValueAsDouble();
        }
      else
        {
        z = position[2];
        }
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetPosition1(x, y, z);
      }
    else if (entry == this->Position2XEntry ||
             entry == this->Position2YEntry ||
             entry == this->Position2ZEntry)
      {
      double x, y, z;
      double *position = activeRulerNode->GetPosition2();
      if (entry == this->Position2XEntry)
        {
        x = this->Position2XEntry->GetValueAsDouble();
        }
      else
        {
        x = position[0];
        }
      if (entry == this->Position2YEntry)
        {
        y = this->Position2YEntry->GetValueAsDouble();
        }
      else
        {
        y = position[1];
        }
      if (entry == this->Position2ZEntry)
        {
        z = this->Position2ZEntry->GetValueAsDouble();
        }
      else
        {
        z = position[2];
        }
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetPosition2(x, y, z);
      }
    else if (entry == this->DistanceAnnotationFormatEntry->GetWidget())
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetDistanceAnnotationFormat(this->DistanceAnnotationFormatEntry->GetWidget()->GetValue());
      }
    else if (entry == this->DistanceAnnotationScaleEntry->GetWidget())
      {
      double scale = this->DistanceAnnotationScaleEntry->GetWidget()->GetValueAsDouble();
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetDistanceAnnotationScale(scale, scale, scale);
      }
    else if (entry == this->ResolutionEntry->GetWidget())
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetResolution(this->ResolutionEntry->GetWidget()->GetValueAsInt());
      }
    this->Update3DWidget(activeRulerNode);
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateMRMLFromWidget(vtkMRMLMeasurementsRulerNode *rulerNode)
{
  // is there a ruler node?
  vtkMRMLMeasurementsRulerNode *activeRulerNode;
  if (rulerNode == NULL)
    {
    activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
    }
  else
    {
    activeRulerNode = rulerNode;
    }
  if (activeRulerNode == NULL)
    {
    vtkErrorMacro("No selected ruler");
    return;
    }

  vtkMeasurementsDistanceWidgetClass *distanceWidgetClass = this->GetDistanceWidget(activeRulerNode->GetID());
  if (!distanceWidgetClass)
    {
    vtkErrorMacro("No distance widget class found for rulernode " << activeRulerNode->GetID());
    return;
    }
  vtkLineWidget2 *distanceWidget = distanceWidgetClass->GetWidget();
  if (distanceWidget == NULL)
    {
    vtkErrorMacro("No distance widget found for ruler node " << activeRulerNode->GetID());
    return;
    }
  activeRulerNode->SetVisibility(distanceWidget->GetEnabled());

  vtkLineRepresentation *rep = vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation());
  if (rep)
    {
    double *p;
    p = rep->GetPoint1WorldPosition();
    activeRulerNode->SetPosition1(p);
    p = rep->GetPoint2WorldPosition();
    activeRulerNode->SetPosition2(p);

    double *rgb = rep->GetEndPointProperty()->GetColor();
    activeRulerNode->SetPointColour(rgb);
    rgb = rep->GetEndPoint2Property()->GetColor();
    activeRulerNode->SetPoint2Colour(rgb);
    rgb = rep->GetLineProperty()->GetColor();
    activeRulerNode->SetLineColour(rgb);
    rgb = rep->GetDistanceAnnotationProperty()->GetColor();
    activeRulerNode->SetDistanceAnnotationTextColour(rgb);

    activeRulerNode->SetDistanceAnnotationVisibility(rep->GetDistanceAnnotationVisibility());
    activeRulerNode->SetDistanceAnnotationFormat(rep->GetDistanceAnnotationFormat());
    activeRulerNode->SetDistanceAnnotationScale(rep->GetDistanceAnnotationScale());

    activeRulerNode->SetResolution(rep->GetResolution());
    }

  // skip the models for now
  
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  
 vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

 // the scene was closed, don't get node removed events so clear up here
  if (callScene != NULL &&
      event == vtkMRMLScene::SceneClosedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
    // the lists are already gone from the scene, so need to clear out all the
    // widget properties, can't call remove with a node
    this->RemoveDistanceWidgets();
    // set colour choosers to white, reset distance label
    this->ResetGUI();
    return;
    }

  // first check to see if there was a ruler list node deleted
  if (callScene != NULL &&
      callScene == this->MRMLScene &&
      event == vtkMRMLScene::NodeRemovedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
    // check to see if it was the current node that was deleted
    if (callData != NULL)
      {
      vtkMRMLMeasurementsRulerNode *delNode = reinterpret_cast<vtkMRMLMeasurementsRulerNode*>(callData);
      if (delNode != NULL &&
          delNode->IsA("vtkMRMLMeasurementsRulerNode"))
        {
        vtkDebugMacro("A ruler node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
        this->RemoveDistanceWidget(delNode);
        }
      }
    }

  vtkMRMLMeasurementsRulerNode *node = vtkMRMLMeasurementsRulerNode::SafeDownCast(caller);
  vtkMRMLMeasurementsRulerNode *activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());

  // check for a node added event
  if (callScene != NULL &&
      callScene == this->MRMLScene &&
      callData != NULL &&
      event == vtkMRMLScene::NodeAddedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");
    // check to see if it was a ruler node    
    vtkMRMLMeasurementsRulerNode *addNode = reinterpret_cast<vtkMRMLMeasurementsRulerNode*>(callData);
    if (addNode != NULL &&
        addNode->IsA("vtkMRMLMeasurementsRulerNode"))
      {
      vtkDebugMacro("Got a node added event with a ruler node " << addNode->GetID());
      // is it currently the active one?
      if (addNode == activeRulerNode)
        {
        // update the GUI 
        vtkDebugMacro("Calling Update widget to set up the ui");
        this->UpdateWidget(addNode);
        }
      else
        {
        // make it active
        this->RulerSelectorWidget->SetSelected(addNode);
        vtkDebugMacro("Set added node to be selected, now setting the ruler node id");
        // this calls UpdateWidget to update the gui
        this->SetRulerNodeID(addNode->GetID());
        }
      // update the 3d widget
      vtkDebugMacro("Calling Update 3D widget to set up a new distance widget");
      this->Update3DWidget(addNode);
      // for now, since missing some of the add calls when open a scene, make sure we're current with the scene
      this->Update3DWidgetsFromMRML();
      return;
      }
    }

  else if (node == activeRulerNode)
    {
    if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent)
      {
      vtkDebugMacro("Modified or node added or removed event on the ruler node.\n");
      if (node == NULL)
        {
        vtkDebugMacro("\tBUT: the node is null\n");
        // check to see if the id used to get the node is not null, if it's
        // a valid string, means that the node was deleted
        if (this->GetRulerNodeID() != NULL)
          {
          this->SetRulerNodeID(NULL);
          this->ResetGUI();
          }
        return;
        }
      vtkDebugMacro("ProcessMRMLEvents: \t\tUpdating the GUI\n");
      // update the gui
      UpdateWidget(activeRulerNode);
      return;
      }
    } // end of events on the active ruler node
  else if (node != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    // it's a modified event on a ruler node that's not being displayed in the
    // 2d gui, so update the 3d widget
    this->Update3DWidget(node);
    }  
  else if (node != NULL &&
      event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkDebugMacro("Got transform modified event on node " << node->GetID());
    this->Update3DWidget(node);
    }
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateWidget(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{

  vtkDebugMacro("UpdateWidget: active ruler node is " << (activeRulerNode == NULL ? "null" : activeRulerNode->GetName()));

  // to get at some top level icons
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkDebugMacro ( "UpdateWidget: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkDebugMacro ( "UpdateWidget: got Null SlicerApplicationGUI" );
    return;
    }
  
  // if the passed node is null, clear out the widget
  if (activeRulerNode == NULL)
    {
    // don't need to do anything yet, especially don't set the node selector to
    // null, as it causes a crash
    this->ResetGUI();
    vtkDebugMacro("UpdateWidget: The passed in node is null, returning.");
    return;
    }
  
  if ( this->RulerSelectorWidget->GetSelected() == NULL )
    {
    vtkDebugMacro("Null selected ruler, selecting it and returning");
    this->RulerSelectorWidget->SetSelected(activeRulerNode);
    return;
    }

  if (activeRulerNode && this->RulerSelectorWidget->GetSelected() &&
        strcmp(activeRulerNode->GetName(),
               this->RulerSelectorWidget->GetSelected()->GetName()) != 0)
      {
      vtkDebugMacro("UpdateWidget: input ruler " << activeRulerNode->GetName() << " doesn't match selector widget value: " << this->RulerSelectorWidget->GetSelected()->GetName());
      this->RulerSelectorWidget->SetSelected(activeRulerNode);
      vtkDebugMacro("... returning, hoping for a invoke event");
      return;
      }

  vtkDebugMacro("UpdateWidget: updating the gui and 3d elements");
  // first update the GUI, then update the 3d elements
  // visibility
  if (activeRulerNode->GetVisibility())
    {
    this->GetVisibilityButton()->SetImageToIcon(appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleIcon());
    this->GetVisibilityButton()->SetBalloonHelpString ( "Hide the selected ruler." );
    }
  else
    {
    this->GetVisibilityButton()->SetImageToIcon(appGUI->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon());
    this->GetVisibilityButton()->SetBalloonHelpString ( "Show the selected ruler." );
    }
  // end point positions
  double *position = activeRulerNode->GetPosition1();
  if (position)
    {
    this->Position1XEntry->SetValueAsDouble(position[0]);
    this->Position1YEntry->SetValueAsDouble(position[1]);
    this->Position1ZEntry->SetValueAsDouble(position[2]);
    }
  position = activeRulerNode->GetPosition2();
  if (position)
    {
    this->Position2XEntry->SetValueAsDouble(position[0]);
    this->Position2YEntry->SetValueAsDouble(position[1]);
    this->Position2ZEntry->SetValueAsDouble(position[2]);
    }

  // constraints
  const char *modelID1 = activeRulerNode->GetModelID1();
  if (modelID1)
    {
    // get the node
    vtkMRMLNode *model = this->GetMRMLScene()->GetNodeByID(modelID1);
    if (model)
      {
      this->RulerModel1SelectorWidget->SetSelected(model);
      }
    }
  const char *modelID2 = activeRulerNode->GetModelID2();
  if (modelID2)
    {
    // get the second  node 
    vtkMRMLNode *model = this->GetMRMLScene()->GetNodeByID(modelID2);
    if (model)
      {
      this->RulerModel2SelectorWidget->SetSelected(model);
      }
    }
  
  // end point colour
  double *rgb = this->PointColourButton->GetColor();
  double *rgb1 = activeRulerNode->GetPointColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->PointColourButton->SetColor(activeRulerNode->GetPointColour());
    }
  rgb = this->Point2ColourButton->GetColor();
  rgb1 = activeRulerNode->GetPoint2Colour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->Point2ColourButton->SetColor(activeRulerNode->GetPoint2Colour());
    }
  
  // line colour
  rgb = this->LineColourButton->GetColor();
  rgb1 = activeRulerNode->GetLineColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->LineColourButton->SetColor(activeRulerNode->GetLineColour());
    }

 

  // distance annotation
  rgb = this->TextColourButton->GetColor();
  rgb1 = activeRulerNode->GetDistanceAnnotationTextColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->TextColourButton->SetColor(activeRulerNode->GetDistanceAnnotationTextColour());
    }
  if (activeRulerNode->GetDistanceAnnotationVisibility())
    {
    this->DistanceAnnotationVisibilityButton->SetImageToIcon(appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleIcon());
    this->DistanceAnnotationVisibilityButton->SetBalloonHelpString ( "Don't show the selected ruler's annotation text." );
    }
  else
    {
    this->DistanceAnnotationVisibilityButton->SetImageToIcon(appGUI->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon());
    this->DistanceAnnotationVisibilityButton->SetBalloonHelpString ( "Show the selected ruler's annotation text." );
    }
  this->DistanceAnnotationFormatEntry->GetWidget()->SetValue(activeRulerNode->GetDistanceAnnotationFormat());
  double *scale = activeRulerNode->GetDistanceAnnotationScale();
  if (scale)
    {
    this->DistanceAnnotationScaleEntry->GetWidget()->SetValueAsDouble(scale[0]);
    }

  // resolution
  this->ResolutionEntry->GetWidget()->SetValueAsInt(activeRulerNode->GetResolution());

  // update the label colours to match the end point colours
  this->UpdateLabelsFromNode(activeRulerNode);
  
  // distance
  this->UpdateDistanceLabel(activeRulerNode);
  
  this->Update3DWidget(activeRulerNode);
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateDistanceLabel(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{
  if (activeRulerNode == NULL || this->DistanceLabel == NULL)
    {
    vtkDebugMacro("UpdateDistanceLabel: no active ruler node or distance label");
    return;
    }

  std::string distanceString = std::string("Distance: ");
  //double distanceValue = activeRulerNode->GetDistance();

  activeRulerNode->UpdateCurrentDistanceAnnotation();
  const char *distanceValue = activeRulerNode->GetCurrentDistanceAnnotation();
  
  vtkDebugMacro("UpdateDistanceLabel: distance = " << distanceValue);
  std::stringstream ss;
  ss << distanceString;
  ss << distanceValue;
  distanceString = ss.str();
  this->DistanceLabel->SetText(distanceString.c_str());
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::Update3DWidgetVisibility(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{
  if (activeRulerNode == NULL)
    {
    vtkDebugMacro("Update3DWidget: passed in ruler node is null, returning");
    return;
    }
  vtkMeasurementsDistanceWidgetClass *distanceWidgetClass = this->GetDistanceWidget(activeRulerNode->GetID());
  if (!distanceWidgetClass)
    {
    vtkErrorMacro("Update3DWidgetVisibility: no widget to update!");
    return;
    }
  vtkLineWidget2 *distanceWidget = distanceWidgetClass->GetWidget();
  if (distanceWidget->GetInteractor() == NULL)
    {
    if (this->GetViewerWidget() &&
        this->GetViewerWidget()->GetMainViewer() &&
        this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor())
      {
      distanceWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
      double p1[3] = {-100.0, 50.0, 0.0};
      double p2[3] = {100.0,  50.0, 0.0};
      vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetPoint1WorldPosition(p1);
      vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetPoint2WorldPosition(p2);
      }
    else
      {
      vtkDebugMacro("Update3DWidgetVisibility: no interactor found! Ruler widget won't work until this is set");
      distanceWidget->SetInteractor(NULL);
      }
    }
  if (activeRulerNode->GetVisibility())
    {
    if (distanceWidget->GetInteractor() != NULL)
      {
      vtkDebugMacro("Update3DWidgetVisibility: distance widget on");
      distanceWidget->On();
      distanceWidget->ProcessEventsOn();
      }
    else
      {
      vtkDebugMacro("Update3DWidgetVisibility: no interactor set");
      }
    }
  else
    {
    vtkDebugMacro("Update3DWidgetVisibility: distance widget off");
    distanceWidget->Off();
    distanceWidget->ProcessEventsOff();
    }

}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::Update3DWidget(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{
  if (activeRulerNode == NULL)
    {
    vtkDebugMacro("Update3DWidget: passed in ruler node is null, returning");
    return;
    }
  if (this->Updating3DWidget)
    {
    vtkDebugMacro("Already updating 3d widget");
    return;
    }
  vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(activeRulerNode->GetID());
  if (!distanceWidget)
    {
    vtkDebugMacro("No distance widget found for rulernode " << activeRulerNode->GetID() << ", have " << this->DistanceWidgets.size() << " widgets, adding one for this one");
    this->AddDistanceWidget(activeRulerNode);
    distanceWidget = this->GetDistanceWidget(activeRulerNode->GetID());
    if (!distanceWidget)
      {
      vtkErrorMacro("Error adding a new distance widget for ruler node " << activeRulerNode->GetID());
      this->Updating3DWidget = 0;
      return;
      }
    }
  if (distanceWidget->GetWidget() == NULL)
    {
    vtkDebugMacro("Update3D widget: distance widget is null");
    return;
    }
  vtkLineRepresentation *rep = vtkLineRepresentation::SafeDownCast(distanceWidget->GetWidget()->GetRepresentation());
  if (rep == NULL)
    {
    vtkDebugMacro("Update3D widget: distance representation is null");
    return;
    }
  this->Updating3DWidget = 1;

  vtkDebugMacro("Updating 3d widget from " << activeRulerNode->GetID());
  
  // visibility
  this->Update3DWidgetVisibility(activeRulerNode);
  
  if (rep)
    {
    // end point colour
    double *rgb1 = activeRulerNode->GetPointColour();
    rep->GetEndPointProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);
    double *rgb2 = activeRulerNode->GetPoint2Colour();
    rep->GetEndPoint2Property()->SetColor(rgb2[0], rgb2[1], rgb2[2]);

    // line colour
    rgb1 = activeRulerNode->GetLineColour();
    rep->GetLineProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // text colour
    rgb1 = activeRulerNode->GetDistanceAnnotationTextColour();
    rep->GetDistanceAnnotationProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // position
    // get any transform on the node
    vtkMRMLTransformNode* tnode = activeRulerNode->GetParentTransformNode();
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    if (tnode != NULL && tnode->IsLinear())
      {
      vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
      lnode->GetMatrixTransformToWorld(transformToWorld);
      }
    double *p = activeRulerNode->GetPosition1();
    if (p)
      {
      // convert by the parent transform
      double xyzw[4];
      xyzw[0] = p[0];
      xyzw[1] = p[1];
      xyzw[2] = p[2];
      xyzw[3] = 1.0;
      double worldxyz[4], *worldp = &worldxyz[0];
      transformToWorld->MultiplyPoint(xyzw, worldp);
      rep->SetPoint1WorldPosition(worldp);
      }
    p =  activeRulerNode->GetPosition2();
    if (p)
      {
      // convert by the parent transform
      double xyzw[4];
      xyzw[0] = p[0];
      xyzw[1] = p[1];
      xyzw[2] = p[2];
      xyzw[3] = 1.0;
      double worldxyz[4], *worldp = &worldxyz[0];
      transformToWorld->MultiplyPoint(xyzw, worldp);
      rep->SetPoint2WorldPosition(worldp);
      }
    tnode = NULL;
    transformToWorld->Delete();
    transformToWorld = NULL;

    // distance annotation
    rep->SetDistanceAnnotationVisibility(activeRulerNode->GetDistanceAnnotationVisibility());
    rep->SetDistanceAnnotationFormat(activeRulerNode->GetDistanceAnnotationFormat());
    double *scale = activeRulerNode->GetDistanceAnnotationScale();
    if (scale)
      {
      rep->SetDistanceAnnotationScale(scale);
      }
    // resolution
    rep->SetResolution(activeRulerNode->GetResolution());
    }

  // first point constraint
  if (activeRulerNode->GetModelID1())
    {
    // get the model node
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID1()));
    vtkMRMLSliceNode *slice =
      vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID1()));
    // is it a slice node?
    if (slice)
      {
      // get the model node associated with it
      vtkDebugMacro("Update3DWidget: Have a slice node, id = " << slice->GetID());
      // get the model node associated with it
      std::string modelName = std::string(slice->GetName()) + std::string(" Volume Slice");
      vtkCollection *modelCollection = this->GetMRMLScene()->GetNodesByName(modelName.c_str());
      if (modelCollection &&
          modelCollection->GetNumberOfItems() > 0)
        {
        model = vtkMRMLModelNode::SafeDownCast(modelCollection->GetItemAsObject(0));
        }
      }
    // is it a valid model?
    if (model &&
        model->GetDisplayNode())
      {
      if (model->GetDisplayNode()->GetVisibility() == 0)
        {
        if (slice)
          {
          vtkWarningMacro("The " <<  slice->GetName() << " slice is not visible, you won't be able to move the end point.");
          }
        else
          {
          vtkWarningMacro("The " <<  model->GetName() << " model is not visible, you won't be able to move the end point");
          }
        }
      vtkProp *prop = vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()));
      // is it already set to constrain the point placer?
      if (prop &&
          !distanceWidget->GetModel1PointPlacer()->HasProp(prop))
        {
        // clear out any others
        distanceWidget->GetModel1PointPlacer()->RemoveAllProps();
        // add this one
        distanceWidget->GetModel1PointPlacer()->AddProp(prop);
        rep->GetPoint1Representation()->ConstrainedOff();
        rep->GetPoint1Representation()->SetPointPlacer(distanceWidget->GetModel1PointPlacer());
        }
      }
    else
      {
      distanceWidget->GetModel1PointPlacer()->RemoveAllProps();
      rep->GetPoint1Representation()->SetPointPlacer(NULL);
      }
    }
  else
    {
    // make sure it's not constrained
    distanceWidget->GetModel1PointPlacer()->RemoveAllProps();
    rep->GetPoint1Representation()->SetPointPlacer(NULL);
    }

  // second point constraint
  if (activeRulerNode->GetModelID2())
    {
    // get the model node
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID2()));
     vtkMRMLSliceNode *slice =
      vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID2()));
    // is it a slice node?
    if (slice)
      {
      // get the model node associated with it
      vtkDebugMacro("Update3DWidget: Have a slice node, id = " << slice->GetID());
      // get the model node associated with it
      std::string modelName = std::string(slice->GetName()) + std::string(" Volume Slice");
      vtkCollection *modelCollection = this->GetMRMLScene()->GetNodesByName(modelName.c_str());
      if (modelCollection &&
          modelCollection->GetNumberOfItems() > 0)
        {
        model = vtkMRMLModelNode::SafeDownCast(modelCollection->GetItemAsObject(0));
        }
      }
    // is it a valid model?
    if (model &&
        model->GetDisplayNode())
      {
      vtkProp *prop = vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()));
      // is it already set to constrain the point placer?
      if (prop &&
          !distanceWidget->GetModel2PointPlacer()->HasProp(prop))
        {
        // clear out any others
        distanceWidget->GetModel2PointPlacer()->RemoveAllProps();
        // add this one
        distanceWidget->GetModel2PointPlacer()->AddProp(prop);
        rep->GetPoint2Representation()->ConstrainedOff();
        rep->GetPoint2Representation()->SetPointPlacer(distanceWidget->GetModel2PointPlacer());
        /*
        // check if need to snap to it
        // TODO: figure out why not snapping
        double pos[3];
        rep->GetPoint2WorldPosition(pos);
        if (!rep->GetPoint2Representation()->GetPointPlacer()->ValidateWorldPosition(pos))
          {
          if (model->GetPolyData())
            {
            model->GetPolyData()->GetPoint(0, pos);
            vtkDebugMacro("Snapping point 2 to " << pos[0] << ", " << pos[1] << ", " << pos[2]);
            rep->SetPoint2WorldPosition(pos);
            }
          }
        */
        }
      }
    else
      {
      distanceWidget->GetModel2PointPlacer()->RemoveAllProps();
//      distanceWidget->GetHandleRepresentation->ConstrainedOn();
      rep->GetPoint2Representation()->SetPointPlacer(NULL);
      }
    }
  else
    {
    // make sure it's not constrained
    distanceWidget->GetModel2PointPlacer()->RemoveAllProps();
    rep->GetPoint2Representation()->SetPointPlacer(NULL);
    }

  // set up call back
  // temp: remove observers
  distanceWidget->GetWidget()->RemoveObservers(vtkCommand::InteractionEvent);
  distanceWidget->GetWidget()->RemoveObservers(vtkCommand::StartInteractionEvent);

  // now add call back
  vtkMeasurementsRulerWidgetCallback *myCallback = vtkMeasurementsRulerWidgetCallback::New();
  //  std::string rulerID = std::string(activeRulerNode->GetID());
  //  myCallback->RulerID = rulerID;
  myCallback->RulerNode = activeRulerNode;
  distanceWidget->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
  distanceWidget->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
  myCallback->Delete();
  //---
  //--- add another observer that triggers the processWidgetEvents
  //--- method as well, where we can update mouse mode changes.
  //--- This pattern, and the methods called by processWidgetEvents
  //--- can be used by other SlicerWidgets that are pickable or placeable.
  //---
  distanceWidget->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  distanceWidget->GetWidget()->AddObserver(vtkCommand::EndInteractionEvent, (vtkCommand *)this->GUICallbackCommand);


  // update the distance label from the widget
  this->UpdateDistanceLabel(activeRulerNode);

  // request a render
  if (this->ViewerWidget)
    {
    this->ViewerWidget->RequestRender();
    }
  // reset the flag
  this->Updating3DWidget = 0;
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::AddMRMLObservers ( )
{
  // the widget as a whole needs to keep track of ruler nodes in the scene
  if (this->MRMLScene)
    {
    vtkDebugMacro("AddMRMLObservers: watching for node removed, added, scene close events on the scene");
    if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (this->MRMLScene->HasObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveMRMLObservers ( )
{
  // remove observers on the ruler nodes
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLMeasurementsRulerNode");
  //vtkIntArray *events = vtkIntArray::New();
  //events->InsertNextValue(vtkCommand::ModifiedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  //events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLMeasurementsRulerNode *rulerNode = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLMeasurementsRulerNode"));
    //vtkSetAndObserveMRMLNodeEventsMacro(rulerNode, NULL, events);
    if (rulerNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      rulerNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (rulerNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      rulerNode->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    rulerNode = NULL;
    }
  //events->Delete();

  if (this->MRMLScene)
    {
    vtkDebugMacro("RemoveMRMLObservers: stopping watching for node removed, added, scene close events on the scene");
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::AddWidgetObservers()
{
  if (this->AllVisibilityMenuButton)
    {
    this->AllVisibilityMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RemoveAllRulersButton)
    {
    this->RemoveAllRulersButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ReportButton)
    {
    this->ReportButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->AnnotationFormatMenuButton)
    {
    this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerFromFiducialsButton)
    {
    this->RulerFromFiducialsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->VisibilityButton)
    {
    this->VisibilityButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PointColourButton)
    {
    this->PointColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->Point2ColourButton)
    {
    this->Point2ColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LineColourButton)
    {
    this->LineColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->TextColourButton)
    {
    this->TextColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerSelectorWidget)
    {
    this->RulerSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel2SelectorWidget)
    {
    this->RulerModel2SelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->DistanceAnnotationVisibilityButton)
    {
    this->DistanceAnnotationVisibilityButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->Position1XEntry)
    {
    this->Position1XEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position1YEntry)
    {
    this->Position1YEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position1ZEntry)
    {
    this->Position1ZEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->Position2XEntry)
    {
    this->Position2XEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position2YEntry)
    {
    this->Position2YEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position2ZEntry)
    {
    this->Position2ZEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->DistanceAnnotationFormatEntry)
    {
    this->DistanceAnnotationFormatEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->DistanceAnnotationScaleEntry)
    {
    this->DistanceAnnotationScaleEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ResolutionEntry)
    {
    this->ResolutionEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

//  this->AddMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveWidgetObservers ( )
{
  if (this->AllVisibilityMenuButton)
    {
    this->AllVisibilityMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RemoveAllRulersButton)
    {
    this->RemoveAllRulersButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ReportButton)
    {
    this->ReportButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->AnnotationFormatMenuButton)
    {
    this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerFromFiducialsButton)
    {
    this->RulerFromFiducialsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->VisibilityButton)
    {
    this->VisibilityButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PointColourButton)
    {
    this->PointColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->Point2ColourButton)
    {
    this->Point2ColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LineColourButton)
    {
    this->LineColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->TextColourButton)
    {
    this->TextColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerSelectorWidget)
    {
    this->RulerSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel2SelectorWidget)
    {
    this->RulerModel2SelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->DistanceAnnotationVisibilityButton)
    {
    this->DistanceAnnotationVisibilityButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->Position1XEntry)
    {
    this->Position1XEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position1YEntry)
    {
    this->Position1YEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position1ZEntry)
    {
    this->Position1ZEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->Position2XEntry)
    {
    this->Position2XEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position2YEntry)
    {
    this->Position2YEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->Position2ZEntry)
    {
    this->Position2ZEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->DistanceAnnotationFormatEntry)
    {
    this->DistanceAnnotationFormatEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->DistanceAnnotationScaleEntry)
    {
    this->DistanceAnnotationScaleEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ResolutionEntry)
    {
    this->ResolutionEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

//  this->RemoveMRMLObservers();
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();

  // to get at some top level icons
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "CreateWidget: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "CreateWidget: got Null SlicerApplicationGUI" );
    return;
    }
  
  // ---
  // GLOBAL CONTROLS FRAME
  vtkSlicerModuleCollapsibleFrame *controlAllFrame = vtkSlicerModuleCollapsibleFrame::New();
  controlAllFrame->SetParent ( this->GetParent() );
  controlAllFrame->Create();
  controlAllFrame->SetLabelText ("Modify All Ruler Nodes" );
  controlAllFrame->ExpandFrame();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                controlAllFrame->GetWidgetName(),
                this->GetParent()->GetWidgetName());

  //---
  //--- create all visibility menu button and set up menu
  //---
  int index = 0;
  this->AllVisibilityMenuButton = vtkKWMenuButton::New();
  this->AllVisibilityMenuButton->SetParent ( controlAllFrame->GetFrame() );
  this->AllVisibilityMenuButton->Create();
  this->AllVisibilityMenuButton->SetBorderWidth(0);
  this->AllVisibilityMenuButton->SetReliefToFlat();
  this->AllVisibilityMenuButton->IndicatorVisibilityOff();
  this->AllVisibilityMenuButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleOrInvisibleIcon() );
  this->AllVisibilityMenuButton->SetBalloonHelpString ( "Set visibility on all ruler nodes." );
  this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "All Rulers Visible");
  index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("All Rulers Visible");
  this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleIcon()  );
  this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "All Rulers Invisible");
  index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("All Rulers Invisible");
  this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, appGUI->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon()  );
  this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AllVisibilityMenuButton->GetMenu()->AddSeparator();
  this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "close");
  index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("close");
  this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);

  // remove all rulers
  this->RemoveAllRulersButton = vtkKWPushButton::New ( );
  this->RemoveAllRulersButton->SetParent ( controlAllFrame->GetFrame() );
  this->RemoveAllRulersButton->Create ( );
  this->RemoveAllRulersButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerDeleteIcon() );
  this->RemoveAllRulersButton->SetReliefToFlat();
  this->RemoveAllRulersButton->SetBorderWidth ( 0 );
  this->RemoveAllRulersButton->SetBalloonHelpString("Delete all rulers.");

  
  this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2",
               this->AllVisibilityMenuButton->GetWidgetName(),
               this->RemoveAllRulersButton->GetWidgetName());

  // generate a report
  this->ReportButton =  vtkKWLoadSaveButton::New();
  this->ReportButton->SetParent ( controlAllFrame->GetFrame() );
  this->ReportButton->Create ( );
  this->ReportButton->GetLoadSaveDialog()->SetFileTypes(" { {CSV} {.csv} } { {Text} {.txt} }");
  this->ReportButton->GetLoadSaveDialog()->SaveDialogOn();
  this->ReportButton->SetText("Generate Report");
  this->ReportButton->SetBalloonHelpString("Generate a report on disk about all rulers.");
  
  this->Script("pack %s -side right -anchor e -padx 2 -pady 2",
               this->ReportButton->GetWidgetName());

  /// create ruler from fids frame
  vtkKWFrame *fromFidsFrame = vtkKWFrame::New();
  fromFidsFrame->SetParent(controlAllFrame->GetFrame());
  fromFidsFrame->Create();
  this->Script ("pack %s -side bottom -anchor nw -fill x -padx 2 -pady 2",
                fromFidsFrame->GetWidgetName());

  /// create ruler from fids button
  this->RulerFromFiducialsButton = vtkKWPushButton::New();
  this->RulerFromFiducialsButton->SetParent ( fromFidsFrame );
  this->RulerFromFiducialsButton->Create ( );
  this->RulerFromFiducialsButton->SetText("Create Ruler from Fiducials");
  this->RulerFromFiducialsButton->SetWidth(27);
  this->RulerFromFiducialsButton->SetBalloonHelpString("Create a new ruler from the last two selected fiducials on the currently active fiducial list (or type Ctrl-m). The fiducials will be deleted once the ruler is made.");
  this->Script( "pack %s -side left -anchor nw -expand false -fill none -padx 2 -pady 2",
                this->RulerFromFiducialsButton->GetWidgetName());
  
  // ---
  // CHOOSE Ruler Node FRAME
  vtkSlicerModuleCollapsibleFrame *pickRulerNodeFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  pickRulerNodeFrame->SetParent ( this->GetParent() );
  pickRulerNodeFrame->Create ( );
  pickRulerNodeFrame->SetLabelText("Modify Selected Ruler Node");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 pickRulerNodeFrame->GetWidgetName() );
  
   // a selector to pick a ruler
  this->RulerSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerSelectorWidget->SetParent ( pickRulerNodeFrame->GetFrame() );
  this->RulerSelectorWidget->Create ( );
  this->RulerSelectorWidget->SetNodeClass("vtkMRMLMeasurementsRulerNode", NULL, NULL, NULL);
  this->RulerSelectorWidget->NewNodeEnabledOn();
  this->RulerSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerSelectorWidget->SetBorderWidth(2);
  // this->RulerSelectorWidget->SetReliefToGroove();
  this->RulerSelectorWidget->SetPadX(2);
  this->RulerSelectorWidget->SetPadY(2);
  this->RulerSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->RulerSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerSelectorWidget->SetLabelText( "Ruler Node Select: ");
  this->RulerSelectorWidget->SetBalloonHelpString("select a ruler node from the current mrml scene (currently only one 3D ruler widget will be displayed at a time, it will update from the selected node).");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->RulerSelectorWidget->GetWidgetName());

   
  /// distance frame
  vtkKWFrame *distanceFrame = vtkKWFrame::New();
  distanceFrame->SetParent(pickRulerNodeFrame->GetFrame());
  distanceFrame->Create();
  this->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                distanceFrame->GetWidgetName());

  this->DistanceLabel = vtkKWLabel::New();
  this->DistanceLabel->SetParent(distanceFrame);
  this->DistanceLabel->Create();
  this->DistanceLabel->SetText("Distance: ");
  this->DistanceLabel->SetForegroundColor(0.0, 0.0, 1.0);
  this->Script( "pack %s -side left -anchor nw -expand false -fill none -padx 2 -pady 2",
                this->DistanceLabel->GetWidgetName());
  
  //
  // Pick Models Frame
  //
  vtkKWFrame *modelFrame = vtkKWFrame::New();
  modelFrame->SetParent( pickRulerNodeFrame->GetFrame() );
  modelFrame->Create();
  //modelFrame->SetLabelText("Constrain Ruler to Models or Slices");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", modelFrame->GetWidgetName());

  

  // end 1
  vtkKWFrame *end1Frame = vtkKWFrame::New();
  end1Frame->SetParent(modelFrame);
  end1Frame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 end1Frame->GetWidgetName() );
  
  this->PointColourButton = vtkKWChangeColorButton::New();
  this->PointColourButton->SetParent ( end1Frame );
  this->PointColourButton->Create ( );
  this->PointColourButton->LabelOutsideButtonOn();
  this->PointColourButton->SetLabelPositionToRight();
  this->PointColourButton->SetLabelText("");
  this->PointColourButton->SetBalloonHelpString("set end point 1 color.");

  
  this->RulerModel1SelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerModel1SelectorWidget->SetParent ( end1Frame );
  this->RulerModel1SelectorWidget->Create ( );
  this->RulerModel1SelectorWidget->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->RulerModel1SelectorWidget->AddNodeClass("vtkMRMLSliceNode", NULL, NULL, NULL);
  this->RulerModel1SelectorWidget->SetChildClassesEnabled(1);
  this->RulerModel1SelectorWidget->NoneEnabledOn();
  this->RulerModel1SelectorWidget->SetShowHidden(1);
  this->RulerModel1SelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerModel1SelectorWidget->SetBorderWidth(0);
  this->RulerModel1SelectorWidget->SetPadX(2);
  this->RulerModel1SelectorWidget->SetPadY(0);
  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
//  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerModel1SelectorWidget->SetLabelText( "Constraint:");
  this->RulerModel1SelectorWidget->SetLabelWidth(11);
//  this->RulerModel1SelectorWidget->GetLabel()->SetForegroundColor(1, 0, 0);
  this->RulerModel1SelectorWidget->SetBalloonHelpString("Select a model or slice on which to anchor the first end of the ruler. Make sure that the handle is rendered on top of the model or slice before you select it from this menu.");

  
  this->Position1XEntry = vtkKWEntry::New();
  this->Position1XEntry->SetParent(end1Frame);
  this->Position1XEntry->Create();
  this->Position1XEntry->SetWidth(8);
  this->Position1XEntry->SetRestrictValueToDouble();
  this->Position1XEntry->SetBalloonHelpString("First end of the line, X position");

  this->Position1YEntry = vtkKWEntry::New();
  this->Position1YEntry->SetParent(end1Frame);
  this->Position1YEntry->Create();
  this->Position1YEntry->SetWidth(8);
  this->Position1YEntry->SetRestrictValueToDouble();
  this->Position1YEntry->SetBalloonHelpString("First end of the line, Y position");

  this->Position1ZEntry = vtkKWEntry::New();
  this->Position1ZEntry->SetParent(end1Frame);
  this->Position1ZEntry->Create();
  this->Position1ZEntry->SetWidth(8);
  this->Position1ZEntry->SetRestrictValueToDouble();
  this->Position1ZEntry->SetBalloonHelpString("First end of the line, Z position");
  
  this->Script ( "pack %s %s %s %s %s -side left -anchor nw -fill x -padx 2 -pady 2",
                 this->PointColourButton->GetWidgetName(),
                 this->RulerModel1SelectorWidget->GetWidgetName(),
                 this->Position1XEntry->GetWidgetName(),
                 this->Position1YEntry->GetWidgetName(),
                 this->Position1ZEntry->GetWidgetName());

  // end 2

  vtkKWFrame *end2Frame = vtkKWFrame::New();
  end2Frame->SetParent(modelFrame);
  end2Frame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 end2Frame->GetWidgetName() );
  
  this->Point2ColourButton = vtkKWChangeColorButton::New();
  this->Point2ColourButton->SetParent ( end2Frame );
  this->Point2ColourButton->Create ( );
  this->Point2ColourButton->LabelOutsideButtonOn();
  this->Point2ColourButton->SetLabelPositionToRight();
  this->Point2ColourButton->SetLabelText("");
  this->Point2ColourButton->SetBalloonHelpString("set point 2 color.");
  
  this->RulerModel2SelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerModel2SelectorWidget->SetParent ( end2Frame );
  this->RulerModel2SelectorWidget->Create ( );
  this->RulerModel2SelectorWidget->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->RulerModel2SelectorWidget->AddNodeClass("vtkMRMLSliceNode", NULL, NULL, NULL);
  this->RulerModel2SelectorWidget->SetChildClassesEnabled(1);
  this->RulerModel2SelectorWidget->NoneEnabledOn();
  this->RulerModel2SelectorWidget->SetShowHidden(1);
  this->RulerModel2SelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerModel2SelectorWidget->SetBorderWidth(0);
  this->RulerModel2SelectorWidget->SetPadX(2);
  this->RulerModel2SelectorWidget->SetPadY(0);
  this->RulerModel2SelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
//  this->RulerModel2SelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerModel2SelectorWidget->SetLabelText( "Constraint: ");
  this->RulerModel2SelectorWidget->SetLabelWidth(11);
//  this->RulerModel2SelectorWidget->GetLabel()->SetForegroundColor(0, 0, 1);
  this->RulerModel2SelectorWidget->SetBalloonHelpString("Select a model or slice on which to anchor the second end of the ruler. Make sure that the handle is rendered on top of the model or slice before you select it from this menu.");

  this->Position2XEntry = vtkKWEntry::New();
  this->Position2XEntry->SetParent(end2Frame);
  this->Position2XEntry->Create();
  this->Position2XEntry->SetWidth(8);
  this->Position2XEntry->SetRestrictValueToDouble();
  this->Position2XEntry->SetBalloonHelpString("Second end of the line, X position");

  this->Position2YEntry = vtkKWEntry::New();
  this->Position2YEntry->SetParent(end2Frame);
  this->Position2YEntry->Create();
  this->Position2YEntry->SetWidth(8);
  this->Position2YEntry->SetRestrictValueToDouble();
  this->Position2YEntry->SetBalloonHelpString("Second end of the line, Y position");

  this->Position2ZEntry = vtkKWEntry::New();
  this->Position2ZEntry->SetParent(end2Frame);
  this->Position2ZEntry->Create();
  this->Position2ZEntry->SetWidth(8);
  this->Position2ZEntry->SetRestrictValueToDouble();
  this->Position2ZEntry->SetBalloonHelpString("Second end of the line, Z position");
  
  this->Script ( "pack %s %s %s %s %s -side left -anchor nw -fill x -padx 2 -pady 2",
                 this->Point2ColourButton->GetWidgetName(),
                 this->RulerModel2SelectorWidget->GetWidgetName(),
                 this->Position2XEntry->GetWidgetName(),
                 this->Position2YEntry->GetWidgetName(),
                 this->Position2ZEntry->GetWidgetName());

  // ---
  // DISPLAY FRAME            
  vtkKWFrameWithLabel *rulerDisplayFrame = vtkKWFrameWithLabel::New ( );
  rulerDisplayFrame->SetParent ( pickRulerNodeFrame->GetFrame() );
  rulerDisplayFrame->SetLabelText("Display Options");
  rulerDisplayFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 rulerDisplayFrame->GetWidgetName() );
  rulerDisplayFrame->ExpandFrame ( );

  
  vtkKWFrame *visibColourFrame = vtkKWFrame::New();
  visibColourFrame->SetParent(rulerDisplayFrame->GetFrame());
  visibColourFrame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 visibColourFrame->GetWidgetName() );

  this->VisibilityButton = vtkKWPushButton::New();
  this->VisibilityButton->SetParent ( visibColourFrame );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetOverReliefToNone();
  this->VisibilityButton->SetBorderWidth(0);
  this->VisibilityButton->SetReliefToFlat();
  this->VisibilityButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleIcon());
  this->VisibilityButton->SetBalloonHelpString("set widget visibility.");
  
    
  this->LineColourButton = vtkKWChangeColorButton::New();
  this->LineColourButton->SetParent ( visibColourFrame );
  this->LineColourButton->Create ( );
  this->LineColourButton->LabelOutsideButtonOn();
  this->LineColourButton->SetLabelPositionToLeft();
  this->LineColourButton->SetLabelText("Line Color:");
  this->LineColourButton->SetBalloonHelpString("set line color.");
  
  this->TextColourButton = vtkKWChangeColorButton::New();
  this->TextColourButton->SetParent ( visibColourFrame );
  this->TextColourButton->Create ( );
  this->TextColourButton->LabelOutsideButtonOn();
  this->TextColourButton->SetLabelPositionToLeft();
  this->TextColourButton->SetLabelText("Annotation Color:");
  this->TextColourButton->SetBalloonHelpString("set the color used to show the distance annotation text.");

  this->DistanceAnnotationVisibilityButton = vtkKWPushButton::New();
  this->DistanceAnnotationVisibilityButton->SetParent ( visibColourFrame );
  this->DistanceAnnotationVisibilityButton->Create ( );
  this->DistanceAnnotationVisibilityButton->SetOverReliefToNone();
  this->DistanceAnnotationVisibilityButton->SetBorderWidth(0);
  this->DistanceAnnotationVisibilityButton->SetReliefToFlat();
  this->DistanceAnnotationVisibilityButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerVisibleIcon());
  this->DistanceAnnotationVisibilityButton->SetText("Annotation visibility:");
  this->DistanceAnnotationVisibilityButton->SetBalloonHelpString("set distance annotation visibility.");
  
  this->Script ( "pack %s %s %s %s -side left -padx 2 -pady 2 -expand true",
                 this->VisibilityButton->GetWidgetName(),
                 this->LineColourButton->GetWidgetName(),
                 this->TextColourButton->GetWidgetName(),
                 this->DistanceAnnotationVisibilityButton->GetWidgetName());

  //---
  //--- create distance annotation format menu button and set up menu
  //---
  this->AnnotationFormatMenuButton = vtkKWMenuButtonWithLabel::New();
  this->AnnotationFormatMenuButton->SetParent ( rulerDisplayFrame->GetFrame() );
  this->AnnotationFormatMenuButton->Create();
  this->AnnotationFormatMenuButton->SetLabelText("Standard Annotation Formats");
  this->AnnotationFormatMenuButton->SetLabelWidth(29);
  this->AnnotationFormatMenuButton->GetWidget()->IndicatorVisibilityOff();
  this->AnnotationFormatMenuButton->SetBalloonHelpString ("Select a standard annotation format. Warning: will undo any custom text in the distance annotation entry box." );
  
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->AddRadioButton("1 decimal");
  index = this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->GetIndexOfItem ("1 decimal");
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->AddRadioButton("0 decimals");
  index = this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->GetIndexOfItem ("0 decimals");
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->AddRadioButton("2 decimals");
  index = this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->GetIndexOfItem ("2 decimals");
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->AddRadioButton("Scientific Notation");
  index = this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->GetIndexOfItem ("Scientific Notation");
  this->AnnotationFormatMenuButton->GetWidget()->GetMenu()->SetItemIndicatorVisibility ( index, 0);
  this->AnnotationFormatMenuButton->GetWidget()->SetWidth(21);
  this->AnnotationFormatMenuButton->GetWidget()->SetValue("1 decimal");
  
  this->DistanceAnnotationFormatEntry = vtkKWEntryWithLabel::New();
  this->DistanceAnnotationFormatEntry->SetParent(rulerDisplayFrame->GetFrame());
  this->DistanceAnnotationFormatEntry->Create();
  this->DistanceAnnotationFormatEntry->SetLabelText("Distance Annotation Format");
  this->DistanceAnnotationFormatEntry->SetLabelWidth(29);
  this->DistanceAnnotationFormatEntry->SetBalloonHelpString("String formatting command, use the defaults from the menu or customise it in this entry. Use %g to print out distance in a default floating point format, %.1f to print out only one digit after the decimal, plus any text you wish");
  this->Script ( "pack %s %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->AnnotationFormatMenuButton->GetWidgetName(),
                 this->DistanceAnnotationFormatEntry->GetWidgetName());

  this->DistanceAnnotationScaleEntry =  vtkKWEntryWithLabel::New();
  this->DistanceAnnotationScaleEntry->SetParent(rulerDisplayFrame->GetFrame());
  this->DistanceAnnotationScaleEntry->Create();
  this->DistanceAnnotationScaleEntry->SetLabelText("Distance Annotation Scale");
  this->DistanceAnnotationScaleEntry->SetLabelWidth(29);
  this->DistanceAnnotationScaleEntry->GetWidget()->SetRestrictValueToDouble();
  this->DistanceAnnotationScaleEntry->SetBalloonHelpString("Scale value applied to the distance annotation text");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->DistanceAnnotationScaleEntry->GetWidgetName());

  this->ResolutionEntry = vtkKWEntryWithLabel::New();
  this->ResolutionEntry->SetParent(rulerDisplayFrame->GetFrame());
  this->ResolutionEntry->Create();
  this->ResolutionEntry->SetLabelText("Resolution");
  this->ResolutionEntry->SetLabelWidth(29);
  this->ResolutionEntry->SetBalloonHelpString(" number of subdivisions on the line");
  this->ResolutionEntry->GetWidget()->SetRestrictValueToInteger();
  // this is not used with the current line widget
  //this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
  //               this->ResolutionEntry->GetWidgetName());
  
  // add observers
  this->AddWidgetObservers();

  modelFrame->Delete();
  rulerDisplayFrame->Delete();
  visibColourFrame->Delete();
  fromFidsFrame->Delete();
  distanceFrame->Delete();
  pickRulerNodeFrame->Delete();
  controlAllFrame->Delete();
  end1Frame->Delete();
  end2Frame->Delete();
  
  // register node classes
  if (this->GetMRMLScene())
    {
    vtkMRMLMeasurementsRulerNode *rulerNode = vtkMRMLMeasurementsRulerNode::New();
    this->GetMRMLScene()->RegisterNodeClass(rulerNode);
    rulerNode->Delete();
    rulerNode = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  if (this->ViewerWidget != NULL)
    {
    // TODO: figure out if this is necessary
    this->RemoveDistanceWidgets();
    if (this->ViewerWidget->HasObserver(
          vtkSlicerViewerWidget::ActiveCameraChangedEvent, 
          this->GUICallbackCommand) == 1)
      {
      this->ViewerWidget->RemoveObservers(
        vtkSlicerViewerWidget::ActiveCameraChangedEvent, 
        (vtkCommand *)this->GUICallbackCommand);
      }
    }
    
  this->ViewerWidget = viewerWidget;

  if (this->ViewerWidget)
    {
    this->ViewerWidget->AddObserver(
      vtkSlicerViewerWidget::ActiveCameraChangedEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }

  vtkDebugMacro("SetViewerWidget: Updating any widget interactors");
  this->UpdateRulerWidgetInteractors();
}

//---------------------------------------------------------------------------
vtkMeasurementsDistanceWidgetClass *vtkMeasurementsRulerWidget::GetDistanceWidget(const char *nodeID)
{
  std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
  for (iter = this->DistanceWidgets.begin();
       iter != this->DistanceWidgets.end();
       iter++)
    {
    if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
      {
      return iter->second;
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::AddDistanceWidget(vtkMRMLMeasurementsRulerNode *rulerNode)
{
  if (!rulerNode)
    {
    return;
    }
  if (this->GetDistanceWidget(rulerNode->GetID()) != NULL)
    {
    vtkDebugMacro("Already have widgets for ruler node " << rulerNode->GetID());
    return;
    }

  // reset the ruler node's position according to the viewer widget bounding
  // box, but only if the ruler node is at default location
  if (this->ViewerWidget)
    {
    vtkBoundingBox *box = this->ViewerWidget->GetBoxAxisBoundingBox();
    if (box)
      {
      // check if the ruler node has been init
      vtkMRMLMeasurementsRulerNode *rnode = vtkMRMLMeasurementsRulerNode::New();
      double *defaultP1 = rnode->GetPosition1();
      double *defaultP2 = rnode->GetPosition2();
      double *p1 = rulerNode->GetPosition1();
      double *p2 = rulerNode->GetPosition2();
      if (defaultP1 && defaultP2 && p1 && p2 &&
          p1[0] == defaultP1[0] && p1[1] == defaultP1[1] && p1[2] == defaultP1[2] &&
          p2[0] == defaultP2[0] && p2[1] == defaultP2[1] && p2[2] == defaultP2[2])
        {
        double maxLength = box->GetMaxLength();
        double minPoint[3], maxPoint[3];
        box->GetMinPoint(minPoint[0], minPoint[1], minPoint[2]);
        box->GetMaxPoint(maxPoint[0], maxPoint[1], maxPoint[2]);
        for (unsigned int i = 0; i < 3; i++)
          {
          if (box->GetLength(i) == 0.0)
            {
            minPoint[i] = minPoint[i] - maxLength * .05;
            maxPoint[i] = maxPoint[i] + maxLength * .05;
            }
          }
       
        double newP1[3] = {0.0, 0.0, 0.0};
        double newP2[3] = {0.0, 0.0, 0.0};
        // make a new pair of points on either side of the bounding box
        newP1[0] = minPoint[0];
        newP1[1] = minPoint[1] + (maxPoint[1] - minPoint[1])/ 2.0;
        newP1[2] = minPoint[2] + (maxPoint[2] - minPoint[2])/ 2.0;
        newP2[0] = maxPoint[0];
        newP2[1] = newP1[1];
        newP2[2] = newP1[2];
        rulerNode->SetPosition1(newP1);
        rulerNode->SetPosition2(newP2);
        }
       rnode->Delete();
      }
    }
  vtkMeasurementsDistanceWidgetClass *c = vtkMeasurementsDistanceWidgetClass::New();
  this->DistanceWidgets[rulerNode->GetID()] = c;
  // make sure we're observing the node for transform changes
  if (rulerNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    rulerNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
  if (rulerNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    rulerNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
  //vtkIntArray *events = vtkIntArray::New();
  //events->InsertNextValue(vtkCommand::ModifiedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  //events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  //vtkSetAndObserveMRMLNodeEventsMacro(NULL, rulerNode, events);
  //events->Delete();
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveDistanceWidget(vtkMRMLMeasurementsRulerNode *rulerNode)
{
  if (!rulerNode)
    {
    return;
    }
  vtkMeasurementsDistanceWidgetClass *rulerWidget = this->GetDistanceWidget(rulerNode->GetID());
  if (rulerWidget != NULL)
    {
    // remove observers
    rulerWidget->GetWidget()->RemoveObservers(vtkCommand::InteractionEvent);
    rulerWidget->GetWidget()->RemoveObservers(vtkCommand::StartInteractionEvent);
    rulerWidget = NULL;
    this->DistanceWidgets[rulerNode->GetID()]->Delete();
    // need to use find and erase with the iterator to really erase it
    std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
    iter = this->DistanceWidgets.find(rulerNode->GetID());
    if (iter != this->DistanceWidgets.end())
      {
      this->DistanceWidgets.erase(iter);
      }
    // request a render
    if (this->ViewerWidget)
      {
      this->ViewerWidget->RequestRender();
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveDistanceWidgets()
{
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLMeasurementsRulerNode");
  vtkDebugMacro("RemoveDistanceWidgets: have " << nnodes << " ruler  nodes in the scene, " << this->DistanceWidgets.size() << " widgets defined already");

  if (nnodes == 0)
    {
    // the scene was closed, all the nodes are gone, so do this w/o reference
    // to the nodes
    vtkDebugMacro("RemoveDistanceWidgets: no ruler nodes in scene, removing ruler widgets w/o reference to nodes");
    std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
    for (iter = this->DistanceWidgets.end();
         iter != this->DistanceWidgets.end();
         iter++)
      {
      vtkDebugMacro("RemoveDistanceWidgets: deleting and erasing " << iter->first);
      iter->second->Delete();
      this->DistanceWidgets.erase(iter);
      }
    this->DistanceWidgets.clear();
    }
  else
    {
    for (int n=0; n<nnodes; n++)
      {
      vtkMRMLMeasurementsRulerNode *rnode = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLMeasurementsRulerNode"));
      if (rnode)
        {
        this->RemoveDistanceWidget(rnode);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::Update3DWidgetsFromMRML()
{
  if (!this->MRMLScene)
    {
      vtkDebugMacro("UpdateFromMRML: no mrml scene from which to update!");
      return;
    }

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLMeasurementsRulerNode");
  vtkDebugMacro("UpdateFromMRML: have " << nnodes << " ruler nodes in the scene, " << this->DistanceWidgets.size() << " widgets defined already");

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLMeasurementsRulerNode *rnode = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLMeasurementsRulerNode"));
    if (rnode)
      {
      // this call will create one if it's missing
      this->Update3DWidget(rnode);
      // let go of the pointer
      rnode = NULL;
      }
    }

  // now have a widget for each node, check that don't have too many widgets
  if ((int)(this->DistanceWidgets.size()) != nnodes)
    {
    vtkDebugMacro("UpdateFromMRML: after adding widgets for scene nodes, have " << this->DistanceWidgets.size() << " instead of " << nnodes);
    // find ones that aren't in the scene, be careful using an iterator because calling erase gets it messed up
    //int numWidgets = this->DistanceWidgets.size();
    std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
    std::vector<std::string> idsToDelete;
    for (iter = this->DistanceWidgets.begin();
         iter != this->DistanceWidgets.end();
         iter++)
      {
      if (this->MRMLScene->GetNodeByID(iter->first.c_str()) == NULL)
        {
        vtkDebugMacro("UpdateFromMRML: found an extra widget with id " << iter->first.c_str());
        // add it to a list and do delete and erase in a second round
        idsToDelete.push_back(iter->first);
        }
      }
    for (int i = 0; i < (int)(idsToDelete.size()); i++)
      {
      std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator delIter;
      delIter = this->DistanceWidgets.find(idsToDelete[i]);
      if (delIter != this->DistanceWidgets.end())
        {
        // can't call this->RemoveDistanceWidget because we don't have a node!
        this->DistanceWidgets[delIter->first.c_str()]->Delete();
        this->DistanceWidgets.erase(delIter->first.c_str());
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ModifyAllRulerVisibility( int visibilityState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyAllRulerVisibility: got NULL MRMLScene." );
    return;
    }
  if ( visibilityState != 0 && visibilityState != 1 )
    {
    vtkErrorMacro ( "ModifyAllRulerVisibility: got bad value for lock state; should be 0 or 1" );
    return;
    }
  
  vtkMRMLMeasurementsRulerNode *rulerNode;
  
  // save state for undo:
  // maybe we should just make a list of all the ruler nodes
  // and save their state here instead of the entire scene?
  this->MRMLScene->SaveStateForUndo();
  int numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLMeasurementsRulerNode" );
  for ( int nn=0; nn<numnodes; nn++ )
    {
    rulerNode = vtkMRMLMeasurementsRulerNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLMeasurementsRulerNode" ));
    if ( rulerNode != NULL )
      {
      rulerNode->SetVisibility ( visibilityState );
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateLabelsFromNode(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{
  if (activeRulerNode == NULL)
    {
    vtkDebugMacro("UpdateLabelsFromNode: passed in ruler node is null, returning");
    return;
    }

  return;
  
  double *rgb1 = activeRulerNode->GetPointColour();
  double *rgb2 = activeRulerNode->GetPoint2Colour();

  if (!rgb1 || !rgb2)
    {
    vtkErrorMacro("UpdateLabelsFromNode: null point colours in node, returning.");
    return;
    }

  // check for white, set it a bit grey
  if (rgb1[0] > 0.9 &&
      rgb1[1] > 0.9 &&
      rgb1[2] > 0.9)
    {
    rgb1[0] = 0.9;
    rgb1[1] = 0.9;
    rgb1[2] = 0.9;
    }
  if (rgb2[0] > 0.9 &&
      rgb2[1] > 0.9 &&
      rgb2[2] > 0.9)
    {
    rgb2[0] = 0.9;
    rgb2[1] = 0.9;
    rgb2[2] = 0.9;
    }
  // match the positions to the end points
  this->Position1Label->SetForegroundColor(rgb1);
  this->Position2Label->SetForegroundColor(rgb2);
  
  // match the models to the end points
  this->RulerModel1SelectorWidget->GetLabel()->SetForegroundColor(rgb1);
  this->RulerModel2SelectorWidget->GetLabel()->SetForegroundColor(rgb2);
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateCamera()
{
  vtkCamera *cam = NULL;
  cam = this->GetActiveCamera();
  
  if (cam == NULL)
    {
    vtkErrorMacro("UpdateCamera: unable to get active camera");
    return;
    }

  std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
  for (iter = this->DistanceWidgets.begin(); iter !=  this->DistanceWidgets.end(); iter++)
    {
    iter->second->SetCamera(cam);
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateRulerWidgetInteractors()
{
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
      this->GetViewerWidget()->GetMainViewer() == NULL ||
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
    {
    vtkDebugMacro("UpdateRulerWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
    }

  std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
  for (iter = this->DistanceWidgets.begin(); iter !=  this->DistanceWidgets.end(); iter++)
    {
    if (iter->second->GetWidget())
      {
      if (isNull)
        {
        iter->second->GetWidget()->SetInteractor(NULL);
        }
      else
        {
        iter->second->GetWidget()->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
        }
      // now update the visibility for the ruler
      vtkMRMLMeasurementsRulerNode *rulerNode = NULL;
      if (this->GetMRMLScene())
        {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
          {
          rulerNode = vtkMRMLMeasurementsRulerNode::SafeDownCast(node);
          }
        }
      if (rulerNode != NULL)
        {
        this->Update3DWidgetVisibility(rulerNode);
        }
      }
    }
}


//---------------------------------------------------------------------------
vtkCamera *vtkMeasurementsRulerWidget::GetActiveCamera()
{
  vtkKWRenderWidget *mainViewer = NULL;
  if (this->GetViewerWidget())
    {
    mainViewer = this->GetViewerWidget()->GetMainViewer();
    }

  if (mainViewer && 
      mainViewer->GetRenderer() &&
      mainViewer->GetRenderer()->IsActiveCameraCreated())
    {
    return mainViewer->GetRenderer()->GetActiveCamera();
    }
  return NULL;
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::GenerateReport(const char *filename)
{
  if (filename == NULL)
    {
    vtkErrorMacro("GenerateReport: no file to save to!");
    return;
    }
  else
    {
    vtkDebugMacro("Saving to file " << filename);
    }
  if (this->MRMLScene == NULL)
    {
    return;
    }
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLMeasurementsRulerNode");
  if (nnodes == 0)
    {
    vtkErrorMacro("GenerateReport: no nodes to report upon!");
    return;
    }
  // now save to file
  FILE *fp = fopen(filename, "w");
  if (!fp)
    {
    vtkErrorMacro("GenerateReport: unable to open file " << filename << " for writing");
    return;
    }
  const char *separator = ",";
  std::string fileExt = vtksys::SystemTools::GetFilenameExtension(filename);
  if (fileExt.compare(".txt") == 0 ||
      fileExt.compare(".text") == 0)
    {
    separator = "\t";
    }
  // write a header with info about the active volume?

  fprintf(fp, "# distance%sp1x%sp1y%sp1z%sp2x%sp2y%sp2z%sname\n",
          separator,
          separator,separator,separator,
          separator,separator,separator);
  // iterate over nodes
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLMeasurementsRulerNode *rulerNode = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLMeasurementsRulerNode"));
    double *p1 = rulerNode->GetPosition1();
    double *p2 = rulerNode->GetPosition2();
    if (!p1 || !p2)
      {
      vtkErrorMacro("GenerateReport: positions invalid on ruler " << rulerNode->GetName());
      }
    else
      {
      double distance = rulerNode->GetDistance();
      fprintf(fp, "%g%s%g%s%g%s%g%s%g%s%g%s%g%s\"%s\"\n",
              distance, separator,
              p1[0], separator,
              p1[1], separator,
              p1[2], separator,
              p2[0], separator,
              p2[1], separator,
              p2[2], separator,
              (rulerNode->GetName() == NULL ? "null" : rulerNode->GetName()));
      }
    }
  fclose(fp);
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ResetGUI()
{
  if (this->DistanceLabel)
    {
    this->DistanceLabel->SetText("Distance: ");
    this->DistanceLabel->SetForegroundColor(0.0, 0.0, 1.0);
    }
  if ( this->Position1XEntry)
    {
    this->Position1XEntry->SetValue(0);
    }
  if ( this->Position1YEntry)
    {
    this->Position1YEntry->SetValue(0);
    }
  if ( this->Position1ZEntry)
    {
    this->Position1ZEntry->SetValue(0);
    }
  if ( this->Position2XEntry)
    {
    this->Position2XEntry->SetValue(0);
    }
  if ( this->Position2YEntry)
    {
    this->Position2YEntry->SetValue(0);
    }
  if ( this->Position2ZEntry)
    {
    this->Position2ZEntry->SetValue(0);
    }
  if (this->PointColourButton)
    {
    this->PointColourButton->SetColor(1.0, 1.0, 1.0);
    }
  if (this->Point2ColourButton)
    {
    this->Point2ColourButton->SetColor(1.0, 1.0, 1.0);
    }
  if (this->LineColourButton)
    {
    this->LineColourButton->SetColor(1.0, 1.0, 1.0);
    }
  if (this->TextColourButton)
    {
    this->TextColourButton->SetColor(1.0, 1.0, 1.0);
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget:: ModifyAllLock(int lockFlag)
{
  std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
  for (iter = this->DistanceWidgets.begin(); iter !=  this->DistanceWidgets.end(); iter++)
    {
    if (iter->second &&
        iter->second->GetWidget())
      {
      iter->second->GetWidget()->SetProcessEvents(!lockFlag);
      }
    }
}
