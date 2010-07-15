#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCollection.h"
#include "vtkTransform.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"

#include "vtkSlicerROIViewerWidget.h"

#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

#include "vtkMRMLROINode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

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
      if (boxWidget && this->ROINode && 
          (this->ROINode->GetInteractiveMode() || 
           (!this->ROINode->GetInteractiveMode() && event == vtkCommand::EndInteractionEvent)) )
        {
        vtkSlicerBoxRepresentation* boxRep = reinterpret_cast<vtkSlicerBoxRepresentation*>(boxWidget->GetRepresentation());
        double extents[3];
        double center[3];
        boxRep->GetExtents(extents);
        boxRep->GetCenter(center);

        vtkMRMLTransformNode* tnode = this->ROINode->GetParentTransformNode();
        if (tnode != NULL && tnode->IsLinear())
          {
          vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
          transformToWorld->Identity();
          vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
          lnode->GetMatrixTransformToWorld(transformToWorld);
          transformToWorld->Invert();
          
          double p[4];
          p[3] = 1;
          int i;
          for (i=0; i<3; i++)
            {
            p[i] = center[i];
            }
          double *xyz = transformToWorld->MultiplyDoublePoint(p);
          for (i=0; i<3; i++)
            {
            center[i] = xyz[i];
            }
          }

        this->ROINode->DisableModifiedEventOn();
        this->ROINode->SetXYZ(center[0], center[1], center[2] );
        this->ROINode->SetRadiusXYZ(0.5*extents[0], 0.5*extents[1], 0.5*extents[2] );
        this->ROINode->DisableModifiedEventOff();
        
        this->ROIViewerWidget->SetProcessingWidgetEvent(1);
        this->ROINode->InvokePendingModifiedEvent();
        this->ROIViewerWidget->SetProcessingWidgetEvent(0);

        }
      else if (boxWidget && this->ROINode && 
           (!this->ROINode->GetInteractiveMode() && event == vtkCommand::InteractionEvent) )
        {
        this->ROIViewerWidget->Render();
        }
      }
  }
  vtkBoxWidgetCallback():ROINode(0), ROIViewerWidget(0) {}
  vtkMRMLROINode *ROINode;
  vtkSlicerROIViewerWidget *ROIViewerWidget;
  
};

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerROIViewerWidget );
vtkCxxRevisionMacro ( vtkSlicerROIViewerWidget, "$Revision$");

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
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);

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


  if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
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
  if (this->MainViewerWidget)
    {
    this->MainViewerWidget->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::Render()
{
  if (this->MainViewerWidget)
    {
    this->MainViewerWidget->RequestRender(); /* let the viewer manage actual render time*/
    }
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

  // remove not used boxes
  std::map<std::string, vtkSlicerBoxWidget2*>::iterator iter;
  std::map<std::string, vtkSlicerBoxWidget2*> temp;
  for (iter = this->DisplayedBoxWidgets.begin(); iter != this->DisplayedBoxWidgets.end(); iter++)
    {
    if (scene->GetNodeByID(iter->first.c_str()) != NULL)
      {
      temp[iter->first] = iter->second;
      }
    else
      {
      this->RemoveBoxWidget(iter->second);
      }
    }
  this->DisplayedBoxWidgets = temp;

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
    rep->GetSelectedHandleProperty()->SetColor(0.2,0.6,0.15);

    if (this->MainViewerWidget && this->MainViewerWidget->GetMainViewer() )
      {
      vtkRenderWindowInteractor *interactor = this->MainViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
      boxWidget->SetInteractor(interactor);
      }
    vtkBoxWidgetCallback *myCallback = vtkBoxWidgetCallback::New();
    
    myCallback->ROINode = roi;
    myCallback->ROIViewerWidget = this;
    boxWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::EndInteractionEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    myCallback->Delete();
    
    //this->DisplayedBoxWidgetCallbacks[roiID] = myCallback;
    this->DisplayedBoxWidgets[roiID] = boxWidget;
    }

  rep = vtkSlicerBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  // update widget from mrml
  double *xyz = roi->GetXYZ();
  double *rxyz = roi->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }

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

  this->UpdateROITransform(roi);

}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::UpdateROITransform(vtkMRMLROINode *roi)
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
    rep->GetSelectedHandleProperty()->SetColor(0.2,0.6,0.15);

    if (this->MainViewerWidget && this->MainViewerWidget->GetMainViewer() )
      {
      vtkRenderWindowInteractor *interactor = this->MainViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
      boxWidget->SetInteractor(interactor);
      }
    vtkBoxWidgetCallback *myCallback = vtkBoxWidgetCallback::New();
    
    myCallback->ROINode = roi;
    myCallback->ROIViewerWidget = this;
    boxWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::EndInteractionEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    myCallback->Delete();

    //this->DisplayedBoxWidgetCallbacks[roiID] = myCallback;    
    this->DisplayedBoxWidgets[roiID] = boxWidget;
    }

  rep = vtkSlicerBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  // get the ROI's transform node
  vtkMRMLTransformNode* tnode = roi->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    transformToWorld->Identity();
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);

    vtkSmartPointer<vtkTransform> xform =  vtkSmartPointer<vtkTransform>::New();
    xform->Identity();
    xform->SetMatrix(transformToWorld);
    rep->SetTransform(xform);

    boxWidget->InvokeEvent(vtkCommand::EndInteractionEvent);
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
vtkBoxWidgetCallback *
vtkSlicerROIViewerWidget::GetBoxWidgetCallbackByID (const char *id)
{  
  if ( !id )      
    {
    return (NULL);
    }
  std::string sid = id;
  
  std::map< std::string, vtkBoxWidgetCallback *>::iterator iter;
  iter = this->DisplayedBoxWidgetCallbacks.find(sid);
  if (iter != this->DisplayedBoxWidgetCallbacks.end())
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

  std::map< std::string, vtkBoxWidgetCallback *>::iterator citer;
  for (citer = this->DisplayedBoxWidgetCallbacks.begin();
       citer != this->DisplayedBoxWidgetCallbacks.end();
       citer++)
    {
    if (citer->second != NULL)
      {
      vtkBoxWidgetCallback* boxWidgetCallback = citer->second;
      boxWidgetCallback->Delete();
      }
    }
  
  this->DisplayedBoxWidgets.clear();
  this->DisplayedBoxWidgetCallbacks.clear();

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

  vtkBoxWidgetCallback* boxWidgetCallback = this->GetBoxWidgetCallbackByID(roiID);
  if  (boxWidgetCallback)
    {
    boxWidgetCallback->Delete();
    this->DisplayedBoxWidgetCallbacks.erase(std::string(roiID));
    }

}

//---------------------------------------------------------------------------
void vtkSlicerROIViewerWidget::RemoveBoxWidget(vtkSlicerBoxWidget2* boxWidget)
{
  vtkSlicerBoxRepresentation* rep = reinterpret_cast<vtkSlicerBoxRepresentation*>(boxWidget->GetRepresentation());
  
  vtkEventBroker::GetInstance()->RemoveObservationsForSubjectByTag( boxWidget, 0 );
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
