#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

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

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

#include "vtkMRMLMeasurementsRulerNode.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"


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
              this->RulerNode->SetPosition1(p1);
              this->RulerNode->SetPosition2(p2);
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
  this->RulerModel1SelectorWidget = NULL;
  this->RulerModel2SelectorWidget = NULL;
  this->PointColourButton = NULL;
  this->LineColourButton = NULL;
 
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

  // 3d elements
  this->ViewerWidget = NULL;

  this->RulerModel1PointPlacer = vtkPolygonalSurfacePointPlacer::New();
  this->RulerModel2PointPlacer = vtkPolygonalSurfacePointPlacer::New();

  this->DistanceHandleRepresentation = vtkPointHandleRepresentation3D::New();
  this->DistanceHandleRepresentation->GetProperty()->SetColor(1, 0, 0);

  this->DistanceRepresentation = vtkLineRepresentation::New();
  this->DistanceRepresentation->SetHandleRepresentation(this->DistanceHandleRepresentation);
  this->DistanceRepresentation->DistanceAnnotationVisibilityOn();
  this->DistanceRepresentation->SetDistanceAnnotationFormat("%g mm");
  // have to set a scale or else it will scale with the length of the line
  double scale[3];
  scale[0] = 10.0;
  scale[1] = 10.0;
  scale[2] = 10.0;
  this->DistanceRepresentation->SetDistanceAnnotationScale(scale);
  // unfortunately, the handle representation is cloned, can't have them
  // different colours yet
  this->DistanceRepresentation->GetPoint1Representation()->GetProperty()->SetColor(1, 0, 0);
  this->DistanceRepresentation->GetPoint2Representation()->GetProperty()->SetColor(0, 0, 1);

  this->DistanceWidget = vtkLineWidget2::New();
  this->DistanceWidget->EnabledOff();
  this->DistanceWidget->CreateDefaultRepresentation();
  this->DistanceWidget->SetRepresentation(this->DistanceRepresentation);
  
  this->SetRulerNodeID(NULL);
  
  this->UpdatingMRML = 0;
  this->UpdatingWidget = 0;
  this->Updating3DWidget = 0;
  this->ProcessingMRMLEvent = 0;
  this->ProcessingWidgetEvent = 0;
}


