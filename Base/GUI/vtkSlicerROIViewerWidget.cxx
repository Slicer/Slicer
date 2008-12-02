#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCollection.h"
#include "vtkTransform.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"

#include "vtkSlicerROIViewerWidget.h"

#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

#include "vtkMRMLROINode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"



// for pick events
#include "vtkSlicerViewerWidget.h"

class vtkBoxWidgetCallback : public vtkCommand
{
public:
  static vtkBoxWidgetCallback *New()
  { return new vtkBoxWidgetCallback; }
  virtual void Execute(vtkObject *caller, unsigned long event, void*)
  {
    // only update the position if it's an interaction event
    if (event == vtkCommand::InteractionEvent || event == vtkCommand::EndInteractionEvent)
      {
      vtkSlicerBoxWidget2 *boxWidget = reinterpret_cast<vtkSlicerBoxWidget2*>(caller);
      if (boxWidget && this->ROINode)
        {
        vtkSlicerBoxRepresentation* boxRep = reinterpret_cast<vtkSlicerBoxRepresentation*>(boxWidget->GetRepresentation());
        //boxRep->GetTransform(this->Transform);
        double* bounds = boxRep->GetBounds();
        this->ROINode->DisableModifiedEventOn();
        this->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),0.5*(bounds[3]+bounds[2]),0.5*(bounds[5]+bounds[4]));
        this->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),0.5*(bounds[3]-bounds[2]),0.5*(bounds[5]-bounds[4]));
        this->ROINode->DisableModifiedEventOff();
        
       //this->ROIViewerWidget->SetProcessingWidgetEvent(1);
        this->ROINode->InvokePendingModifiedEvent();
       //this->ROIViewerWidget->SetProcessingWidgetEvent(0);

        }
      }
  }
  vtkBoxWidgetCallback():ROINode(0), ROIViewerWidget(0) {}
  vtkMRMLROINode *ROINode;
  vtkSlicerROIViewerWidget *ROIViewerWidget;
  
};

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerROIViewerWidget );
vtkCxxRevisionMacro ( vtkSlicerROIViewerWidget, "$Revision: $");

//---------------------------------------------------------------------------
vtkSlicerROIViewerWidget::vtkSlicerROIViewerWidget ( )
{
  vtkDebugMacro("vtkSlicerROIViewerWidget::Constructor");
  
  this->MainViewerWidget = NULL;

  this->ProcessingMRMLEvent = 0;
  this->ProcessingWidgetEvent = 0;

}

