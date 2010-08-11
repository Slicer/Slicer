#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationROIManager.h"

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

//#include "vtkMeasurementsDistanceWidgetClass.h"

//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerApplication.h"
//#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLAnnotationROINode.h"
//#include "vtkSlicerBoxWidget2.h"
//#include "vtkSlicerBoxRepresentation.h"


class vtSlicerAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtSlicerAnnotationROIWidgetCallback *New()
  { return new vtSlicerAnnotationROIWidgetCallback; }
  virtual void Execute(vtkObject *caller, unsigned long event, void*)
  {/*
    vtkSlicerBoxWidget2 *widget = reinterpret_cast<vtkSlicerBoxWidget2*>(caller);
    ROIManager->UpdateROIMeasurement(ROINode, widget);

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

        this->ROIManager->SetProcessingWidgetEvent(1);
        this->ROINode->InvokePendingModifiedEvent();
        this->ROIManager->SetProcessingWidgetEvent(0);

      }
      else if (boxWidget && this->ROINode && 
        (!this->ROINode->GetInteractiveMode() && event == vtkCommand::InteractionEvent) )
      {
        this->ROIManager->Render();
      }
    }*/
  }
  vtSlicerAnnotationROIWidgetCallback():ROINode(0), ROIManager(0) {}
  vtkMRMLAnnotationROINode *ROINode;
  vtkSlicerAnnotationROIManager *ROIManager;

};


//---------------------------------------------------------------------------
//vtkStandardNewMacro (vtkSlicerAnnotationROIManager );
//vtkCxxRevisionMacro ( vtkSlicerAnnotationROIManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerAnnotationROIManager::vtkSlicerAnnotationROIManager ( )
{
  this->NodeID = NULL;
  this->Updating3DNode = 0;
  //this->ViewerWidget = NULL;

}