//---------------------------------------------------------------------------
vtkMeasurementsRulerWidget::~vtkMeasurementsRulerWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  // gui elements
  if (this->RulerSelectorWidget)
    {
    this->RulerSelectorWidget->SetParent(NULL);
    this->RulerSelectorWidget->SetMRMLScene(NULL);
    this->RulerSelectorWidget->Delete();
    this->RulerSelectorWidget = NULL;
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
  if (this->LineColourButton)
    {
    this->LineColourButton->SetParent(NULL);
    this->LineColourButton->Delete();
    this->LineColourButton= NULL;
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
  if (this->RulerModel1PointPlacer)
    {
    this->RulerModel1PointPlacer->Delete();
    this->RulerModel1PointPlacer = NULL;
    }
  if (this->RulerModel2PointPlacer)
    {
    this->RulerModel2PointPlacer->Delete();
    this->RulerModel2PointPlacer = NULL;
    }
  if (this->DistanceHandleRepresentation)
    {
    this->DistanceHandleRepresentation->Delete();
    this->DistanceHandleRepresentation = NULL;
    }
  if (this->DistanceRepresentation)
    {
    this->DistanceRepresentation->SetHandleRepresentation(NULL);
    this->DistanceRepresentation->Delete();
    this->DistanceRepresentation = NULL;
    }
  if (this->DistanceWidget)
    {
    this->DistanceWidget->SetRepresentation(NULL);
    this->DistanceWidget->Delete();
    this->DistanceWidget = NULL;
    }

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
    if (this->DistanceWidget)
      {
      os << indent << "Distance Widget:\n";
      this->DistanceWidget->PrintSelf(os, indent);
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
  
  // get the old node
  vtkMRMLMeasurementsRulerNode *oldRuler = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetRulerNodeID()));
 
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
  // set up observers on the new node
  if (newRuler != NULL)
    {
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
//    events->InsertNextValue(vtkMRMLMeasurementsRulerNode::DisplayModifiedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(oldRuler, newRuler, events);
    events->Delete();

    // set up the GUI
    this->UpdateWidget(newRuler);
    }
  else
    {
    vtkDebugMacro ("ERROR: unable to get the mrml fiducial node to observe!\n");
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->ProcessingMRMLEvent != 0)
    {
    vtkDebugMacro("ProcessWidgetEvents: already processing mrml event " << this->ProcessingMRMLEvent);
    return;
    }
   if (this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessWidgetEvents: already processing widget event " << this->ProcessingWidgetEvent);
    return;
    }
  
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "ProcessWidgetEvents: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplicationGUI" );
    return;
    }
  
  this->ProcessingWidgetEvent = event;

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
      this->ProcessingWidgetEvent = 0;
      return;
    }

  // get the currently displayed list
  
  // is there one list?
  vtkMRMLMeasurementsRulerNode *activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
  if (activeRulerNode == NULL)
    {
    vtkDebugMacro("No selected ruler");
    this->ProcessingWidgetEvent = 0;
    return;
    /*
      not allowing spontaneous creation of a ruler
    vtkDebugMacro ("ERROR: No Fiducial List, adding one first!\n");
      vtkMRMLMeasurementsRulerNode *newList = this->GetLogic()->AddRuler();      
      if (newList != NULL)
        {
        this->SetRulerNodeID(newList->GetID());
        newList->Delete();
        }
      else
        {
        vtkErrorMacro("Unable to add a new fid list via the logic\n");
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
  vtkKWCheckButton *b = vtkKWCheckButton::SafeDownCast ( caller );
  vtkKWChangeColorButton *ccbutton = vtkKWChangeColorButton::SafeDownCast(caller);
  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);
 
  if (b && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    if (b == this->VisibilityButton->GetWidget()) 
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetVisibility(this->VisibilityButton->GetWidget()->GetSelectedState());
      this->Update3DWidget(activeRulerNode);
      }
    else if (b == this->DistanceAnnotationVisibilityButton->GetWidget() )
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetDistanceAnnotationVisibility(this->DistanceAnnotationVisibilityButton->GetWidget()->GetSelectedState());
      this->Update3DWidget(activeRulerNode);
      }
    }
  else if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->RulerModel1SelectorWidget &&
           event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->RulerModel1SelectorWidget->GetSelected());
    if (model != NULL  && model->GetDisplayNode() != NULL && this->GetViewerWidget())
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetModelID1(model->GetID());
      this->Update3DWidget(activeRulerNode);
      }
    }
  else if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->RulerModel2SelectorWidget &&
           event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->RulerModel2SelectorWidget->GetSelected());
    if (model != NULL  && model->GetDisplayNode() != NULL && this->GetViewerWidget())
      {
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetModelID2(model->GetID());
      this->Update3DWidget(activeRulerNode);
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
    this->ProcessingWidgetEvent = 0;
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
    this->Update3DWidget(activeRulerNode);
    }
  else if (entry && event == vtkKWEntry::EntryValueChangedEvent)
    {
    if (entry == this->Position1XEntry ||
        entry == this->Position1YEntry ||
        entry == this->Position1ZEntry)
      {
      double x = this->Position1XEntry->GetValueAsDouble();
      double y = this->Position1YEntry->GetValueAsDouble();
      double z = this->Position1ZEntry->GetValueAsDouble();
      if (this->MRMLScene) { this->MRMLScene->SaveStateForUndo(activeRulerNode); }
      activeRulerNode->SetPosition1(x, y, z);
      }
    else if (entry == this->Position2XEntry ||
             entry == this->Position2YEntry ||
             entry == this->Position2ZEntry)
      {
      double x = this->Position2XEntry->GetValueAsDouble();
      double y = this->Position2YEntry->GetValueAsDouble();
      double z = this->Position2ZEntry->GetValueAsDouble();
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
  // now update the mrml node with all the changed values from the GUI
  //this->UpdateMRML();

  this->ProcessingWidgetEvent = 0;
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateMRMLFromGUI()
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;

  // is there a ruler node?
  vtkMRMLMeasurementsRulerNode *activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
  if (activeRulerNode == NULL)
    {
    vtkErrorMacro("No selected ruler");
    this->UpdatingMRML = 0;
    return;
    }
  
  int visibility = this->VisibilityButton->GetWidget()->GetSelectedState();
  activeRulerNode->SetVisibility(visibility);
  
    
  double *rgb = this->PointColourButton->GetColor();
  double *rgb1 = activeRulerNode->GetPointColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    activeRulerNode->SetPointColour(this->PointColourButton->GetColor());
    }
  rgb = this->LineColourButton->GetColor();
  rgb1 = activeRulerNode->GetLineColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    activeRulerNode->SetLineColour(this->LineColourButton->GetColor());
    }

  double x = this->Position1XEntry->GetValueAsDouble();
  double y = this->Position1YEntry->GetValueAsDouble();
  double z = this->Position1ZEntry->GetValueAsDouble();
  activeRulerNode->SetPosition1(x, y, z);

  x = this->Position2XEntry->GetValueAsDouble();
  y = this->Position2YEntry->GetValueAsDouble();
  z = this->Position2ZEntry->GetValueAsDouble();
  activeRulerNode->SetPosition2(x, y, z);
      
  activeRulerNode->SetDistanceAnnotationFormat(this->DistanceAnnotationFormatEntry->GetWidget()->GetValue());
  
  double scale = this->DistanceAnnotationScaleEntry->GetWidget()->GetValueAsDouble();
  activeRulerNode->SetDistanceAnnotationScale(scale, scale, scale);
  
  activeRulerNode->SetResolution(this->ResolutionEntry->GetWidget()->GetValueAsInt());
                                 
  this->UpdatingMRML = 0;

}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateMRMLFromWidget(vtkMRMLMeasurementsRulerNode *rulerNode)
{
  if (this->UpdatingMRML || this->Updating3DWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;

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
    this->UpdatingMRML = 0;
    return;
    }

  if (this->DistanceWidget)
    {
    activeRulerNode->SetVisibility(this->DistanceWidget->GetEnabled());
    }
  if ( this->DistanceRepresentation)
    {
    double *p;
    p = this->DistanceRepresentation->GetPoint1WorldPosition();
    activeRulerNode->SetPosition1(p);
    p = this->DistanceRepresentation->GetPoint2WorldPosition();
    activeRulerNode->SetPosition2(p);

    double *rgb = this->DistanceRepresentation->GetPoint1Representation()->GetProperty()->GetColor();
    activeRulerNode->SetPointColour(rgb);
    rgb = this->DistanceRepresentation->GetLineProperty()->GetColor();
    activeRulerNode->SetLineColour(rgb);

    activeRulerNode->SetDistanceAnnotationVisibility(this->DistanceRepresentation->GetDistanceAnnotationVisibility());
    activeRulerNode->SetDistanceAnnotationFormat(this->DistanceRepresentation->GetDistanceAnnotationFormat());
    activeRulerNode->SetDistanceAnnotationScale(this->DistanceRepresentation->GetDistanceAnnotationScale());

    activeRulerNode->SetResolution(this->DistanceRepresentation->GetResolution());
    }

  // skip the models for now
  
  this->UpdatingMRML = 0;
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if ( !this->RulerNodeID )
    {
    return;
    }
  if (this->ProcessingMRMLEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing mrml event " << this->ProcessingMRMLEvent);
    return;
    }

  if (this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing widget event " << this->ProcessingWidgetEvent);
    return;
    }
  
  this->ProcessingMRMLEvent = event;
  
// first check to see if there was a fiducial list node deleted
  if (vtkMRMLScene::SafeDownCast(caller) != NULL &&
      vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene &&
      event == vtkMRMLScene::NodeRemovedEvent)
    {
    vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: got a node deleted event on scene");
    // check to see if it was the current node that was deleted
    if (callData != NULL)
      {
      vtkMRMLNode *delNode = (vtkMRMLNode *)callData;
      if (delNode != NULL &&
          delNode->GetID() == this->GetRulerNodeID())
        {
        vtkDebugMacro("My node got deleted " << this->GetRulerNodeID());
        }
      }
    }

  vtkMRMLMeasurementsRulerNode *node = vtkMRMLMeasurementsRulerNode::SafeDownCast(caller);
  vtkMRMLMeasurementsRulerNode *activeRulerNode = (vtkMRMLMeasurementsRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());
    
  // check for a node added event
  if (//vtkMRMLScene::SafeDownCast(caller) != NULL &&
      //vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene &&
      node != NULL &&
      event == vtkMRMLScene::NodeAddedEvent)
    {
    vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: got a node added event on scene");
    // check to see if it was a ruler node    
    if (callData != NULL)
      {
      //vtkMRMLNode *addNode = (vtkMRMLNode *)callData;
      vtkMRMLMeasurementsRulerNode *addNode = reinterpret_cast<vtkMRMLMeasurementsRulerNode*>(callData);
      if (addNode != NULL &&
          addNode->IsA("vtkMRMLMeasurementsRulerNode"))
        {
        vtkDebugMacro("Got a node added event on a ruler node " << addNode->GetID());
        // is it the currently active one?
        if (addNode == activeRulerNode)
          {
          UpdateWidget(activeRulerNode);
          this->ProcessingMRMLEvent = 0;
          return;
          }
        }
      }
    }
  if (node == activeRulerNode)
    {
    if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent)
      {
      vtkDebugMacro("Modified or node added or removed event on the fiducial list node.\n");
      if (node == NULL)
        {
        vtkDebugMacro("\tBUT: the node is null\n");
        // check to see if the id used to get the node is not null, if it's
        // a valid string, means that the node was deleted
        if (this->GetRulerNodeID() != NULL)
          {
          this->SetRulerNodeID(NULL);
          }
        this->ProcessingMRMLEvent = 0;
        return;
        }
      vtkDebugMacro("ProcessMRMLEvents: \t\tUpdating the GUI\n");
      // update the gui
      UpdateWidget(activeRulerNode);
      this->ProcessingMRMLEvent = 0;
      return;
      }
    /*
      else if ( event == an event that signals the end of moving a widget
      {
      if (node == NULL)
      {
      return;
      }
      vtkDebugMacro("ProcessMRMLEvents: setting the gui from the acitve fid list node");
      SetGUIFromList(activeRulerNode);
      return;
      }
    */
    } // end of events on the active ruler node
  
  if (node == vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected()) &&// vtkMRMLMeasurementsRulerNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    if (activeRulerNode !=  vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected()))
      {
      // select it first off
      this->SetRulerNodeID(vtkMRMLMeasurementsRulerNode::SafeDownCast(this->RulerSelectorWidget->GetSelected())->GetID());
      }
    this->UpdateWidget(activeRulerNode);
    }
  this->ProcessingMRMLEvent = 0;
}