//---------------------------------------------------------------------------
vtkSlicerROIViewerWidget::~vtkSlicerROIViewerWidget ( )
{
  this->SetMainViewerWidget(NULL);

  this->RemoveMRMLObservers();

  this->RemoveBoxWidgets();

}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::AddMRMLObservers ()
{
  
 // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosingEvent);

  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEvents(this->MRMLScene, events);
  events->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveMRMLObservers()
{
  this->RemoveMRMLROIObservers();
  
  this->SetAndObserveMRMLScene(NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::CreateWidget ( )
{
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  this->AddMRMLObservers();
  
}


//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{
  if (this->ProcessingMRMLEvent != 0 )
    {
    vtkDebugMacro("Returning because already processing an event, " << this->ProcessingMRMLEvent);
    return;
    }
  if (this->ProcessingWidgetEvent != 0 )
    {
    vtkDebugMacro("Returning because already processing an event, " << this->ProcessingMRMLEvent);
    return;
    }
    
  this->ProcessingMRMLEvent = event;

  vtkMRMLROINode *callerROINode = vtkMRMLROINode::SafeDownCast(caller);
  vtkMRMLROINode *callDataNode =  NULL;
  callDataNode = reinterpret_cast<vtkMRMLROINode *>(callData);
  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);


  if (callScene != NULL && event == vtkMRMLScene::SceneCloseEvent)
    {
    this->RemoveBoxWidgets();
    this->ProcessingMRMLEvent = 0;
    this->RequestRender();
    return;
    }

  // if get a node added event from the scene
  else if (callScene != NULL &&
           event == vtkMRMLScene::NodeAddedEvent &&
           callDataNode != NULL &&
           vtkMRMLROINode::SafeDownCast ( (vtkObjectBase *)callData ))
    {
      this->UpdateFromMRML();
      this->RequestRender();
    }
  else if (callScene != NULL &&
           event == vtkMRMLScene::NodeRemovedEvent &&
           callDataNode != NULL)
    {
    vtkMRMLROINode *roi = vtkMRMLROINode::SafeDownCast ( (vtkObjectBase *)callData );
    if ( roi )
      {
      this->RemoveBoxWidget(roi->GetID());
      this->RequestRender();
      }
    }
  else if (callerROINode != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateROIFromMRML(callerROINode);
    this->RequestRender();
    }
  
  // if the roi transfrom was updated...
  else if (event == vtkMRMLTransformableNode::TransformModifiedEvent &&
           (callerROINode != NULL))
    {
    this->UpdateROIFromMRML(callerROINode);
    this->RequestRender();
    }
  
  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RequestRender()
{
  this->MainViewerWidget->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::Render()
{
  this->MainViewerWidget->Render();
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::UpdateFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (scene == NULL)
    {
    vtkErrorMacro("...the scene is null... returning");
    return;
    }

  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLROINode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLROINode *roi = vtkMRMLROINode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLROINode"));
    this->UpdateROIFromMRML(roi);
    }

  //this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::UpdateROIFromMRML(vtkMRMLROINode *roi)
{
  if (roi == NULL)
    {
    vtkWarningMacro("UpdateROIFromMRML: null input list!");
    return;
    }
  
  this->AddMRMLROIObservers(roi);
  
  std::string roiID = std::string(roi->GetID());
  
  vtkSlicerBoxWidget2 *boxWidget = this->GetBoxWidgetByID(roi->GetID());
  
  vtkSlicerBoxRepresentation *rep = NULL;

  if (!boxWidget)
    {
    // create one
    boxWidget = vtkSlicerBoxWidget2::New();
    rep = vtkSlicerBoxRepresentation::New();
    boxWidget->SetRepresentation(rep);
    boxWidget->SetPriority(1);
    rep->SetPlaceFactor( 1.0 );
    
    vtkRenderWindowInteractor *interactor = this->MainViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
    boxWidget->SetInteractor(interactor);
    
    vtkBoxWidgetCallback *myCallback = vtkBoxWidgetCallback::New();
    
    myCallback->ROINode = roi;
    myCallback->ROIViewerWidget = this;
    boxWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    myCallback->Delete();
    
    this->DisplayedBoxWidgets[roiID] = boxWidget;
    }

  rep = vtkSlicerBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  // update widget from mrml
  float *xyz = roi->GetXYZ();
  float *rxyz = roi->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  
  // first get the list's transform node
  vtkMRMLTransformNode* tnode = roi->GetParentTransformNode();
  vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }
  /***
   // get this point
   float *xyz = roi->GetNthROIXYZ(f);
   // convert by the parent transform
   float xyzw[4];
   xyzw[0] = xyz[0];
   xyzw[1] = xyz[1];
   xyzw[2] = xyz[2];
   xyzw[3] = 1.0;
   float worldxyz[4], *worldp = &worldxyz[0];        
   transformToWorld->MultiplyPoint(xyzw, worldp);
  ***/
  
  transformToWorld->Delete();
  transformToWorld = NULL;
  
  if (rep) 
    {
    double b[6];
    b[0] = bounds[0];
    b[1] = bounds[3];
    b[2] = bounds[1];
    b[3] = bounds[4];
    b[4] = bounds[2];
    b[5] = bounds[5];
    rep->PlaceWidget(b);
    }

  if (roi->GetVisibility())
    {
    boxWidget->EnabledOn();
    }
  else
    {
    boxWidget->EnabledOff();
    }
  
  //this->Render();
}

//---------------------------------------------------------------------------
vtkSlicerBoxWidget2 *
vtkSlicerROIViewerWidget::GetBoxWidgetByID (const char *id)
{  
  if ( !id )      
    {
    return (NULL);
    }
  std::string sid = id;
  
  std::map< std::string, vtkSlicerBoxWidget2 *>::iterator iter;
  iter = this->DisplayedBoxWidgets.find(sid);
  if (iter != this->DisplayedBoxWidgets.end())
    {
    return (iter->second);
    }
  else
    {
    return (NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveBoxWidgets()
{
  std::map< std::string, vtkSlicerBoxWidget2 *>::iterator iter;
  for (iter = this->DisplayedBoxWidgets.begin();
       iter != this->DisplayedBoxWidgets.end();
       iter++)
    {
    if (iter->second != NULL)
      {
      vtkSlicerBoxWidget2* boxWidget = iter->second;
      this->RemoveBoxWidget(boxWidget);
      }
    }
  
  this->DisplayedBoxWidgets.clear();
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveBoxWidget(const char *roiID)
{
  if (roiID == NULL)
    {
    return;
    }
  
  vtkSlicerBoxWidget2* boxWidget = this->GetBoxWidgetByID(roiID);
  if  (boxWidget)
    {
    this->RemoveBoxWidget(boxWidget);
    this->DisplayedBoxWidgets.erase(std::string(roiID));
    }
  else
    {
    vtkWarningMacro("RemoveBoxWidget: couldn't find point widget");
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveBoxWidget(vtkSlicerBoxWidget2* boxWidget)
{
  vtkSlicerBoxRepresentation* rep = reinterpret_cast<vtkSlicerBoxRepresentation*>(boxWidget->GetRepresentation());
  
  boxWidget->RemoveAllObservers();
  
  boxWidget->SetRepresentation(NULL);
  boxWidget->EnabledOff();
  boxWidget->SetInteractor(NULL);
  boxWidget->Delete();
  boxWidget = NULL;
  if (rep) 
    {
    rep->Delete();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::AddMRMLROIObservers(vtkMRMLROINode *roi)
{
  if (roi == NULL)
    {
    return;
    }
  
  if (roi->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {
    roi->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
  if (roi->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {
    roi->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveMRMLROIObservers()
{
  if (this->GetMRMLScene() == NULL)
    {
    return;
    }
  // remove the observers on all the ROI nodes
  vtkMRMLROINode *roi;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLROINode");
  for (int n=0; n<nnodes; n++)
    {
    roi = vtkMRMLROINode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLROINode"));
    this->RemoveMRMLROINodeObservers(roi);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveMRMLROINodeObservers(vtkMRMLROINode *roi)
{
  if (roi == NULL)
    {
    vtkWarningMacro("RemoveROIObserversForList: null input list!");
    return;
    }
  
  if (roi->HasObserver (vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    roi->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
  if (roi->HasObserver( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    roi->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    }
}