//---------------------------------------------------------------------------
vtkSlicerAnnotationROIManager::~vtkSlicerAnnotationROIManager ( )
{
  this->RemoveMRMLObservers();

  // gui elements

  // 3d widgets
  //this->SetMRMLScene ( NULL );
  
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::AddMRMLObservers ()
{

  // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);

  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  //this->SetAndObserveMRMLSceneEvents(this->MRMLScene, events);
  events->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RemoveMRMLObservers()
{
  this->RemoveMRMLROIObservers();

  //this->SetAndObserveMRMLScene(NULL);
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationROIManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    //this->vtkObject::PrintSelf ( os, indent );
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{

  this->ProcessingMRMLEvent = event;

  vtkMRMLAnnotationROINode *callerROINode = vtkMRMLAnnotationROINode::SafeDownCast(caller);
  vtkMRMLAnnotationROINode *callDataNode =  NULL;
  callDataNode = reinterpret_cast<vtkMRMLAnnotationROINode *>(callData);
  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

  if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
  {
    this->RemoveBoxWidgets();
    this->RequestRender();
    return;
  }

  // if get a node added event from the scene
  else if (callScene != NULL && event == vtkMRMLScene::NodeAddedEvent && callDataNode != NULL && vtkMRMLAnnotationROINode::SafeDownCast ( (vtkObjectBase *)callData ))
  {
    this->UpdateFromMRML();
    this->RequestRender();
  }
  else if (event == vtkMRMLAnnotationROINode::ROINodeAddedEvent)
  {
    this->UpdateFromMRML();
    this->Render();
  }
  else if (callScene != NULL && event == vtkMRMLScene::NodeRemovedEvent && callDataNode != NULL)
  {
    vtkMRMLAnnotationROINode *roi = vtkMRMLAnnotationROINode::SafeDownCast ( (vtkObjectBase *)callData );
    if ( roi )
    {
      this->RemoveBoxWidget(roi->GetID());
      this->RequestRender();
    }
  }
  else if (callerROINode != NULL && event == vtkCommand::ModifiedEvent)
  {
    this->UpdateROIFromMRML(callerROINode);
    this->RequestRender();
  }

  // if the roi transfrom was updated...
  else if (event == vtkMRMLTransformableNode::TransformModifiedEvent && (callerROINode != NULL))
  {
    this->UpdateROIFromMRML(callerROINode);
    this->RequestRender();
  }
  else if (callerROINode != NULL && event == vtkMRMLAnnotationNode::LockModifiedEvent)
  {
    this->UpdateLockUnlock(callerROINode);
  }

  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RequestRender()
{
  //this->GetViewerWidget()->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::Render()
{
  //this->GetViewerWidget()->Render();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RemoveBoxWidgets()
{
  /*std::map< std::string, vtkSlicerBoxWidget2 *>::iterator iter;
  for (iter = this->DisplayedBoxWidgets.begin(); iter != this->DisplayedBoxWidgets.end(); iter++)
  {
    if (iter->second != NULL)
    {
      vtkSlicerBoxWidget2* boxWidget = iter->second;
      this->RemoveBoxWidget(boxWidget);
    }
  }

  std::map< std::string, vtSlicerAnnotationROIWidgetCallback *>::iterator citer;
  for (citer = this->DisplayedBoxWidgetCallbacks.begin(); citer != this->DisplayedBoxWidgetCallbacks.end(); citer++)
  {
    if (citer->second != NULL)
    {
      vtSlicerAnnotationROIWidgetCallback* boxWidgetCallback = citer->second;
      boxWidgetCallback->Delete();
    }
  }

  this->DisplayedBoxWidgets.clear();
  this->DisplayedBoxWidgetCallbacks.clear();
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RemoveBoxWidget(const char *roiID)
{
  if (roiID == NULL)
  {
    return;
  }

  /*vtkSlicerBoxWidget2* boxWidget = this->GetBoxWidgetByID(roiID);
  if  (boxWidget)
  {
    this->RemoveBoxWidget(boxWidget);
    //this->DisplayedBoxWidgets.erase(std::string(roiID));
  }
  else
  {
    //vtkWarningMacro("RemoveBoxWidget: couldn't find point widget");
  }

  vtSlicerAnnotationROIWidgetCallback* boxWidgetCallback = this->GetBoxWidgetCallbackByID(roiID);
  if  (boxWidgetCallback)
  {
    boxWidgetCallback->Delete();
    //this->DisplayedBoxWidgetCallbacks.erase(std::string(roiID));
  }*/

}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationROIManager::RemoveBoxWidget(vtkSlicerBoxWidget2* boxWidget)
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
}*/

//---------------------------------------------------------------------------
/*vtkSlicerBoxWidget2* vtkSlicerAnnotationROIManager::GetBoxWidgetByID (const char *id)
{  
  if ( !id )      
  {
    return (NULL);
  }
  std::string sid = id;

  //std::map< std::string, vtkSlicerBoxWidget2 *>::iterator iter;
  //iter = this->DisplayedBoxWidgets.find(sid);
  if (iter != this->DisplayedBoxWidgets.end())
  {
    return (iter->second);
  }
  else
  {
    return (NULL);
  } return NULL;
}*/

//---------------------------------------------------------------------------
vtSlicerAnnotationROIWidgetCallback* vtkSlicerAnnotationROIManager::GetBoxWidgetCallbackByID (const char *id)
{  
  if ( !id )      
  {
    return (NULL);
  }
  std::string sid = id;

  std::map< std::string, vtSlicerAnnotationROIWidgetCallback*>::iterator iter;
  //iter = this->DisplayedBoxWidgetCallbacks.find(sid);
  /*if (iter != this->DisplayedBoxWidgetCallbacks.end())
  {
    return (iter->second);
  }
  else
  {
    return (NULL);
  }*/ return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::UpdateFromMRML()
{
  /*vtkMRMLScene *scene = this->GetMRMLScene();
  if (scene == NULL)
  {
    //vtkErrorMacro("...the scene is null... returning");
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

  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationROINode");
  for (int n=0; n<nnodes; n++)
  {
    vtkMRMLAnnotationROINode *roi = vtkMRMLAnnotationROINode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLAnnotationROINode"));
    this->UpdateROIFromMRML(roi);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::UpdateROIFromMRML(vtkMRMLAnnotationROINode *roi)
{/*
  if (roi == NULL)
  {
    //vtkWarningMacro("UpdateROIFromMRML: null input list!");
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

    if (this->ViewerWidget && this->ViewerWidget->GetMainViewer() )
    {
      vtkRenderWindowInteractor *interactor = this->ViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
      boxWidget->SetInteractor(interactor);
    }
    vtSlicerAnnotationROIWidgetCallback *myCallback = vtSlicerAnnotationROIWidgetCallback::New();

    myCallback->ROINode = roi;
    myCallback->ROIManager = this;
    boxWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::EndInteractionEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    myCallback->Delete();

    //this->DisplayedBoxWidgetCallbacks[roiID] = myCallback;
    this->DisplayedBoxWidgets[roiID] = boxWidget;

    //if (roi->HasObserver(vtkMRMLAnnotationROINode::ROINodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    //{
    //  roi->AddObserver(vtkMRMLAnnotationROINode::ROINodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    //}

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
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::UpdateROITransform(vtkMRMLAnnotationROINode *roi)
{/*
  if (roi == NULL)
  {
    //vtkWarningMacro("UpdateROIFromMRML: null input list!");
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

    if (this->ViewerWidget && this->ViewerWidget->GetMainViewer() )
    {
      //vtkRenderWindowInteractor *interactor = this->ViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
      //boxWidget->SetInteractor(interactor);
    }
    vtSlicerAnnotationROIWidgetCallback *myCallback = vtSlicerAnnotationROIWidgetCallback::New();

    myCallback->ROINode = roi;
    myCallback->ROIManager = this;
    boxWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::EndInteractionEvent, myCallback);
    boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    myCallback->Delete();

    //this->DisplayedBoxWidgetCallbacks[roiID] = myCallback;    
    //this->DisplayedBoxWidgets[roiID] = boxWidget;
  }

  rep = vtkSlicerBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  // get the ROI's transform node
  vtkMRMLTransformNode* tnode = roi->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
  {
    //vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    //transformToWorld->Identity();
    //vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    //lnode->GetMatrixTransformToWorld(transformToWorld);

    //vtkSmartPointer<vtkTransform> xform =  vtkSmartPointer<vtkTransform>::New();
    //xform->Identity();
    //xform->SetMatrix(transformToWorld);
    //rep->SetTransform(xform);

    //boxWidget->InvokeEvent(vtkCommand::EndInteractionEvent);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::AddMRMLROIObservers(vtkMRMLAnnotationROINode *roi)
{/*
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
  if (roi->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    roi->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RemoveMRMLROIObservers()
{
  /*if (this->GetMRMLScene() == NULL)
  {
    return;
  }
  // remove the observers on all the ROI nodes
  vtkMRMLAnnotationROINode *roi;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationROINode");
  for (int n=0; n<nnodes; n++)
  {
    roi = vtkMRMLAnnotationROINode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLAnnotationROINode"));
    this->RemoveMRMLROINodeObservers(roi);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::RemoveMRMLROINodeObservers(vtkMRMLAnnotationROINode *roi)
{/*
  if (roi == NULL)
  {
    //vtkWarningMacro("RemoveROIObserversForList: null input list!");
    return;
  }

  if (roi->HasObserver (vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 1)
  {
    roi->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  }
  if (roi->HasObserver( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 1)
  {
    roi->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
  }*/
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationROIManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  if (this->ViewerWidget != NULL)
  {
    // TODO: figure out if this is necessary
    this->RemoveBoxWidgets();
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
  this->UpdateROIWidgetInteractors();
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::UpdateROIWidgetInteractors()
{/*
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
    this->GetViewerWidget()->GetMainViewer() == NULL ||
    this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
  {
    vtkDebugMacro("UpdateRulerWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
  }

  std::map<std::string, vtkSlicerBoxWidget2 *>::iterator iter;
  for (iter = this->DisplayedBoxWidgets.begin(); iter !=  this->DisplayedBoxWidgets.end(); iter++)
  {
    if (iter->second)
    {
      if (isNull)
      {
        iter->second->SetInteractor(NULL);
      }
      else
      {
        iter->second->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
      }
      // now update the visibility for the ruler
      vtkMRMLAnnotationROINode *roiNode = NULL;
      if (this->GetMRMLScene())
      {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
        {
          roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);
        }
      }
      if (roiNode != NULL)
      {
        //this->Update3DWidgetVisibility(roiNode);
      }
    }
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationROIManager::UpdateLockUnlock(vtkMRMLAnnotationROINode* roiNode)
{
  if (roiNode == NULL)
  {
    return;
  }

  /*vtkSlicerBoxWidget2 *widget = this->GetBoxWidgetByID(roiNode->GetID());
  if (!widget)
  {
    cout << "No distance widget found, adding a distance widget for this one" << endl;
    return;
  }

  if ( roiNode->GetLocked() )
  {
    widget->ProcessEventsOff();
  } 
  else
  {
    widget->ProcessEventsOn();
  }*/
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationROIManager::UpdateROIMeasurement(vtkMRMLAnnotationROINode* node, vtkSlicerBoxWidget2* widget)
{
  //vtkSplineWidget* widget = this->GetSplineWidget(node->GetID());
  if ( widget==NULL)
  {
    return;
  }  

  double exts[3];
  //vtkSlicerBoxRepresentation* rep = vtkSlicerBoxRepresentation::SafeDownCast(widget->GetRepresentation());
  //rep->GetExtents(exts);
  
  //node->SetROIMeasurement(exts[0], exts[1], exts[2]);
}*/