//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::UpdateWidget(vtkMRMLMeasurementsRulerNode *activeRulerNode)
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    vtkDebugMacro("UpdateWidget: updating mrml = " << this->UpdatingMRML << ", updating widget = " <<  this->UpdatingWidget);
    return;
    }
  this->UpdatingWidget = 1;


  vtkDebugMacro("UpdateWidget: active ruler node is " << (activeRulerNode == NULL ? "null" : activeRulerNode->GetName()));
  
  // if the passed node is null, clear out the widget
  if (activeRulerNode == NULL)
    {
    // don't need to do anything yet, especially don't set the node selector to
    // null, as it causes a crash
    this->UpdatingWidget = 0;
    return;
    }
  
  if ( this->RulerSelectorWidget->GetSelected() == NULL )
    {
    vtkDebugMacro("Null selected fid list, selecting it");
    this->RulerSelectorWidget->SetSelected(activeRulerNode);
    this->UpdatingWidget = 0;
    return;
    }

  if (activeRulerNode && this->RulerSelectorWidget->GetSelected() &&
        strcmp(activeRulerNode->GetName(),
               this->RulerSelectorWidget->GetSelected()->GetName()) != 0)
      {
      vtkDebugMacro("UpdateWidget: input ruler " << activeRulerNode->GetName() << " doesn't match selector widget value: " << this->RulerSelectorWidget->GetSelected()->GetName());
      //this->RulerSelectorWidget->GetWidget()->GetWidget()->SetValue(activeRulerNode->GetName());
      this->RulerSelectorWidget->SetSelected(activeRulerNode);
      vtkDebugMacro("... returning, hoping for a invoke event");
      this->UpdatingWidget = 0;
      return;
      }

  vtkDebugMacro("UpdateWidget: updating the gui and 3d elements");
  // first update the GUI, then update the 3d elements
  // visibility
  this->VisibilityButton->GetWidget()->SetSelectedState(activeRulerNode->GetVisibility());
 
  // end point colour
  double *rgb = this->PointColourButton->GetColor();
  double *rgb1 = activeRulerNode->GetPointColour();
  if (fabs(rgb[0]-rgb1[0]) > 0.001 ||
      fabs(rgb[1]-rgb1[1]) > 0.001 ||
      fabs(rgb[2]-rgb1[2]) > 0.001)
    {
    this->PointColourButton->SetColor(activeRulerNode->GetPointColour());
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

  // end point position
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

  // distance annotation
  this->DistanceAnnotationVisibilityButton->GetWidget()->SetSelectedState(activeRulerNode->GetDistanceAnnotationVisibility());
  this->DistanceAnnotationFormatEntry->GetWidget()->SetValue(activeRulerNode->GetDistanceAnnotationFormat());
  double *scale = activeRulerNode->GetDistanceAnnotationScale();
  if (scale)
    {
    this->DistanceAnnotationScaleEntry->GetWidget()->SetValueAsDouble(scale[0]);
    }

  // resolution
  this->ResolutionEntry->GetWidget()->SetValueAsInt(activeRulerNode->GetResolution());

  this->UpdatingWidget = 0;

  this->Update3DWidget(activeRulerNode);
   
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
  if (this->DistanceWidget == NULL)
    {
    vtkDebugMacro("Update3D widget: distance widget is null");
    return;
    }
  if (this->DistanceRepresentation == NULL)
    {
    vtkDebugMacro("Update3D widget: distance representation is null");
    return;
    }
  this->Updating3DWidget = 1;

  vtkDebugMacro("Updating 3d widget from " << activeRulerNode->GetID());
  
  // visibility
  if ( activeRulerNode->GetVisibility() )
    {
    if (this->DistanceWidget->GetInteractor() == NULL &&
        this->GetViewerWidget())
      {
      this->DistanceWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
      double p1[3] = {-250.0, 50.0, 0.0};
      double p2[3] = {250.0,  50.0, 0.0};
      this->DistanceRepresentation->SetPoint1WorldPosition(p1);
      this->DistanceRepresentation->SetPoint2WorldPosition(p2);
      }
    vtkDebugMacro("UpdateWidget: distance widget on");
    this->DistanceWidget->On();
    }
  else
    {
    vtkDebugMacro("UpdateWidget: distance widget off");
    this->DistanceWidget->Off();
    }

  if (this->DistanceRepresentation)
    {
    // end point colour
    double *rgb1 = activeRulerNode->GetPointColour();
    this->DistanceRepresentation->GetPoint1Representation()->GetProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);
    this->DistanceRepresentation->GetPoint2Representation()->GetProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // line colour
    rgb1 = activeRulerNode->GetLineColour();
    this->DistanceRepresentation->GetLineProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // position
    double *p = activeRulerNode->GetPosition1();
    if (p)
      {
      this->DistanceRepresentation->SetPoint1WorldPosition(p);
      }
    p =  activeRulerNode->GetPosition2();
    if (p)
      {
      this->DistanceRepresentation->SetPoint2WorldPosition(p);
      }

    // distance annotation
    this->DistanceRepresentation->SetDistanceAnnotationVisibility(activeRulerNode->GetDistanceAnnotationVisibility());
    this->DistanceRepresentation->SetDistanceAnnotationFormat(activeRulerNode->GetDistanceAnnotationFormat());
    double *scale = activeRulerNode->GetDistanceAnnotationScale();
    if (scale)
      {
      this->DistanceRepresentation->SetDistanceAnnotationScale(scale);
      }
    // resolution
    this->DistanceRepresentation->SetResolution(activeRulerNode->GetResolution());
    }
  // first point constraint
  if (activeRulerNode->GetModelID1())
    {
    // get the model node
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID1()));
    // is it a valid model?
    if (model &&
        model->GetDisplayNode())
      {
      // is it already set to constrain the point placer?
      if (!this->RulerModel1PointPlacer->HasProp(vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()))))
        {
        this->RulerModel1PointPlacer->AddProp(vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID())));
        this->DistanceRepresentation->GetPoint1Representation()->ConstrainedOff();
        this->DistanceRepresentation->GetPoint1Representation()->SetPointPlacer(this->RulerModel1PointPlacer);
        // check if need to snap to it
        // TODO: figure out why not snapping
        double pos[3];
        this->DistanceRepresentation->GetPoint1WorldPosition(pos);
        if (!this->DistanceRepresentation->GetPoint1Representation()->GetPointPlacer()->ValidateWorldPosition(pos))
          {
          if (model->GetPolyData())
            {
            model->GetPolyData()->GetPoint(0, pos);
            vtkDebugMacro("Snapping point 1 to " << pos[0] << ", " << pos[1] << ", " << pos[2]);
            this->DistanceRepresentation->SetPoint1WorldPosition(pos);
            }
          }
        }
      }
    else
      {
      this->RulerModel1PointPlacer->RemoveAllProps();
//      this->DistanceHandleRepresentation->ConstrainedOn();
      this->DistanceRepresentation->GetPoint1Representation()->SetPointPlacer(NULL);
      }
    }
  else
    {
    // make sure it's not constrained
    this->RulerModel1PointPlacer->RemoveAllProps();
    this->DistanceRepresentation->GetPoint1Representation()->SetPointPlacer(NULL);
    }

  // second point constraint
  if (activeRulerNode->GetModelID2())
    {
    // get the model node
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeRulerNode->GetModelID2()));
    // is it a valid model?
    if (model &&
        model->GetDisplayNode())
      {
      // is it already set to constrain the point placer?
      if (!this->RulerModel2PointPlacer->HasProp(vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()))))
        {
        this->RulerModel2PointPlacer->AddProp(vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID())));
        this->DistanceRepresentation->GetPoint2Representation()->ConstrainedOff();
        this->DistanceRepresentation->GetPoint2Representation()->SetPointPlacer(this->RulerModel2PointPlacer);
        // check if need to snap to it
        // TODO: figure out why not snapping
        double pos[3];
        this->DistanceRepresentation->GetPoint2WorldPosition(pos);
        if (!this->DistanceRepresentation->GetPoint2Representation()->GetPointPlacer()->ValidateWorldPosition(pos))
          {
          if (model->GetPolyData())
            {
            model->GetPolyData()->GetPoint(0, pos);
            vtkDebugMacro("Snapping point 2 to " << pos[0] << ", " << pos[1] << ", " << pos[2]);
            this->DistanceRepresentation->SetPoint2WorldPosition(pos);
            }
          }
        }
      }
    else
      {
      this->RulerModel2PointPlacer->RemoveAllProps();
//      this->DistanceHandleRepresentation->ConstrainedOn();
      this->DistanceRepresentation->GetPoint2Representation()->SetPointPlacer(NULL);
      }
    }
  else
    {
    // make sure it's not constrained
    this->RulerModel2PointPlacer->RemoveAllProps();
    this->DistanceRepresentation->GetPoint2Representation()->SetPointPlacer(NULL);
    }

  // set up call back
  // temp: remove observers
  this->DistanceWidget->RemoveObservers(vtkCommand::InteractionEvent);
  this->DistanceWidget->RemoveObservers(vtkCommand::StartInteractionEvent);

  // now add call back
  vtkMeasurementsRulerWidgetCallback *myCallback = vtkMeasurementsRulerWidgetCallback::New();
