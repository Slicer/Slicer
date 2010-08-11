#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationFiducialManager.h"

//#include "vtkSlicerSeedWidgetClass.h"
#include "vtkSphereHandleRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerApplication.h"
//#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLFiducialListNode.h"

#include "vtkPointHandleRepresentation3D.h"
#include "vtkSeedWidget.h"
#include "vtkSeedRepresentation.h"

class vtkSlicerAnnotationFiducialManagerCallback : public vtkCommand
{
public:
  static vtkSlicerAnnotationFiducialManagerCallback *New()
  { return new vtkSlicerAnnotationFiducialManagerCallback; }
  virtual void Execute (vtkObject *caller, unsigned long event, void *callData)
  {
      /*
    if (event ==  vtkCommand::PlacePointEvent)
    {
      std::cout << "Fiducial Manager: PlacePointEvent.\n";
      if (this->FiducialListNode)
      {
        if (this->FiducialListNode->GetScene())
        {
          this->FiducialListNode->GetScene()->SaveStateForUndo(this->FiducialListNode);
        }
        // get the one that was added
        vtkSeedWidget *seedWidget = reinterpret_cast<vtkSeedWidget*>(caller);
        int *n =  reinterpret_cast<int *>(callData);
        if (seedWidget && n )
        {
          double p[3];
          vtkSeedRepresentation *rep = NULL;
          rep = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
          if (rep)
          {
            rep->GetSeedWorldPosition(*n, p);
            // does the fiducial list node have a transform? undo it?
            this->FiducialListNode->AddFiducialWithXYZ(float(p[0]), float(p[1]), float(p[2]), 1);
          }
        }
      }
    }
    // save node for undo if it's the start of an interaction event
    else if (event == vtkCommand::StartInteractionEvent)
    {
      std::cout << "StartInteraction Event\n";
      if (this->FiducialListNode && this->FiducialListNode->GetScene())
      {
        this->FiducialListNode->GetScene()->SaveStateForUndo(this->FiducialListNode);
      }
    }
    else if (event == vtkCommand::InteractionEvent)
    {
      vtkSeedWidget *seedWidget = reinterpret_cast<vtkSeedWidget*>(caller);

      if (!seedWidget)
      {
        std::cerr << "InteractionEvent: Null seed widget, returning.\n";
        return;
      }
      if (!this->FiducialListNode)
      {
        std::cerr << "InteractionEvent: no fiducial list associated with this widget.\n";
        return;
      }
      vtkSeedRepresentation *rep = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
      if (!rep)
      {
        std::cerr << "Representation is null.\n";
        return;
      }
      // does the fiducial list node have a transform?
      vtkMRMLTransformNode* tnode = this->FiducialListNode->GetParentTransformNode();
      //vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
      //transformToWorld->Identity();
      if (tnode != NULL && tnode->IsLinear())
      {
        vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
        lnode->GetMatrixTransformToWorld(transformToWorld);
      }
      // will convert by the inverted parent transform
      transformToWorld->Invert();
      if (callData != NULL)
      {
        // have a single seed that moved
        int *n =  reinterpret_cast<int *>(callData);
        if (n ||
          (n == 0 && rep->GetNumberOfSeeds() == 1))
        {

          double p[3];
          rep->GetSeedWorldPosition(*n, p);

          // convert by the inverted parent transform of the fiducial list
          double  xyzw[4];
          xyzw[0] = p[0];
          xyzw[1] = p[1];
          xyzw[2] = p[2];
          xyzw[3] = 1.0;
          double worldxyz[4], *worldp = &worldxyz[0];

          transformToWorld->MultiplyPoint(xyzw, worldp);
          //std::cout << "n is " << *n << ", p = " << p[0] << ", " << p[1] <<
          //", " << p[2] << ", setting nth fid to world " <<  worldxyz[0] <<
          //"," << worldxyz[1] << "," << worldxyz[2] << std::endl;
          // figure out which fiducial id this nth seed widget is associated with
          int fidIndex = *n;
          if (this->SeedWidgetClass)
          {
            //std::string fidID = this->SeedWidgetClass->GetIDFromIndex(*n);
            if (fidID.compare("") != 0)
            {
              // now get the fiducial index from the id
              fidIndex = this->FiducialListNode->GetFiducialIndex(fidID.c_str());
              //std::cout << "InteractionEvent: for widget index " << *n << " got fidID " << fidID.c_str() << ", and fiducial index = " << fidIndex << std::endl;
            }
          }
          this->FiducialListNode->SetNthFiducialXYZ(fidIndex, worldxyz[0], worldxyz[1], worldxyz[2]);
        }
        else
        {
          std::cout << "InteractionEvent: no point number given in the call data, returning.";
        }
      }
      else
      {
        // iterate through all the seeds and update the list
        //std::cout << "InteractionEvent: iterating through all the seeds\n";
        int numSeeds = rep->GetNumberOfSeeds();
        for (int n = 0; n < numSeeds; n++)
        {
          double p[3];
          rep->GetSeedWorldPosition(n, p);
          // convert by the inverted parent transform of the fiducial list
          double  xyzw[4];
          xyzw[0] = p[0];
          xyzw[1] = p[1];
          xyzw[2] = p[2];
          xyzw[3] = 1.0;
          double worldxyz[4], *worldp = &worldxyz[0];

          transformToWorld->MultiplyPoint(xyzw, worldp);
          this->FiducialListNode->SetNthFiducialXYZ(n, worldxyz[0], worldxyz[1], worldxyz[2]);
        } // end for
      }
      transformToWorld = NULL;
      tnode = NULL;
    }*/
  }
  //vtkSlicerAnnotationFiducialManagerCallback():FiducialListNode(0), SeedWidgetClass(0) {}
  vtkMRMLFiducialListNode *FiducialListNode;
  //vtkSlicerSeedWidgetClass *SeedWidgetClass;
};

