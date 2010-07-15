#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationSplineManager.h"

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerApplication.h"
//#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkSplineRepresentation.h"
#include "vtkSplineWidget.h"
#include "vtkMRMLAnnotationSplineNode.h"

class vtkSlicerAnnotationSplineManagerCallback : public vtkCommand
{
public:
  static vtkSlicerAnnotationSplineManagerCallback *New()
  { return new vtkSlicerAnnotationSplineManagerCallback; }
  virtual void Execute(vtkObject *caller, unsigned long event, void* callData)
  {
    vtkSplineWidget *widget = reinterpret_cast<vtkSplineWidget*>(caller);
    widget->GetPolyData(Poly);
    ManagerPointer->UpdateSplineMeasurement(SplineNode, widget);
  if (event == vtkCommand::InteractionEvent)
  {
    if (widget != NULL)
    {
      double p1[3], p2[3], p3[3], p4[3], p5[3];
      widget->GetHandlePosition(0, p1);
      widget->GetHandlePosition(1, p2);
      widget->GetHandlePosition(2, p3);
      widget->GetHandlePosition(3, p4);
      widget->GetHandlePosition(4, p5);

      if (this->SplineNode)
      {
        // does the angle node have a transform?
        vtkMRMLTransformNode* tnode = this->SplineNode->GetParentTransformNode();
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
        this->SplineNode->SetControlPoint(worldxyz, 0);
        // second point
        xyzw[0] = p2[0];
        xyzw[1] = p2[1];
        xyzw[2] = p2[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->SplineNode->SetControlPoint(worldxyz, 1);
        // 3rd point
        xyzw[0] = p3[0];
        xyzw[1] = p3[1];
        xyzw[2] = p3[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->SplineNode->SetControlPoint(worldxyz, 2);
        // 4th point
        xyzw[0] = p4[0];
        xyzw[1] = p4[1];
        xyzw[2] = p4[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->SplineNode->SetControlPoint(worldxyz, 3);
        // 5th point
        xyzw[0] = p5[0];
        xyzw[1] = p5[1];
        xyzw[2] = p5[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->SplineNode->SetControlPoint(worldxyz, 3);

        transformToWorld->Delete();
        transformToWorld = NULL;
        tnode = NULL;
      }
    }
  }

  }
  vtkSlicerAnnotationSplineManagerCallback():Poly(0){};
  vtkPolyData* Poly;
  vtkMRMLAnnotationSplineNode* SplineNode;
  vtkSlicerAnnotationSplineManager* ManagerPointer;
};


//---------------------------------------------------------------------------
//vtkStandardNewMacro (vtkSlicerAnnotationSplineManager );
//vtkCxxRevisionMacro ( vtkSlicerAnnotationSplineManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerAnnotationSplineManager::vtkSlicerAnnotationSplineManager ( )
{
  this->NodeID = NULL;
  //this->ViewerWidget = NULL;
  this->Updating3DWidget = 0;
}

//---------------------------------------------------------------------------
vtkSlicerAnnotationSplineManager::~vtkSlicerAnnotationSplineManager ( )
{
  this->RemoveMRMLObservers();
 
  // 3d widgets
  //this->SetMRMLScene ( NULL );
  
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationSplineManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{/*
  this->ProcessingMRMLEvent = event;

  vtkMRMLAnnotationSplineNode *callerNode = vtkMRMLAnnotationSplineNode::SafeDownCast(caller);
  vtkMRMLAnnotationSplineNode *callDataNode =  NULL;
  callDataNode = reinterpret_cast<vtkMRMLAnnotationSplineNode *>(callData);
  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

    // the scene was closed, don't get node removed events so clear up here
    if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
        // the lists are already gone from the scene, so need to clear out all the
        // widget properties, can't call remove with a node
        this->RemoveSplineWidgets();
        return;
    }
      // first check to see if there was a ruler list node deleted
    else if (callScene != NULL && callScene == this->MRMLScene && event == vtkMRMLScene::NodeRemovedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
        // check to see if it was the current node that was deleted
        if (callData != NULL)
        {
            vtkMRMLAnnotationSplineNode *delNode = reinterpret_cast<vtkMRMLAnnotationSplineNode*>(callData);
            if (delNode != NULL && delNode->IsA("vtkMRMLAnnotationSplineNode"))
            {
                //vtkDebugMacro("A ruler node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
                this->RemoveSplineWidget(delNode);
            }
        }
    }
  else if (callScene != NULL && event == vtkMRMLScene::NodeAddedEvent && callDataNode != NULL && vtkMRMLAnnotationSplineNode::SafeDownCast ( (vtkObjectBase *)callData ))
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
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::Update3DWidget(vtkMRMLAnnotationSplineNode *activeNode)
{/*
    if (activeNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in node is null, returning");
        return;
    }

    if (this->Updating3DWidget)
    {
        //vtkDebugMacro("Already updating 3d widget");
        return;
    }

  vtkSplineWidget *widget = this->GetSplineWidget(activeNode->GetID());
  if (!widget)
  {
    cout << "No widget found, adding a widget for this one" << endl;
    this->AddSplineWidget(activeNode);
    widget = this->GetSplineWidget(activeNode->GetID());
    if (!widget)
    {
      //vtkErrorMacro("Error adding a new widget for node " << activeNode->GetID());
      this->Updating3DWidget = 0;
      return;
    }
  }

  vtkPolyData* poly = vtkPolyData::New();
  widget->GetPolyData(poly);

  vtkSlicerAnnotationSplineManagerCallback* mycallback = vtkSlicerAnnotationSplineManagerCallback::New();
  mycallback->Poly = poly;
  mycallback->SplineNode = activeNode;
  widget->AddObserver(vtkCommand::InteractionEvent, mycallback);


    this->Updating3DWidget = 1;

  //vtkSplineRepresentation* sRep = vtkSplineRepresentation::New();

  widget->PlaceWidget();
  //widget->ProjectToPlaneOn();
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
  if (activeNode->HasObserver ( vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    activeNode->AddObserver ( vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this->MRMLCallbackCommand );
  }
  if (activeNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    activeNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }

  this->Updating3DWidget = 0; 
*/}


//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::AddMRMLObservers ( )
{/*
  // the widget as a whole needs to keep track of ruler nodes in the scene
    if (this->MRMLScene)
    {
        //vtkDebugMacro("AddMRMLObservers: watching for node removed, added, scene close events on the scene");
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
    if (this->MRMLScene->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
      this->MRMLScene->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }

    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::RemoveMRMLObservers ( )
{/*
    // remove observers on the ruler nodes
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationSplineNode");
    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationSplineNode *node = vtkMRMLAnnotationSplineNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationSplineNode"));
        if (node->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            node->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
        if (node->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            node->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
    if (node->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
    {
      node->RemoveObservers(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
        node = NULL;
    }
    //events->Delete();

    if (this->MRMLScene)
    {
        //vtkDebugMacro("RemoveMRMLObservers: stopping watching for node removed, added, scene close events on the scene");
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::AddSplineWidget(vtkMRMLAnnotationSplineNode *node)
{/*
    if (!node)
    {
        return;
    }
    if (this->GetSplineWidget(node->GetID()) != NULL)
    {
        //vtkDebugMacro("Already have widgets for node " << node->GetID());
        return;
    }

  vtkSplineWidget* c = vtkSplineWidget::New();

    this->SplineWidgets[node->GetID()] = c;

  if (node->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    node->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  }
  if (node->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    node->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
  }
  if (node->HasObserver ( vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    node->AddObserver ( vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this->MRMLCallbackCommand );
  }
  if (node->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    node->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
*/
}

//---------------------------------------------------------------------------
/*vtkSplineWidget* vtkSlicerAnnotationSplineManager::GetSplineWidget(const char *nodeID)
{
    std::map<std::string, vtkSplineWidget *>::iterator iter;
    for (iter = this->SplineWidgets.begin(); iter != this->SplineWidgets.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }
    return NULL;
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::RemoveSplineWidget(vtkMRMLAnnotationSplineNode *node)
{
    if (!node)
    {
        return;
    }
    /*if (this->GetSplineWidget(node->GetID()) != NULL)
    {
        //this->SplineWidgets[node->GetID()]->Delete();
        //this->SplineWidgets.erase(node->GetID());
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::RemoveSplineWidgets()
{/*
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationSplineNode");
    //vtkDebugMacro("RemoveSplineWidgets: have " << nnodes << " ruler  nodes in the scene, " << this->SplineWidgets.size() << " widgets defined already");

    if (nnodes == 0)
    {
        // the scene was closed, all the nodes are gone, so do this w/o reference
        // to the nodes
        //vtkDebugMacro("RemoveDistanceWidgets: no ruler nodes in scene, removing ruler widgets w/o reference to nodes");
        std::map<std::string, vtkSplineWidget *>::iterator iter;
        for (iter = this->SplineWidgets.end(); iter != this->SplineWidgets.end(); iter++)
        {
            //vtkDebugMacro("RemoveSplineWidgets: deleting and erasing " << iter->first);
            iter->second->Delete();
            //this->SplineWidgets.erase(iter);
        }
        this->SplineWidgets.clear();
    }
    else
    {
        for (int n=0; n<nnodes; n++)
        {
            vtkMRMLAnnotationSplineNode *node = vtkMRMLAnnotationSplineNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationSplineNode"));
            if (node)
            {
                this->RemoveSplineWidget(node);
            }
        }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::RequestRender()
{
  //this->GetViewerWidget()->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::Render()
{
  //this->GetViewerWidget()->Render();
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationSplineManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  if (this->ViewerWidget != NULL)
  {
    // TODO: figure out if this is necessary
    this->RemoveSplineWidgets();
    if (this->ViewerWidget->HasObserver(
      vtkSlicerViewerWidget::ActiveCameraChangedEvent, 
      //this->GUICallbackCommand) == 1)
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

  //vtkDebugMacro("SetViewerWidget: Updating any widget interactors");
  this->UpdateSplineWidgetInteractors();
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::UpdateSplineWidgetInteractors()
{/*
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
    this->GetViewerWidget()->GetMainViewer() == NULL ||
    this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
  {
    //vtkDebugMacro("UpdateSplineWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
  }

  std::map<std::string, vtkSplineWidget *>::iterator iter;
  for (iter = this->SplineWidgets.begin(); iter !=  this->SplineWidgets.end(); iter++)
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
      vtkMRMLAnnotationSplineNode *splineNode = NULL;
      if (this->GetMRMLScene())
      {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
        {
          splineNode = vtkMRMLAnnotationSplineNode::SafeDownCast(node);
        }
      }
      if (splineNode != NULL)
      {
        //this->Update3DWidgetVisibility(roiNode);
      }
    }
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::UpdateLockUnlock(vtkMRMLAnnotationSplineNode* node)
{/*
  if (node == NULL)
  {
    return;
  }

  vtkSplineWidget *widget = this->GetSplineWidget(node->GetID());
  if (!widget)
  {
    cout << "No distance widget found, adding a distance widget for this one" << endl;
    return;
  }

  if ( node->GetLocked() )
  {
    //widget->ProvessEventOff();
  } 
  else
  {
    //widget->ProvessEventOn();
  }*/
}

void vtkSlicerAnnotationSplineManager::UpdateVisibility(vtkMRMLAnnotationSplineNode *activeNode)
{
  /*vtkSplineWidget *widget = this->GetSplineWidget(activeNode->GetID());
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
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationSplineManager::UpdateSplineMeasurement(vtkMRMLAnnotationSplineNode* node, vtkSplineWidget* widget)
{
  //vtkSplineWidget* widget = this->GetSplineWidget(node->GetID());
  if ( widget==NULL)
  {
    return;
  }  

  if ( widget->GetSummedLength() != node->GetSplineMeasurement() )
  {
    node->SetSplineMeasurement(widget->GetSummedLength());
  }
}