//  std::string rulerID = std::string(activeRulerNode->GetID());
//  myCallback->RulerID = rulerID;
  myCallback->RulerNode = activeRulerNode;
//  myCallback->DistanceRepresentation = this->DistanceRepresentation;
  this->DistanceWidget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  this->DistanceWidget->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
  myCallback->Delete();

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
  // should I register my mrml node classes here?
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveMRMLObservers ( )
{
  if (this->GetRulerNodeID())
    {
    vtkMRMLMeasurementsRulerNode *rulerNode = vtkMRMLMeasurementsRulerNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetRulerNodeID()));
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(rulerNode, NULL, events);
    events->Delete();
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::AddWidgetObservers()
{
  if (this->VisibilityButton)
    {
    this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PointColourButton)
    {
    this->PointColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LineColourButton)
    {
    this->LineColourButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
    this->DistanceAnnotationVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  
  // observe the scene for node deleted events
  if (this->MRMLScene)
    {
    if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (this->MRMLScene->HasObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsRulerWidget::RemoveWidgetObservers ( ) {
  if (this->VisibilityButton)
    {
    this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PointColourButton)
    {
    this->PointColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LineColourButton)
    {
    this->LineColourButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
    this->DistanceAnnotationVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
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

  // ---
  // CHOOSE Ruler Node FRAME
  vtkKWFrame *pickRulerNodeFrame = vtkKWFrame::New ( );
  pickRulerNodeFrame->SetParent ( this->GetParent() );
  pickRulerNodeFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 pickRulerNodeFrame->GetWidgetName() );
  
   // a selector to pick a ruler
  this->RulerSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerSelectorWidget->SetParent ( pickRulerNodeFrame );
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

  
  this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->VisibilityButton->SetParent ( pickRulerNodeFrame );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Toggle Visibility");
  this->VisibilityButton->SetBalloonHelpString("set widget visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );

    // position 1 frame
  vtkKWFrame *position1Frame = vtkKWFrame::New();
  position1Frame->SetParent(pickRulerNodeFrame);
  position1Frame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 position1Frame->GetWidgetName() );
  
  this->Position1Label = vtkKWLabel::New();
  this->Position1Label->SetParent(position1Frame);
  this->Position1Label->Create();
  this->Position1Label->SetText("Postion 1");

  this->Position1XEntry = vtkKWEntry::New();
  this->Position1XEntry->SetParent(position1Frame);
  this->Position1XEntry->Create();
  this->Position1XEntry->SetWidth(8);
  this->Position1XEntry->SetRestrictValueToDouble();
  this->Position1XEntry->SetBalloonHelpString("First end of the line, X position");

  this->Position1YEntry = vtkKWEntry::New();
  this->Position1YEntry->SetParent(position1Frame);
  this->Position1YEntry->Create();
  this->Position1YEntry->SetWidth(8);
  this->Position1YEntry->SetRestrictValueToDouble();
  this->Position1YEntry->SetBalloonHelpString("First end of the line, Y position");

  this->Position1ZEntry = vtkKWEntry::New();
  this->Position1ZEntry->SetParent(position1Frame);
  this->Position1ZEntry->Create();
  this->Position1ZEntry->SetWidth(8);
  this->Position1ZEntry->SetRestrictValueToDouble();
  this->Position1ZEntry->SetBalloonHelpString("First end of the line, Z position");

  this->Script( "pack %s %s %s %s -side left -anchor nw -expand y -fill x -padx 2 -pady 2",
                  this->Position1Label->GetWidgetName(),
                  this->Position1XEntry->GetWidgetName(),
                  this->Position1YEntry->GetWidgetName(),
                  this->Position1ZEntry->GetWidgetName());

  // position 2 frame
  vtkKWFrame *position2Frame = vtkKWFrame::New();
  position2Frame->SetParent(pickRulerNodeFrame);
  position2Frame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 position2Frame->GetWidgetName() );

  this->Position2Label = vtkKWLabel::New();
  this->Position2Label->SetParent(position2Frame);
  this->Position2Label->Create();
  this->Position2Label->SetText("Postion 2");

  this->Position2XEntry = vtkKWEntry::New();
  this->Position2XEntry->SetParent(position2Frame);
  this->Position2XEntry->Create();
  this->Position2XEntry->SetWidth(8);
  this->Position2XEntry->SetRestrictValueToDouble();
  this->Position2XEntry->SetBalloonHelpString("First end of the line, X position");

  this->Position2YEntry = vtkKWEntry::New();
  this->Position2YEntry->SetParent(position2Frame);
  this->Position2YEntry->Create();
  this->Position2YEntry->SetWidth(8);
  this->Position2YEntry->SetRestrictValueToDouble();
  this->Position2YEntry->SetBalloonHelpString("First end of the line, Y position");

  this->Position2ZEntry = vtkKWEntry::New();
  this->Position2ZEntry->SetParent(position2Frame);
  this->Position2ZEntry->Create();
  this->Position2ZEntry->SetWidth(8);
  this->Position2ZEntry->SetRestrictValueToDouble();
  this->Position2ZEntry->SetBalloonHelpString("First end of the line, Z position");
  this->Script( "pack %s %s %s %s -side left -anchor nw -expand y -fill x -padx 2 -pady 2",
                  this->Position2Label->GetWidgetName(),
                  this->Position2XEntry->GetWidgetName(),
                  this->Position2YEntry->GetWidgetName(),
                  this->Position2ZEntry->GetWidgetName());
  
  //
  // Pick Models Frame
  //
  vtkKWFrameWithLabel *modelFrame = vtkKWFrameWithLabel::New();
  modelFrame->SetParent(this->GetParent());
  modelFrame->Create();
  modelFrame->SetLabelText("Constrain Ruler to Models");
  modelFrame->ExpandFrame();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", modelFrame->GetWidgetName());
  modelFrame->CollapseFrame();
  

  this->RulerModel1SelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerModel1SelectorWidget->SetParent ( modelFrame->GetFrame() );
  this->RulerModel1SelectorWidget->Create ( );
  this->RulerModel1SelectorWidget->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->RulerModel1SelectorWidget->SetChildClassesEnabled(0);
  this->RulerModel1SelectorWidget->NoneEnabledOn();
  this->RulerModel1SelectorWidget->SetShowHidden(1);
  this->RulerModel1SelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerModel1SelectorWidget->SetBorderWidth(2);
  this->RulerModel1SelectorWidget->SetPadX(2);
  this->RulerModel1SelectorWidget->SetPadY(2);
  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerModel1SelectorWidget->SetLabelText( "Select Ruler Model 1: ");
//  this->RulerModel1SelectorWidget->GetLabel()->SetForegroundColor(1, 0, 0);
  this->RulerModel1SelectorWidget->SetBalloonHelpString("Select a model on which to anchor the first end of the ruler.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->RulerModel1SelectorWidget->GetWidgetName());
  
  this->RulerModel2SelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerModel2SelectorWidget->SetParent ( modelFrame->GetFrame() );
  this->RulerModel2SelectorWidget->Create ( );
  this->RulerModel2SelectorWidget->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->RulerModel2SelectorWidget->SetChildClassesEnabled(0);
  this->RulerModel2SelectorWidget->NoneEnabledOn();
  this->RulerModel2SelectorWidget->SetShowHidden(1);
  this->RulerModel2SelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerModel2SelectorWidget->SetBorderWidth(2);
  this->RulerModel2SelectorWidget->SetPadX(2);
  this->RulerModel2SelectorWidget->SetPadY(2);
  this->RulerModel2SelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->RulerModel2SelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerModel2SelectorWidget->SetLabelText( "Select Ruler Model 2: ");
//  this->RulerModel2SelectorWidget->GetLabel()->SetForegroundColor(0, 0, 1);
  this->RulerModel2SelectorWidget->SetBalloonHelpString("Select a model on which to anchor the second end of the ruler.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->RulerModel2SelectorWidget->GetWidgetName());

  // ---
  // DISPLAY FRAME            
  vtkKWFrameWithLabel *rulerDisplayFrame = vtkKWFrameWithLabel::New ( );
  rulerDisplayFrame->SetParent ( this->GetParent() );
  rulerDisplayFrame->SetLabelText("Display Options");
  rulerDisplayFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 rulerDisplayFrame->GetWidgetName() );
  rulerDisplayFrame->CollapseFrame ( );

  this->PointColourButton = vtkKWChangeColorButton::New();
  this->PointColourButton->SetParent ( rulerDisplayFrame->GetFrame() );
  this->PointColourButton->Create ( );
  this->PointColourButton->SetColor(0.0, 0.0, 1.0);
  this->PointColourButton->LabelOutsideButtonOn();
  this->PointColourButton->SetLabelPositionToRight();
  this->PointColourButton->SetLabelText("Set End Point Color");
  this->PointColourButton->SetBalloonHelpString("set point color.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->PointColourButton->GetWidgetName() );

  this->LineColourButton = vtkKWChangeColorButton::New();
  this->LineColourButton->SetParent ( rulerDisplayFrame->GetFrame() );
  this->LineColourButton->Create ( );
  this->LineColourButton->SetColor(1.0, 1.0, 1.0);
  this->LineColourButton->LabelOutsideButtonOn();
  this->LineColourButton->SetLabelPositionToRight();
  this->LineColourButton->SetLabelText("Set Line Color");
  this->LineColourButton->SetBalloonHelpString("set line color.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->LineColourButton->GetWidgetName() );

  // distance annotation frame
  vtkKWFrame *annotationFrame = vtkKWFrame::New();
  annotationFrame->SetParent(rulerDisplayFrame->GetFrame());
  annotationFrame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 annotationFrame->GetWidgetName() );

  this->DistanceAnnotationVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->DistanceAnnotationVisibilityButton->SetParent ( annotationFrame );
  this->DistanceAnnotationVisibilityButton->Create ( );
  this->DistanceAnnotationVisibilityButton->SetLabelText("Toggle Distance Annotation Visibility");
  this->DistanceAnnotationVisibilityButton->SetBalloonHelpString("set distance annotation visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->DistanceAnnotationVisibilityButton->GetWidgetName() );
  
  this->DistanceAnnotationFormatEntry = vtkKWEntryWithLabel::New();
  this->DistanceAnnotationFormatEntry->SetParent(annotationFrame);
  this->DistanceAnnotationFormatEntry->Create();
  this->DistanceAnnotationFormatEntry->SetLabelText("Distance Annotation Format");
  this->DistanceAnnotationFormatEntry->SetBalloonHelpString("string formatting command, use %g to print out distance, plus any text you wish");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->DistanceAnnotationFormatEntry->GetWidgetName());

  this->DistanceAnnotationScaleEntry =  vtkKWEntryWithLabel::New();
  this->DistanceAnnotationScaleEntry->SetParent(annotationFrame);
  this->DistanceAnnotationScaleEntry->Create();
  this->DistanceAnnotationScaleEntry->SetLabelText("Distance Annotation Scale");
  this->DistanceAnnotationScaleEntry->GetWidget()->SetRestrictValueToDouble();
  this->DistanceAnnotationScaleEntry->SetBalloonHelpString("Scale value applied to the distance annotation text");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->DistanceAnnotationScaleEntry->GetWidgetName());

  this->ResolutionEntry = vtkKWEntryWithLabel::New();
  this->ResolutionEntry->SetParent(annotationFrame);
  this->ResolutionEntry->Create();
  this->ResolutionEntry->SetLabelText("Resolution");
  this->ResolutionEntry->SetBalloonHelpString(" number of subdivisions on the line");
  this->ResolutionEntry->GetWidget()->SetRestrictValueToInteger();
  // this is not used with the current line widget
  //this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
  //               this->ResolutionEntry->GetWidgetName());
  
  // add observers
  this->AddWidgetObservers();

  modelFrame->Delete();
  rulerDisplayFrame->Delete();
  position1Frame->Delete();
  position2Frame->Delete();
  pickRulerNodeFrame->Delete();
  annotationFrame->Delete();
  
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
  this->ViewerWidget = viewerWidget;
}
