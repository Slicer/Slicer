#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationBidimensionalManager.h"

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkBiDimensionalRepresentation2D.h"
#include "vtkBiDimensionalWidget.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerAnnotationBidimensionalManager );
vtkCxxRevisionMacro ( vtkSlicerAnnotationBidimensionalManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerAnnotationBidimensionalManager::vtkSlicerAnnotationBidimensionalManager ( )
{
  this->NodeID = NULL;
  this->ViewerWidget = NULL;
  this->Updating3DWidget = 0;
}

//---------------------------------------------------------------------------
vtkSlicerAnnotationBidimensionalManager::~vtkSlicerAnnotationBidimensionalManager ( )
{
  this->RemoveMRMLObservers();
 
  // 3d widgets
  this->SetMRMLScene ( NULL );
  
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  this->ProcessingMRMLEvent = event;

  vtkMRMLAnnotationBidimensionalNode *callerNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(caller);
  vtkMRMLAnnotationBidimensionalNode *callDataNode =  NULL;
  callDataNode = reinterpret_cast<vtkMRMLAnnotationBidimensionalNode *>(callData);
  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

    // the scene was closed, don't get node removed events so clear up here
    if (callScene != NULL && event == vtkMRMLScene::SceneCloseEvent)
    {
        vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
        // the lists are already gone from the scene, so need to clear out all the
        // widget properties, can't call remove with a node
        this->RemoveBidimensionalWidgets();
        return;
    }
      // first check to see if there was a ruler list node deleted
    else if (callScene != NULL && callScene == this->MRMLScene && event == vtkMRMLScene::NodeRemovedEvent)
    {
        vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
        // check to see if it was the current node that was deleted
        if (callData != NULL)
        {
            vtkMRMLAnnotationBidimensionalNode *delNode = reinterpret_cast<vtkMRMLAnnotationBidimensionalNode*>(callData);
            if (delNode != NULL && delNode->IsA("vtkMRMLAnnotationBidimensionalNode"))
            {
                vtkDebugMacro("A ruler node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
                this->RemoveBidimensionalWidget(delNode);
            }
        }
    }
  else if (callScene != NULL && event == vtkMRMLScene::NodeAddedEvent && callDataNode != NULL && vtkMRMLAnnotationBidimensionalNode::SafeDownCast ( (vtkObjectBase *)callData ))
  {
    this->Update3DWidget(callDataNode);
    this->RequestRender();
  }
  else if (callerNode != NULL && event == vtkMRMLAnnotationNode::LockModifiedEvent)
  {
    this->UpdateLockUnlock(callerNode);
  }
  else if (callerNode != NULL && event == vtkCommand::ModifiedEvent)
  {
    this->UpdateVisibility(callerNode);
    this->RequestRender();
  }

}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::Update3DWidget(vtkMRMLAnnotationBidimensionalNode *activeNode)
{
    if (activeNode == NULL)
    {
        vtkDebugMacro("Update3DWidget: passed in node is null, returning");
        return;
    }

    if (this->Updating3DWidget)
    {
        vtkDebugMacro("Already updating 3d widget");
        return;
    }

  vtkBiDimensionalWidget *widget = this->GetBidimensionalWidget(activeNode->GetID());
  if (!widget)
  {
    cout << "No widget found, adding a widget for this one" << endl;
    this->AddBidimensionalWidget(activeNode);
    widget = this->GetBidimensionalWidget(activeNode->GetID());
    if (!widget)
    {
      vtkErrorMacro("Error adding a new widget for node " << activeNode->GetID());
      this->Updating3DWidget = 0;
      return;
    }
  }

    this->Updating3DWidget = 1;

  vtkBiDimensionalRepresentation2D* bidRep = vtkBiDimensionalRepresentation2D::New();
  widget->SetRepresentation(bidRep);
  widget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
  widget->On();

  if (activeNode->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    activeNode->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  }
  if (activeNode->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    activeNode->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
  }
  if (activeNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    activeNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }

  this->Updating3DWidget = 0; 
}


//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::AddMRMLObservers ( )
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
        if (this->MRMLScene->HasObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
        {
            this->MRMLScene->AddObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::RemoveMRMLObservers ( )
{
    // remove observers on the ruler nodes
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationBidimensionalNode");
    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationBidimensionalNode *node = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationBidimensionalNode"));
        if (node->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            node->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
        if (node->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            node->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
        node = NULL;
    }
    //events->Delete();

    if (this->MRMLScene)
    {
        vtkDebugMacro("RemoveMRMLObservers: stopping watching for node removed, added, scene close events on the scene");
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::AddBidimensionalWidget(vtkMRMLAnnotationBidimensionalNode *node)
{
    if (!node)
    {
        return;
    }
    if (this->GetBidimensionalWidget(node->GetID()) != NULL)
    {
        vtkDebugMacro("Already have widgets for node " << node->GetID());
        return;
    }

  vtkBiDimensionalWidget* c = vtkBiDimensionalWidget::New();

    this->BidimensionalWidgets[node->GetID()] = c;
}

//---------------------------------------------------------------------------
vtkBiDimensionalWidget* vtkSlicerAnnotationBidimensionalManager::GetBidimensionalWidget(const char *nodeID)
{
    std::map<std::string, vtkBiDimensionalWidget *>::iterator iter;
    for (iter = this->BidimensionalWidgets.begin(); iter != this->BidimensionalWidgets.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::RemoveBidimensionalWidget(vtkMRMLAnnotationBidimensionalNode *node)
{
    if (!node)
    {
        return;
    }
    if (this->GetBidimensionalWidget(node->GetID()) != NULL)
    {
        this->BidimensionalWidgets[node->GetID()]->Delete();
        this->BidimensionalWidgets.erase(node->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::RemoveBidimensionalWidgets()
{
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationBidimensionalNode");
    vtkDebugMacro("RemoveBidimensionalWidgets: have " << nnodes << " ruler  nodes in the scene, " << this->BidimensionalWidgets.size() << " widgets defined already");

    if (nnodes == 0)
    {
        // the scene was closed, all the nodes are gone, so do this w/o reference
        // to the nodes
        vtkDebugMacro("RemoveDistanceWidgets: no ruler nodes in scene, removing ruler widgets w/o reference to nodes");
        std::map<std::string, vtkBiDimensionalWidget *>::iterator iter;
        for (iter = this->BidimensionalWidgets.end(); iter != this->BidimensionalWidgets.end(); iter++)
        {
            vtkDebugMacro("RemoveBidimensionalWidgets: deleting and erasing " << iter->first);
            iter->second->Delete();
            this->BidimensionalWidgets.erase(iter);
        }
        this->BidimensionalWidgets.clear();
    }
    else
    {
        for (int n=0; n<nnodes; n++)
        {
            vtkMRMLAnnotationBidimensionalNode *node = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationBidimensionalNode"));
            if (node)
            {
                this->RemoveBidimensionalWidget(node);
            }
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::RequestRender()
{
  this->GetViewerWidget()->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::Render()
{
  this->GetViewerWidget()->Render();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  if (this->ViewerWidget != NULL)
  {
    // TODO: figure out if this is necessary
    this->RemoveBidimensionalWidgets();
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
  this->UpdateWidgetInteractors();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::UpdateWidgetInteractors()
{
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
    this->GetViewerWidget()->GetMainViewer() == NULL ||
    this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
  {
    vtkDebugMacro("UpdateSplineWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
  }

  std::map<std::string, vtkBiDimensionalWidget *>::iterator iter;
  for (iter = this->BidimensionalWidgets.begin(); iter !=  this->BidimensionalWidgets.end(); iter++)
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
      vtkMRMLAnnotationBidimensionalNode *splineNode = NULL;
      if (this->GetMRMLScene())
      {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
        {
          splineNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);
        }
      }
      if (splineNode != NULL)
      {
        //this->Update3DWidgetVisibility(roiNode);
      }
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationBidimensionalManager::UpdateLockUnlock(vtkMRMLAnnotationBidimensionalNode* node)
{
  if (node == NULL)
  {
    return;
  }

  vtkBiDimensionalWidget *widget = this->GetBidimensionalWidget(node->GetID());
  if (!widget)
  {
    cout << "No distance widget found, adding a distance widget for this one" << endl;
    return;
  }

  if ( node->GetLocked() )
  {
    widget->ProcessEventsOff();
  } 
  else
  {
    widget->ProcessEventsOn();
  }
}

void vtkSlicerAnnotationBidimensionalManager::UpdateVisibility(vtkMRMLAnnotationBidimensionalNode *activeNode)
{
  vtkBiDimensionalWidget *widget = this->GetBidimensionalWidget(activeNode->GetID());
  if ( widget==NULL)
  {
    return;
  }  

  if (activeNode->GetVisible())
  {
    widget->EnabledOn();
  }
  else
  {
    widget->EnabledOff();
  }
}