class vtkSlicerAnnotationFiducialManagerNodeCallback : public vtkCommand
{
public:
  static vtkSlicerAnnotationFiducialManagerNodeCallback *New()
  { return new vtkSlicerAnnotationFiducialManagerNodeCallback; }
  virtual void Execute (vtkObject *caller, unsigned long event, void *callData)
  {
    if (event ==  vtkCommand::PlacePointEvent)
    {
      std::cout << "Fiducial Manager Node Call Back: PlacePointEvent.\n";
      if (this->FiducialNode)
      {
        if (this->FiducialNode->GetScene())
        {
          this->FiducialNode->GetScene()->SaveStateForUndo(this->FiducialNode);
        }
        // get the one that was added
        vtkSeedWidget *seedWidget = reinterpret_cast<vtkSeedWidget*>(caller);
        int *n =  reinterpret_cast<int *>(callData);
        if (seedWidget && n )
        {
          double p[3];
          vtkSeedRepresentation *rep = NULL;
          rep = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
          if (rep)
          {
            rep->GetSeedWorldPosition(*n, p);
            // does the fiducial list node have a transform? undo it?
            this->FiducialNode->SetFiducialCoordinates(p);
          }
        }
      }
    }
  }
  //vtkSlicerAnnotationFiducialManagerNodeCallback():FiducialNode(0), SeedWidgetClass(0) {}
  vtkMRMLAnnotationFiducialNode *FiducialNode;
  //vtkSlicerSeedWidgetClass *SeedWidgetClass;
};


//---------------------------------------------------------------------------
//vtkStandardNewMacro (vtkSlicerAnnotationFiducialManager );
//vtkCxxRevisionMacro ( vtkSlicerAnnotationFiducialManager, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkMeasurementsFiducialWidget: " << this->GetClassName ( ) << "\n";
    std::map<std::string, vtkSlicerSeedWidgetClass*>::iterator iter;
    for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
      {
      os << indent << "Seed Widget: " << iter->first.c_str() << "\n";
      iter->second->GetWidget()->PrintSelf(os, indent);
      }
}*/


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::SetFiducialNodeID ( char *id )
{
  if (this->GetFiducialNodeID() != NULL &&
    id != NULL &&
    strcmp(id,this->GetFiducialNodeID()) == 0)
  {
    //vtkDebugMacro("SetFiducialNodeID: no change in id, not doing anything for now: " << id << endl);
    return;
  }

  // set the id properly - see the vtkSetStringMacro
  this->FiducialNodeID = id;

  if (id == NULL)
  {
    //vtkDebugMacro("SetFiducialNodeID: NULL input id, clearing GUI and returning.\n");
    this->UpdateWidget(NULL);
    return;
  }

  // get the new node
  vtkMRMLAnnotationFiducialNode *newFiducial = vtkMRMLAnnotationFiducialNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetFiducialNodeID()));
  // set up observers on the new node - now done in addistancewidget
  if (newFiducial != NULL)
  {
    // set up the GUI
    this->UpdateWidget(newFiducial);
  }
  else
  {
    //vtkDebugMacro ("ERROR: unable to get the mrml Fiducial node to observe!\n");
  }
}*/


//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtStartInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
/*void vtkSlicerAnnotationFiducialManager::UpdateInteractionModeAtStartInteraction()
{

  if ( this->MRMLScene == NULL )
    {
    //vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }*/
  //vtkMRMLInteractionNode *interactionNode =
  //  vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  /*if ( interactionNode == NULL )
    {
    //vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
    return;
    }*/

  //--- check to see if we're already in "place" mode, and
  //--- if so, allow 1 click to create an Fiducial widget with its
  //--- Fiducial vertex at the click-location.
  //--- If we are not in place mode, then use the mouse-click
  //--- to pick the widget.
  //--- TODO: In the future, may want to modify for THREE 
  //--- clicks to place individual vertices of an Fiducial widget.
  //int currentMode = interactionNode->GetCurrentInteractionMode();
  //int pickPersistence = interactionNode->GetPickModePersistence();
  /*if ( currentMode != vtkMRMLInteractionNode::Place && pickPersistence == 0 )
    {
    interactionNode->SetLastInteractionMode ( currentMode );
    }
  interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::PickManipulate);

}*/



//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtEndInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
/*void vtkSlicerAnnotationFiducialManager::UpdateInteractionModeAtEndInteraction()
{
  if ( this->MRMLScene == NULL )
    {
    //vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }

  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    //vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
    return;
    }

  int pickPersistence = interactionNode->GetPickModePersistence();
  int placePersistence = interactionNode->GetPlaceModePersistence ();
  if ( pickPersistence == 0 && placePersistence == 0 )
    {
    interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
    }
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
/*
  vtkMRMLFiducialListNode *callerList = vtkMRMLFiducialListNode::SafeDownCast(caller);
  vtkMRMLFiducialListNode *callDataList =  NULL;
  
  //if (callData != NULL)
  //{
  callDataList = reinterpret_cast<vtkMRMLFiducialListNode *>(callData);
  //}

  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

  // the scene was closed, don't get node removed events so clear up here
  if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
  {
    //vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
    // the lists are already gone from the scene, so need to clear out all the
    // widget properties, can't call remove with a node
    this->RemoveSeedWidgets();
    return;
  }
  
  // first check to see if there was a Fiducial list node deleted
  if (callScene != NULL && callScene == this->MRMLScene && event == vtkMRMLScene::NodeRemovedEvent)
  {
    //vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
    // check to see if it was the current node that was deleted
    if (callData != NULL)
    {
      vtkMRMLAnnotationFiducialNode *delNode = reinterpret_cast<vtkMRMLAnnotationFiducialNode*>(callData);
      if (delNode != NULL && delNode->IsA("vtkMRMLAnnotationFiducialNode"))
      {
        //vtkDebugMacro("A Fiducial node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
        this->RemoveSeedWidget(delNode);
        this->RemoveTextDisplayNodeID(delNode);
        this->RemovePointDisplayNodeID(delNode);
      }
    }
  }

  // check for a node added to the scene event
  if (callScene != NULL && callScene == this->MRMLScene && callData != NULL && event == vtkMRMLScene::NodeAddedEvent)
  {
    //vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");
    // check to see if it was a Fiducial node    
    vtkMRMLAnnotationFiducialNode *addNode = reinterpret_cast<vtkMRMLAnnotationFiducialNode*>(callData);
    vtkMRMLFiducialListNode *addNode2 = reinterpret_cast<vtkMRMLFiducialListNode*>(callData);
    if (addNode != NULL && addNode->IsA("vtkMRMLAnnotationFiducialNode"))
    {
      //vtkDebugMacro("Got a node added event with a Fiducial node " << addNode->GetID());
      this->AddSeedWidget(addNode);
      return;
    }
    else if (addNode2 != NULL && addNode2->IsA("vtkMRMLFiducialListNode"))
    {
      this->AddSeedWidget(addNode2);
      //this->InvokeEvent(vtkSlicerAnnotationFiducialManager::AddFiducialCompletedEvent);
    }
  }
  else if (callData != NULL && event == vtkMRMLScene::NodeAddedEvent)
  {
    vtkMRMLFiducialListNode *addNode = reinterpret_cast<vtkMRMLFiducialListNode*>(callData);
    if (addNode != NULL && addNode->IsA("vtkMRMLFiducialListNode"))
    {
    //vtkDebugMacro("Got a node added event with a Fiducial node " << addNode->GetID());
    this->AddSeedWidget(addNode);
    this->InvokeEvent(vtkSlicerAnnotationFiducialManager::AddFiducialCompletedEvent);
    if (this->GetSeedWidget(addNode->GetID())==NULL)
    {
      this->AddSeedWidget(addNode);
      this->InvokeEvent(vtkSlicerAnnotationFiducialManager::AddFiducialCompletedEvent);
    }
    }
  }
  else if (callerList != NULL && event == vtkMRMLFiducialListNode::DisplayModifiedEvent)
  {
    this->UpdateLockUnlock(callerList);
  }
 */
}


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::UpdateWidget(vtkMRMLAnnotationFiducialNode *activeFiducialNode)
{ 
  //vtkDebugMacro("UpdateWidget: active Fiducial node is " << (activeFiducialNode == NULL ? "null" : activeFiducialNode->GetName()));

  // if the passed node is null, clear out the widget
  if (activeFiducialNode == NULL)
  {
    // don't need to do anything yet, especially don't set the node selector to
    // null, as it causes a crash
    //vtkDebugMacro("UpdateWidget: The passed in node is null, returning.");
    return;
  }

  //vtkDebugMacro("UpdateWidget: updating the gui and 3d elements");

   
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::Update3DWidget(vtkMRMLFiducialListNode *activeFiducialListNode)
{
  cout << "vtkSlicerAnnotationFiducialManager::Update3DFiducial Start" << endl;

  if (activeFiducialListNode == NULL)
  {
    //vtkDebugMacro("Update3DWidget: passed in Fiducial node is null, returning");
    return;
  }

  if (this->Updating3DFiducial)
  {
    //vtkDebugMacro("Already updating 3d widget");
    return;
  }


  // request a render
  /*if (this->GetViewerWidget())
  {
    this->GetViewerWidget()->RequestRender();
  }*/
  // reset the flag
  this->Updating3DFiducial = 0;
  cout << "vtkSlicerAnnotationModuleLogic::Update3DFiducial End: Successful" << endl;

}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::UpdateDisplayNode(vtkMRMLAnnotationFiducialNode* FiducialNode)
{
  cout << "vtkSlicerAnnotationFiducialManager::UpdateDisplayNode Start" << endl;

  if (FiducialNode == NULL)
  {
    //vtkDebugMacro("Update3DWidget: passed in Fiducial node is null, returning");
    return;
  }

  std::string textDisplayNodeID = this->GetTextDisplayNodeID(FiducialNode->GetID());
  if (textDisplayNodeID.empty())
  {
    cout << "No text display node found, adding a new text display node for this one" << endl;
    this->AddTextDiaplayNodeID(FiducialNode);
    textDisplayNodeID = this->GetTextDisplayNodeID(FiducialNode->GetID());
    if (textDisplayNodeID.empty())
    {
      //vtkErrorMacro("Error adding a new text display node for Fiducial node " << FiducialNode->GetID());
      return;
    }
  }

  std::string pointDisplayNodeID = this->GetPointDisplayNodeID(FiducialNode->GetID());
  if (pointDisplayNodeID.empty())
  {
    cout << "No point display node found, adding a new text display node for this one" << endl;
    this->AddPointDiaplayNodeID(FiducialNode);
    pointDisplayNodeID = this->GetPointDisplayNodeID(FiducialNode->GetID());
    if (pointDisplayNodeID.empty())
    {
      //vtkErrorMacro("Error adding a new point display node for Fiducial node " << FiducialNode->GetID());
      return;
    }
  }
}*/

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node)
{
  cout << "vtkSlicerAnnotationFiducialManager::Update Text Display Property" << endl;

  if (node == NULL)
  {
    //vtkDebugMacro("UpdateTextDisplayProperty: passed in text display node is null, returning");
    return;
  }

  std::string FiducialNodeID = this->GetFiducialNodeIDFromTextDisplayNode(node->GetID());

}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node)
{
  cout << "vtkSlicerAnnotationFiducialManager::Update Point Display Property" << endl;

  if (node == NULL)
  {
    //vtkDebugMacro("UpdateTextDisplayProperty: passed in point display node is null, returning");
    return;
  }

  std::string FiducialNodeID = this->GetFiducialNodeIDFromPointDisplayNode(node->GetID());
  if (FiducialNodeID.empty())
  {
    cout << "No Fiducial found" << endl;
    return;
  }

}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddMRMLObservers ( )
{
  // the widget as a whole needs to keep track of Fiducial nodes in the scene
  /*if (this->MRMLScene)
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
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemoveMRMLObservers ( )
{
  // remove observers on the Fiducial nodes
  //int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  /*for (int n=0; n<nnodes; n++)
  {
    vtkMRMLAnnotationFiducialNode *FiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationFiducialNode"));
    if (FiducialNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
    {
      FiducialNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (FiducialNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
    {
      FiducialNode->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    FiducialNode = NULL;
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
void vtkSlicerAnnotationFiducialManager::UpdateFiducialWidgetInteractors()
{
  //bool isNull = false;
  /*if (this->GetViewerWidget() == NULL ||
      this->GetViewerWidget()->GetMainViewer() == NULL ||
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
    {
    //vtkDebugMacro("UpdateFiducialWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
    }

  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
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
      // now update the visibility for the Fiducial
      vtkMRMLFiducialListNode *FiducialListNode = NULL;
      if (this->GetMRMLScene())
        {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
          {
          FiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
          }
        }
      if (FiducialListNode != NULL)
        {
        this->Update3DWidgetVisibility(FiducialListNode);
        }
      }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddSeedWidget(vtkMRMLAnnotationFiducialNode *FiducialNode)
{
  if (!FiducialNode)
  {
    return;
  }
  /*if (this->GetSeedWidget(FiducialNode->GetID()) != NULL)
  {
    //vtkDebugMacro("Already have widgets for Fiducial node " << FiducialNode->GetID());
    return;
  }

  vtkSlicerSeedWidgetClass *c = vtkSlicerSeedWidgetClass::New();
  this->SeedWidgets[FiducialNode->GetID()] = c;

  // now add call back
  vtkSlicerAnnotationFiducialManagerNodeCallback *myCallback = vtkSlicerAnnotationFiducialManagerNodeCallback::New();
  myCallback->FiducialNode = FiducialNode;
  myCallback->SeedWidgetClass = c;
  c->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
  c->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
  c->GetWidget()->AddObserver(vtkCommand::PlacePointEvent, myCallback);
  myCallback->Delete();

  // watch for control point modified events
  if (FiducialNode->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->AddObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
  if (FiducialNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
  if (FiducialNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
  if (FiducialNode->HasObserver(vtkMRMLAnnotationFiducialNode::FiducialNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->AddObserver(vtkMRMLAnnotationFiducialNode::FiducialNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
  if (FiducialNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddSeedWidget(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (!fiducialListNode)
  {
    //vtkErrorMacro("AddSeedWidget: null fiducial list node, can't add a seed widget without one");
    return;
  }
  /*if (this->GetSeedWidget(fiducialListNode->GetID()) != NULL)
  {
    //vtkDebugMacro("AddSeedWidget: Already have widgets for fiducial list node " << fiducialListNode->GetID());
    return;
  }*/

  //vtkDebugMacro("AddSeedWidget: Creating new fiducial list widget class for list with id " << fiducialListNode->GetID());
  /*vtkSlicerSeedWidgetClass *c = vtkSlicerSeedWidgetClass::New();

  // now add call back
  vtkSlicerAnnotationFiducialManagerCallback *myCallback = vtkSlicerAnnotationFiducialManagerCallback::New();
  myCallback->FiducialListNode = fiducialListNode;
  myCallback->SeedWidgetClass = c;
  c->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
  c->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
  c->GetWidget()->AddObserver(vtkCommand::PlacePointEvent, myCallback);
  myCallback->Delete();

  this->SeedWidgets[fiducialListNode->GetID()] = c;

  // make sure we're observing the node events
  if (fiducialListNode->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    fiducialListNode->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  }
  // watch for a transform modified event from the list
  if (fiducialListNode->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    fiducialListNode->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
  }
  // observe display changes so can update the representation
  if (fiducialListNode->HasObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {     
    fiducialListNode->AddObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
  }
  // watch for indices being swapped, will have to update the widgets
  if (fiducialListNode->HasObserver ( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {     
    fiducialListNode->AddObserver ( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand );
  }
  // watch for a fiducial point being modified
  if (fiducialListNode->HasObserver ( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand ) == 0)
  {
    fiducialListNode->AddObserver( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand );
  }
  // fiducial point removed?
  if (fiducialListNode->HasObserver ( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand ) == 0)
  {
    fiducialListNode->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
  }
  // watch for node added events on the fiducial list (triggered when add a new point)
  if (fiducialListNode->HasObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand ) == 0)
  {
    fiducialListNode->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
  }
  if (fiducialListNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    fiducialListNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }
  if (fiducialListNode->HasObserver(vtkMRMLFiducialListNode::DisplayModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    fiducialListNode->AddObserver(vtkMRMLFiducialListNode::DisplayModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }

  // set the interactor first
  this->Update3DWidgetVisibility(fiducialListNode);

  // bit of a hack: if the list was loaded from mrml it comes in with fids
  // already in it, no add fiducial event is captured, so add them one by one first
  if (fiducialListNode->GetNumberOfFiducials() != 0)
  {
    int numFids = fiducialListNode->GetNumberOfFiducials();
    //vtkDebugMacro("AddSeedWidget: list already has " << numFids << ", so iterating over add seed");
    for (int f = 0; f < numFids; f++)
    {
      this->AddSeed(fiducialListNode, fiducialListNode->GetNthFiducialID(f));
      this->UpdateSeed(fiducialListNode, fiducialListNode->GetNthFiducialID(f));
    }
  }

  // now update it
  //vtkDebugMacro("AddSeedWidget: calling update on the new widget");
  this->Update3DWidget(fiducialListNode);
  */
}

//---------------------------------------------------------------------------
/*vtkSlicerSeedWidgetClass* vtkSlicerAnnotationFiducialManager::GetSeedWidget(const char *nodeID)
{
  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin(); iter != this->SeedWidgets.end(); iter++)
  {
    if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
    {
      return iter->second;
    }
  }
  return NULL;
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemoveSeedWidget(vtkMRMLAnnotationFiducialNode *FiducialNode)
{
  if (!FiducialNode)
  {
    return;
  }
  /*if (this->GetSeedWidget(FiducialNode->GetID()) != NULL)
  {
    this->SeedWidgets[FiducialNode->GetID()]->Delete();
    this->SeedWidgets.erase(FiducialNode->GetID());
  }*/

}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationFiducialManager::GetTextDisplayNodeID(const char *nodeID)
{
  /*std::string tempstring;
  std::map<std::string, std::string>::iterator iter;
  for (iter = this->TextDisplayNodes.begin(); iter != this->TextDisplayNodes.end(); iter++)
  {
    if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
    {
      return iter->second;
    }
  }
  return tempstring;
*/
return NULL;
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationFiducialManager::GetFiducialNodeIDFromTextDisplayNode(const char *nodeID)
{
  /*std::string tempstring;
  std::map<std::string, std::string>::iterator iter;
  for (iter = this->TextDisplayNodes.begin();  iter != this->TextDisplayNodes.end(); iter++)
  {
    if (iter->second.c_str() && !strcmp(iter->second.c_str(), nodeID))
    {
      return iter->first;
    }
  }
  return tempstring;*/ return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddTextDiaplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode)
{
  /*if (!FiducialNode)
  {
    return;
  }
  if (FiducialNode->GetAnnotationTextDisplayNode() == NULL)
  {
    FiducialNode->CreateAnnotationTextDisplayNode();
  }

  this->TextDisplayNodes[FiducialNode->GetID()] = FiducialNode->GetAnnotationTextDisplayNode()->GetID();
  if (FiducialNode->GetAnnotationTextDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->GetAnnotationTextDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemoveTextDisplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode)
{
  if (!FiducialNode)
  {
    return;
  }
  /*if (this->GetTextDisplayNodeID(FiducialNode->GetID()).empty())
  {
    this->TextDisplayNodes.erase(FiducialNode->GetID());
  }*/
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationFiducialManager::GetPointDisplayNodeID(const char *nodeID)
{
  /*std::string tempstring;
  std::map<std::string, std::string>::iterator iter;
  for (iter = this->PointDisplayNodes.begin(); iter != this->PointDisplayNodes.end(); iter++)
  {
    if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
    {
      return iter->second;
    }
  }
  return tempstring;*/ return NULL;
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationFiducialManager::GetFiducialNodeIDFromPointDisplayNode(const char *nodeID)
{/*
  std::string tempstring;
  std::map<std::string, std::string>::iterator iter;
  for (iter = this->PointDisplayNodes.begin(); iter != this->PointDisplayNodes.end(); iter++)
  {
    if (iter->second.c_str() && !strcmp(iter->second.c_str(), nodeID))
    {
      return iter->first;
    }
  }
  return tempstring;*/ return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddPointDiaplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode)
{/*
  if (!FiducialNode)
  {
    return;
  }
  if (FiducialNode->GetAnnotationPointDisplayNode() == NULL)
  {
    FiducialNode->CreateAnnotationPointDisplayNode();
  }

  this->PointDisplayNodes[FiducialNode->GetID()] = FiducialNode->GetAnnotationPointDisplayNode()->GetID();
  if (FiducialNode->GetAnnotationPointDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
  {
    FiducialNode->GetAnnotationPointDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemovePointDisplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode)
{/*
  if (!FiducialNode)
  {
    return;
  }
  if (this->GetPointDisplayNodeID(FiducialNode->GetID()).empty())
  {
    this->PointDisplayNodes.erase(FiducialNode->GetID());
  }*/
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationFiducialManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  if (this->ViewerWidget != NULL)
  {
    // TODO: figure out if this is necessary
    //this->RemoveSeedWidgets();
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

  //vtkDebugMacro("SetViewerWidget: Updating any widget interactors");
  this->UpdateFiducialWidgetInteractors();
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::Update3DWidgetVisibility(vtkMRMLFiducialListNode *activeFiducialListNode)
{/*
  if (activeFiducialListNode == NULL)
  {
    //vtkDebugMacro("Update3DWidget: passed in Fiducial node is null, returning");
    return;
  }
  vtkSlicerSeedWidgetClass *FiducialWidgetClass = this->GetSeedWidget(activeFiducialListNode->GetID());
  if (!FiducialWidgetClass)
  {
    //vtkErrorMacro("Update3DWidgetVisibility: no widget to update!");
    return;
  }

  vtkSeedWidget *FiducialWidget = FiducialWidgetClass->GetWidget();

  if (FiducialWidget->GetInteractor() == NULL)
  {
    if (this->GetViewerWidget() && this->GetViewerWidget()->GetMainViewer() && this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor())
    {
      FiducialWidgetClass->GetWidget()->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
    }
    else
    {
      FiducialWidgetClass->GetWidget()->SetInteractor(NULL);
    }
  }

  if ( activeFiducialListNode->GetVisibility() )
  {
    if (FiducialWidgetClass->GetWidget()->GetInteractor())
    {
      //vtkDebugMacro("Update3DWidgetVisibility: turning it on");
      FiducialWidgetClass->GetWidget()->On();
      // First, get out out of the mode where we are interactively defining seeds
      FiducialWidgetClass->GetWidget()->CompleteInteraction();

      // now iterate through the seeds to set visibility
      int numFids = activeFiducialListNode->GetNumberOfFiducials();
      for (int n = 0; n < numFids; n++)
      {
        //vtkDebugMacro("Update3DWidgetVisibility: fiducial " << n << " setting indiv visib")
          FiducialWidgetClass->SetNthSeedVisibility(n, activeFiducialListNode->GetNthFiducialVisibility(n));
      }
    }
  }
  else
  {
    //vtkDebugMacro("UpdateWidgetVisibility: fiducial list widget off");
    FiducialWidgetClass->GetWidget()->Off();
    FiducialWidgetClass->GetWidget()->ProcessEventsOff();
  }
  
  if (this->GetViewerWidget())
  {
    this->GetViewerWidget()->RequestRender();
  }*/
}


void vtkSlicerAnnotationFiducialManager::UpdateCamera()
{/*
  vtkCamera *cam = NULL;
  cam = this->GetActiveCamera();

  if (cam == NULL)
  {
    //vtkErrorMacro("UpdateCamera: unable to get active camera");
    return;
  }

  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
  {
    iter->second->SetCamera(cam);
  }*/
}

//---------------------------------------------------------------------------
vtkCamera *vtkSlicerAnnotationFiducialManager::GetActiveCamera()
{/*
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
  }*/
  return NULL;
}

void vtkSlicerAnnotationFiducialManager::UpdateFiducialMeasurement(vtkMRMLAnnotationFiducialNode* FiducialNode)
{/*
  if (this->GetViewerWidget()) 
  {
    this->GetViewerWidget()->Render();
  }*/

}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemoveSeedWidgets()
{/*
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  //vtkDebugMacro("RemoveSeedWidgets: have " << nnodes << " fiducial list nodes in the scene, " << this->SeedWidgets.size() << " widgets defined already");

  if (nnodes == 0)
  {
    // the scene was closed, all the nodes are gone, so do this w/o reference
    // to the nodes
    //vtkDebugMacro("RemoveSeedWidgets: no fiducials in scene, removing seeds w/o reference to nodes");
    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    for (iter = this->SeedWidgets.end();
      iter != this->SeedWidgets.end();
      iter++)
    {
      //vtkDebugMacro("RemoveSeeds: deleting and erasing " << iter->first);
      iter->second->Delete();
      this->SeedWidgets.erase(iter);
    }
    this->SeedWidgets.clear();
  }
  else
  {
    for (int n=0; n<nnodes; n++)
    {
      vtkMRMLAnnotationFiducialNode *rnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
      if (rnode)
      {
        this->RemoveSeedWidget(rnode);
      }
    }
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::AddSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID)
{
  if (!fiducialListNode)
  {
    //vtkErrorMacro("AddSeed: null fiducial list node, can't add a seed for a point in it");
    return;
  }
  // don't create the widget if it's not already there
  /*vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget != NULL)
  {
    int pointIndex;
    const char *addedPointID;
    if (pointID == NULL)
    {
      // use the id of the last point
      addedPointID = fiducialListNode->GetNthFiducialID(fiducialListNode->GetNumberOfFiducials() - 1);
    }
    else
    {
      addedPointID = pointID;
    }
    if (addedPointID == NULL)
    {
      //vtkErrorMacro("AddSeed: unable to get the point ID for the added seed.");
      return;
    }
    // get the point index from the point id
    pointIndex = fiducialListNode->GetFiducialIndex(addedPointID);
    if (pointIndex == -1)
    {
      //vtkErrorMacro("AddSeed: unable to find index for point id " << (addedPointID == NULL ? "null" : addedPointID) << " in list " << fiducialListNode->GetID());
      return;
    }
    // get the position
    double xyz[4];
    if (fiducialListNode->GetNthFiducialXYZWorld(pointIndex, xyz))
    {
      // add the seed at that position
      seedWidget->AddSeed(xyz, addedPointID);
    }
    else
    {
      //vtkErrorMacro("AddSeed: unable to get position for point at index " << pointIndex << ", from id " << addedPointID);
    }
  }
  else
  {
    //vtkErrorMacro("AddSeed: there wasn't a seed widget for the list " << fiducialListNode->GetID() << ", unable to add a seed to it");
  }*/
 
}
  
//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::RemoveSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID)
{/*
  if (!fiducialListNode)
  {
    //vtkErrorMacro("RemoveSeed: null input list!");
    return;
  }
  // don't create the widget if it's not already there
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget != NULL)
  {
    //seedWidget->RemoveSeed(pointIndex);
    seedWidget->RemoveSeedByID(pointID);
  }
  else
  {
    //vtkErrorMacro("RemoveSeed: couldn't find a seed widget for this list: " << fiducialListNode->GetID());
  }
  if (this->GetViewerWidget()) 
  {
    this->GetViewerWidget()->Render();
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::UpdateSeed(vtkMRMLFiducialListNode *flist, const char *fidID) // int f)
{
  if (flist == NULL || fidID == NULL) 
  {
    return;
  }
/*
  int f = flist->GetFiducialIndex(fidID);

  // don't create if doesn't exist for this list
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(flist->GetID());

  if (!seedWidget)
  {
    //vtkErrorMacro("UpdateSeed: can't update a seed, this list doesn't have a seed widget");
    return;
  }

  // get the index of the seed representing this fiducial
  int seedIndex = seedWidget->GetIndexFromID(fidID);
  if (seedIndex == -1)
  {
    //vtkErrorMacro("UpdateSeed: couldn't get the seed widget index from id " << fidID << ", using fiducial index of " << f);
    seedIndex = f;
  }
  else
  {
    //vtkDebugMacro("UpdateSeed: fiducial index = " << f << ", seed index = " << seedIndex);
  }

  double worldxyz[4];
  flist->GetNthFiducialXYZWorld(f, worldxyz);

  // does this seed exist already?
  if (seedWidget->GetNthSeedExists(f) == 0)
  {
    //vtkDebugMacro("Seed " << f << " does not exist, adding it");
    seedWidget->AddSeed(worldxyz, fidID);
  }

  int visib = 1;
  if (flist->GetVisibility() == 0 || flist->GetNthFiducialVisibility(f) == 0)
  {
    // Point is not visible
    visib = 0;
  }
  int locked = flist->GetLocked();
  int selected = flist->GetNthFiducialSelected(f);
  vtkCamera *cam = this->GetActiveCamera();
  seedWidget->SetNthSeed(seedIndex,
    cam,
    worldxyz, flist->GetNthFiducialLabelText(f),
    visib, locked, selected, 
    flist->GetColor(), flist->GetSelectedColor(),
    flist->GetTextScale(), flist->GetSymbolScale(),
    flist->GetGlyphType(),
    flist->GetOpacity(), flist->GetAmbient(), flist->GetDiffuse(), flist->GetSpecular(), flist->GetPower());

  if (this->GetViewerWidget()) 
  {
    this->GetViewerWidget()->Render();
  }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationFiducialManager::UpdateLockUnlock(vtkMRMLFiducialListNode* node)
{/*
  if (node == NULL)
  {
    return;
  }

  vtkSlicerSeedWidgetClass *widget = this->GetSeedWidget(node->GetID());
  if (!widget)
  {
    cout << "No distance widget found, adding a distance widget for this one" << endl;
    return;
  }

  widget->SetSeedsLocked( node->GetLocked() );
*/}
