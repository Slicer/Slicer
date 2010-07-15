#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerFiducialListWidget.h"

#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

#include "vtkRenderer.h"
#include "vtkProperty.h"

#include "vtkHandleWidget.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLFiducialListNode.h"


#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"

// for pick events
#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerViewerInteractorStyle.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"

// for the seed widget
#include "vtkSlicerSeedWidgetClass.h"
#include "vtkSeedWidget.h"
#include "vtkWidgetRepresentation.h"
#include "vtkSeedRepresentation.h"


// a call back for the seed widget
class vtkSlicerFiducialsSeedWidgetCallback  : public vtkCommand
{
public:
  static vtkSlicerFiducialsSeedWidgetCallback *New()
  { return new vtkSlicerFiducialsSeedWidgetCallback; }
  virtual void Execute (vtkObject *caller, unsigned long event, void *callData)
  {
    if (event ==  vtkCommand::PlacePointEvent)
      {
      //std::cout << "PlacePointEvent.\n";
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
      //std::cout << "StartInteraction Event\n";
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
      vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
      transformToWorld->Identity();
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
            std::string fidID = this->SeedWidgetClass->GetIDFromIndex(*n);
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
      }
  }
  vtkSlicerFiducialsSeedWidgetCallback():FiducialListNode(0), SeedWidgetClass(0) {}
  vtkMRMLFiducialListNode *FiducialListNode;
  vtkSlicerSeedWidgetClass *SeedWidgetClass;
};


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFiducialListWidget );
vtkCxxRevisionMacro ( vtkSlicerFiducialListWidget, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget::vtkSlicerFiducialListWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::Constructor");
  
  this->ProcessingMRMLEvent = 0;
  this->RenderPending = 0;

  this->ViewNode = NULL;
   
  // for picking
  this->ViewerWidget = NULL;
  this->InteractorStyle = NULL;

  this->Updating3DWidget = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveMRMLObservers()
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveMRMLObservers\n");
  this->RemoveFiducialObservers();

  this->SetAndObserveViewNode (NULL);
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget::~vtkSlicerFiducialListWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::Destructor\n");

  this->RemoveMRMLObservers();

  // this call was triggering an inf loop of observers when deleting the
  // second fid list widget
  this->SetViewerWidget(NULL);
  //this->ViewerWidget = NULL;
  
  this->SetInteractorStyle(NULL);
  
  // 3d widgets
  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin();
       iter != this->SeedWidgets.end();
       iter++)
    {
    iter->second->Delete();
    }
  this->SeedWidgets.clear();

  // already done in remove mrml observers
  vtkSetMRMLNodeMacro(this->ViewNode, NULL);

  
}
//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerFiducialListWidget: " << this->GetClassName ( ) << "\n";
    
    vtkIndent nextIndent;
    nextIndent = indent.GetNextIndent();
    if (this->GetViewerWidget() != NULL)
      {
      os << indent << "Main Viewer:\n";
      this->GetViewerWidget()->PrintSelf(os, nextIndent);
      }

    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
      {
      os << indent << "\nSeed Widget: " << iter->first.c_str() << "\n";
      iter->second->GetWidget()->PrintSelf(os, indent);
      }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::StartInteractionEvent)
    {
    this->UpdateInteractionModeAtStartInteraction();
    }
  else if (event == vtkCommand::EndInteractionEvent)
    {
    this->UpdateInteractionModeAtEndInteraction();
    }

  // check for pick events
  if (event == vtkSlicerViewerInteractorStyle::PickEvent &&
      vtkSlicerViewerInteractorStyle::SafeDownCast(caller) != NULL &&
      callData != NULL)
    {
    vtkDebugMacro("FiducialListWidget: Pick event!\n");
    // do the pick
    int x = ((int *)callData)[0];
    int y = ((int *)callData)[1];
    if (this->GetViewerWidget()->Pick(x,y) != 0)
      {
      // check for a valid RAS point
      double *rasPoint = this->GetViewerWidget()->GetPickedRAS();
      if (rasPoint != NULL)
        {
        vtkSlicerFiducialsGUI *fidGUI = vtkSlicerFiducialsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Fiducials"));
        if (fidGUI != NULL )
          {
          vtkSlicerFiducialsLogic *fidLogic  = fidGUI->GetLogic();
           
          // the fiducials logic will take care of applying any inverted
          // transform on the list
          int fidIndex = fidLogic->AddFiducialPicked(rasPoint[0], rasPoint[1], rasPoint[2], 1);
          if (fidIndex == -1)
            {
            vtkErrorMacro("Pick failed, could not add a fiducial at " << rasPoint[0] << "," << rasPoint[1] << "," << rasPoint[2]); 
            }
          else
            {
            vtkDebugMacro("Pick succeeded, new fiducial at index " << fidIndex);
            }
          // swallow the event
          if (this->GUICallbackCommand != NULL)
            {
            this->GUICallbackCommand->SetAbortFlag(1);
            }
          else
            {
            vtkErrorMacro("Unable to get the gui call back command that calls process widget events, event = " << event << " is not swallowed here");
            }
          }
        }
      }
    }
  else if (event == vtkSlicerViewerWidget::ActiveCameraChangedEvent &&
           vtkSlicerViewerWidget::SafeDownCast(caller) != NULL)
    {
    this->UpdateFiducialsCamera();
    }
} 

//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtStartInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkSlicerFiducialListWidget::UpdateInteractionModeAtStartInteraction()
{
  
/*
  //--- one way to get the interaction node...
  vtkSlicerFiducialsLogic *fidLogic  = vtkSlicerFiducialsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Fiducials"))->GetLogic();
  if ( fidLogic == NULL)
    {
    vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: got NULL fiducialsLogic" );
    return;
    }
    
  vtkSlicerApplicationLogic *appLogic = fidLogic->GetApplicationLogic();
  if (  appLogic == NULL )
    {
    vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: got NULL applicationLogic" );
    return;
    }

  vtkMRMLInteractionNode *interactionNode = appLogic->GetInteractionNode();
*/

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
  int currentMode = interactionNode->GetCurrentInteractionMode();
  if ( pickPersistence == 0 )
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
void vtkSlicerFiducialListWidget::UpdateInteractionModeAtEndInteraction()
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
void vtkSlicerFiducialListWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{
  
  vtkDebugMacro("ProcessMRMLEvents: processing = " << this->ProcessingMRMLEvent << ", event = " << event);
  
  if (this->ProcessingMRMLEvent != 0 )
    {
    vtkDebugMacro("ProcessMRMLEvents: Returning because already processing an event, " << this->ProcessingMRMLEvent);
    return;
    }

  this->ProcessingMRMLEvent = event;

  vtkMRMLFiducialListNode *callerList = vtkMRMLFiducialListNode::SafeDownCast(caller);
  vtkMRMLFiducialListNode *callDataList =  NULL;
  if (callData != NULL)
    {
    // safe down cast was causing a crash when moving a fiducial
    //vtkMRMLFiducialListNode::SafeDownCast((vtkObjectBase *)callData);
    callDataList = reinterpret_cast<vtkMRMLFiducialListNode *>(callData);
    }
  vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

  // the scene was closed, don't get node removed events so clear up here
  if (callScene != NULL &&
      event == vtkMRMLScene::SceneClosedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
    // the lists are already gone from the scene, so need to clear out all the
    // fiducial properties, can't call remove fiducial
    //this->RemoveFiducialProps();
    this->RemoveSeedWidgets();
    this->ProcessingMRMLEvent = 0;
    return;
    }

  // if get a node added event from the scene, check to see if it was a fiducial list that was added
  else if (callScene != NULL &&
           event == vtkMRMLScene::NodeAddedEvent &&
           callDataList != NULL &&
           vtkMRMLFiducialListNode::SafeDownCast ( (vtkObjectBase *)callData ))
    {
    vtkDebugMacro("Got a scene node added event: fiducial list node was added, adding a seed widget for it, list added has " << callDataList->GetNumberOfFiducials() << " fids");
    //this->UpdateFiducialListFromMRML(callDataList);
    //this->UpdateFromMRML();
    this->AddSeedWidget(callDataList);
    }

  // if get a node remove event
  else if (callScene != NULL &&
      event == vtkMRMLScene::NodeRemovedEvent &&
      callDataList != NULL)
    {
//    vtkMRMLFiducialListNode *flist = callDataList;
    // relying on the reinterpret_cast method was causing the removal of other node types to sneak in here.
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast ( (vtkObjectBase *)callData );
    if ( flist )
     {
     vtkDebugMacro("ProcessMRMLEvents: got a node removed event, fid list removed, size = " << flist->GetNumberOfFiducials());
     this->RemoveSeedWidget(flist);
     //this->UpdateFromMRML();
     }
    }

  // one fiducial was removed
  else if (callerList != NULL &&
           event == vtkMRMLScene::NodeRemovedEvent)
    {
    int pointNum = -1;
    const char *pointID = NULL;
    if (callData != NULL)
      {
      //vtkMRMLFiducial *fid = reinterpret_cast<vtkMRMLFiducial *>(callData);
      std::string *pointIDStr = reinterpret_cast<std::string *>(callData);
      if (pointIDStr != NULL)
        {
        pointID = pointIDStr->c_str();
        }
      }
    vtkDebugMacro("ProcessMRMLEvents: got a node removed event, a fiducial was removed from list, point num = " << pointNum << ", point id = " << (pointID == NULL ? "null" : pointID));
    if (pointID != NULL)
      {
      this->RemoveSeed(callerList, pointID);
      }
    //this->UpdateFromMRML();
    }

  else if (callerList != NULL &&
           event == vtkMRMLFiducialListNode::FiducialModifiedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got fiducial modified event");
    const char *pointID = NULL;
    if (callData != NULL)
      {
      std::string *pointIDStr = reinterpret_cast<std::string *>(callData);
      if (pointIDStr != NULL)
        {
        pointID = pointIDStr->c_str();
        }
      }
    if (pointID != NULL)
      {
      vtkDebugMacro("ProcessMRMLEvents: fiducial modified: calling update point widget with point id " << pointID << ", then update from mrml");
      this->UpdateSeed(callerList, pointID);
//      this->UpdateFiducialFromMRML(callerList, pointID);
//      this->UpdateFromMRML();
      }
    else
      {
      vtkDebugMacro("ProcessMRMLEvents: fiducial modified event on list " << callerList->GetName() << ", no point id given, calling update from mrml");
      // this may be an undo that deleted a point from the list, since the
      // point widgets are ad hoc right now, remove any widgets that don't
      // appear in a list
      //this->RemoveExtraPointWidgets(callerList);
      //this->UpdateFromMRML();
      this->Update3DWidget(callerList);
      }
    }
  
  // if it's a general fid display event, just update display properties
  else if (event == vtkMRMLFiducialListNode::DisplayModifiedEvent && callerList != NULL)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a fiduical display modified event " << event << ", calling update 3d widget display properties");
    this->Update3DWidgetDisplayProperties(callerList);
    }
  // if a fiducial index was modified, a fid was moved up or down in the list
  else if (callerList != NULL && event == vtkMRMLFiducialListNode::FiducialIndexModifiedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a Fiducial Index Modified Event " << event);
    FiducialListSwappedIndices *swap = reinterpret_cast<FiducialListSwappedIndices *>(callData);
    if (swap)
      {
      vtkDebugMacro("ProcessMRMLEvents: fid index modified, first = " << swap->first << ", second = " << swap->second);
      this->Swap(callerList, swap->first, swap->second);
      }
    else
      {
      vtkErrorMacro("\t unable to get the data structure saying which two indices swapped!");
      }
    }
  // if it's a modified
  // event on a fid list, update
  else if (callerList != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a  modified on list " << event << ", calling update widget");  
    this->Update3DWidget(callerList);
    }
  
    
  // if a fiducial was added to a list
  else if (callerList != NULL &&
           event ==  vtkMRMLScene::NodeAddedEvent && 
           callDataList != NULL)
    {
    vtkDebugMacro("ProcessMRMLEvents: got a node added event " << event << ", a fiducial was added to " << callDataList->GetID() << ", calling AddSeed and then UpdateSeed (list has " << callDataList->GetNumberOfFiducials() << " fids)");
    // is the list being read in via a storage node?
    int enabledFlag = 1;
    if (callDataList->GetStorageNode())
      {
      if (callDataList->GetStorageNode()->GetReadState() == vtkMRMLStorageNode::TransferDone)
        {
        // it's reading it in, don't enable it just yet
        enabledFlag = 0;
        //vtkWarningMacro("The storage node's read state is transfer done, so not enabling the new seed yet");
        }
      }
    this->AddSeed(callDataList, NULL);
    // need to update it, get the id of the last one we just added
    const char *addedPointID = callDataList->GetNthFiducialID(callDataList->GetNumberOfFiducials() - 1);
    vtkDebugMacro("ProcessMRMLEvents: got a node added event, now calling UpdateSeed with id " << addedPointID);
    this->UpdateSeed(callDataList, addedPointID);
    }

  // if the list transfrom was updated...
  else if (event == vtkMRMLTransformableNode::TransformModifiedEvent &&
      (callerList != NULL))
    {
    vtkDebugMacro("Got transform modified event, calling update positions on list " << callerList->GetID());
    this->Update3DWidgetPositions(callerList);
    }

  else if (vtkMRMLViewNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessingMRMLEvents: got a view node modified event");
    this->UpdateViewNode();
    this->RequestRender();
    }
  
  this->ProcessingMRMLEvent = 0;

}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::CreateWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::CreateWidget...\n");
  
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  // set up event observers
  if (this->MRMLScene)
    {
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
    events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
    events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    this->SetAndObserveMRMLSceneEvents (this->MRMLScene, events );
    events->Delete();
    }
}




//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateFromMRML()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  this->UpdateViewNode();
  
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  vtkDebugMacro("UpdateFromMRML: nnodes = " << nnodes);
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    this->Update3DWidget(flist);
    // let go of the pointer
    flist = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RequestRender()
{
    if (this->GetRenderPending())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after idle \"%s Render\"", this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Render()
{
  if (this->GetViewerWidget())
    {
    this->GetViewerWidget()->RequestRender();
    }
  this->SetRenderPending(0);
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateFiducialsFromMRML()
{
  vtkErrorMacro("UpdateFiducialsFromMRML: deprecated, returning.");
  return;
  /*
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (scene == NULL)
    {
    vtkErrorMacro("...the scene is null... returning");
    return;
    }

  vtkDebugMacro("UpdateFiducialsFromMRML: Starting to update the viewer's actors, glyphs for the fid lists.");
  
  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  vtkDebugMacro("UpdateFiducialsFromMRML: nnodes = " << nnodes);
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    vtkDebugMacro("n = " << n << ", calling update fiducial list from mrml");
    this->UpdateFiducialListFromMRML(flist);
    // let go of the pointer
    flist = NULL;
    }

  // Render
  if (this->ViewerWidget != NULL)
    {
    this->RequestRender();
    }
  */
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateFiducialListFromMRML(vtkMRMLFiducialListNode *flist)
{
  vtkDebugMacro("UpdateFiducialListFromMRML: deprecated, calling Update3DWidget instead");
  this->Update3DWidget(flist);
  return;
  
  // check for  widgets
  for (int f=0; f<flist->GetNumberOfFiducials(); f++)
    {
    std::string fid = flist->GetNthFiducialID(f);
    this->UpdateSeed(flist, fid.c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateSeed(vtkMRMLFiducialListNode *flist, const char *fidID) // int f)
{
  if (flist == NULL || fidID == NULL) 
    {
    return;
    }

  int f = flist->GetFiducialIndex(fidID);

  // don't create if doesn't exist for this list
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(flist->GetID(), 0);

  if (!seedWidget)
    {
    vtkErrorMacro("UpdateSeed: can't update a seed, this list doesn't have a seed widget");
    return;
    }

  double worldxyz[4];
  flist->GetNthFiducialXYZWorld(f, worldxyz);
  
  // get the index of the seed representing this fiducial
  int seedIndex = seedWidget->GetIndexFromID(fidID);
  if (seedIndex == -1)
    {
    vtkDebugMacro("UpdateSeed: couldn't get the seed widget index from id " << fidID << ", adding a new seed for fiducial index of " << f);
    seedIndex = seedWidget->AddSeed(worldxyz, fidID);;
    }
  else
    {
    vtkDebugMacro("UpdateSeed: fiducial index = " << f << ", seed index = " << seedIndex << ", from fid id " << fidID);
    }
  
  

  // does this seed exist already?
  if (seedWidget->GetNthSeedExists(seedIndex) == 0)
    {
    vtkDebugMacro("UpdateSeed: Seed index " << seedIndex << " for fid index " << f << " does not exist, adding it with ID " << fidID);
    seedIndex = seedWidget->AddSeed(worldxyz, fidID);
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

  // only request a render if the seed is enabled
  if (seedWidget->GetWidget()->GetSeed(seedIndex)->GetEnabled())
    {
    this->RequestRender();
    }
  else
    {
    //vtkWarningMacro("UpdateSeed: seed " << seedIndex << " is not enabled, skipping render request");
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveFiducialObservers()
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveFiducialObservers\n");
    if (this->GetMRMLScene() == NULL)
    {
    vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveFiducialObservers: no scene, returning...");
        return;
    }
    // remove the observers on all the fiducial lists
    vtkMRMLFiducialListNode *flist;
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
    for (int n=0; n<nnodes; n++)
      {
      flist = vtkMRMLFiducialListNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
      this->RemoveFiducialObserversForList(flist);
      }
    flist = NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveFiducialObserversForList(vtkMRMLFiducialListNode *flist)
{
  if (flist == NULL)
    {
    vtkWarningMacro("RemoveFiducialObserversForList: null input list!");
    return;
    }

  vtkDebugMacro("Removing observers on fiducial list " << flist->GetID());
  
  if (flist->HasObserver (vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
  if (flist->HasObserver( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
    }
   if (flist->HasObserver( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand );
    }
  if (flist->HasObserver( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    }
  if (flist->HasObserver(vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand );
    }
  if (flist->HasObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand ) == 1)
    {
    flist->RemoveObservers ( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
    }
  if (flist->HasObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand ) == 1)
    {
    vtkDebugMacro("Removing observer on node added event on the fid list");
    flist->RemoveObservers ( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetSeedWidgetMaterialProperties(vtkMRMLFiducialListNode *flist) 
{
  vtkDebugMacro("SetSeedWidgetMaterialProperties: flist = " << flist->GetID());

  // don't create if doesn't exist for this list
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(flist->GetID(), 0);

  if (!seedWidget)
    {
    vtkErrorMacro("SetSeedWidgetMaterialProperties: can't update display properties, this list " << flist->GetID() << " doesn't have a seed widget");
    return;
    }

  seedWidget->SetMaterialProperties(flist->GetOpacity(),
                                    flist->GetAmbient(),
                                    flist->GetDiffuse(),
                                    flist->GetSpecular(),
                                    flist->GetPower());
  return;
      
  if (!seedWidget->GetProperty())
    {
    vtkErrorMacro("SetSeedWidgetMaterialProperties: can't update display properties, this list " << flist->GetID() << " has a seed widget with no display property");
    return;
    }
 
  seedWidget->GetProperty()->SetOpacity(flist->GetOpacity());
  seedWidget->GetProperty()->SetAmbient(flist->GetAmbient());
  seedWidget->GetProperty()->SetDiffuse(flist->GetDiffuse());
  seedWidget->GetProperty()->SetSpecular(flist->GetSpecular());
  seedWidget->GetProperty()->SetSpecularPower(flist->GetPower());
}

//---------------------------------------------------------------------------
vtkCamera *vtkSlicerFiducialListWidget::GetActiveCamera()
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
void vtkSlicerFiducialListWidget::UpdateFiducialsCamera()
{
  vtkCamera *cam = NULL;
  cam = this->GetActiveCamera();
  
  if (cam == NULL)
    {
    vtkDebugMacro("UpdateFiducialsCamera: unable to get active camera");
    return;
    }

  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
    {
    iter->second->SetCamera(cam);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateSeedWidgetInteractors()
{
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
      this->GetViewerWidget()->GetMainViewer() == NULL ||
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
    {
    vtkDebugMacro("UpdateSeedWidgetInteractors: viewer widget or main viewer are null, resetting to null");
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
      // now update the visibility for the list
      vtkMRMLFiducialListNode *flist = NULL;
      if (this->GetMRMLScene())
        {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
          {
          flist = vtkMRMLFiducialListNode::SafeDownCast(node);
          }
        }
      if (flist != NULL)
        {
        // this causes a crash if start up with a mrml scene and try to enable
        // the widgets
        this->Update3DWidgetVisibility(flist);
        }
      }
    }
}

//---------------------------------------------------------------------------
/*
std::string vtkSlicerFiducialListWidget::GetFiducialNodeID (const char *actorid, int &index)
{
  // take the index off the actor id to get the fiducial node's id
  std::string actorString = actorid;
  std::stringstream ss;
  std::string sid;
  ss << actorid;
  ss >> sid;
  ss >> index;
  return sid;
}
*/

//----------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetViewerWidget(
  vtkSlicerViewerWidget *viewerWidget)
{
  if (this->ViewerWidget != NULL)
    {
    // TODO: figure out if this is necessary
    this->RemoveSeedWidgets();
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

  vtkDebugMacro("SetViewerWidget: Updating any seed widget interactors");
  this->UpdateSeedWidgetInteractors();
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetInteractorStyle( vtkSlicerViewerInteractorStyle *interactorStyle )
{
  // note: currently the GUICallbackCommand calls ProcessWidgetEvents
  // remove observers
  if (this->InteractorStyle != NULL &&
      this->InteractorStyle->HasObserver(vtkSlicerViewerInteractorStyle::PickEvent, this->GUICallbackCommand) == 1)
    {
    this->InteractorStyle->RemoveObservers(vtkSlicerViewerInteractorStyle::PickEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  
  this->InteractorStyle = interactorStyle;

  // add observers
  if (this->InteractorStyle)
    {
    vtkDebugMacro("vtkSlicerFiducialWidget: Adding observer on interactor style");
    this->InteractorStyle->AddObserver(vtkSlicerViewerInteractorStyle::PickEvent, (vtkCommand *)this->GUICallbackCommand);
    }
}

/*
//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemovePointWidgets()
{
  std::map< std::string, vtkPointWidget*>::iterator pointIter;
  for (pointIter = this->DisplayedPointWidgets.begin();
       pointIter != this->DisplayedPointWidgets.end();
       pointIter++)
    {
    if (pointIter->second != NULL)
      {
      vtkDebugMacro("Deleting displayed point widget at id " << pointIter->first.c_str());
      if (pointIter->second->GetInteractor())
        {
        pointIter->second->EnabledOff();
        pointIter->second->SetInteractor(NULL);
        }
      pointIter->second->Delete();
      pointIter->second = NULL;
      }
    //this->DisplayedPointWidgets.erase(pointIter->first);
    }
  this->DisplayedPointWidgets.clear();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemovePointWidget(const char *pointID)
{
  if (pointID == NULL)
    {
    return;
    }
  std::string stringID = std::string(pointID);
  
  std::map< std::string, vtkPointWidget *>::iterator pointIter;
  pointIter = this->DisplayedPointWidgets.find(stringID);
  if (pointIter != this->DisplayedPointWidgets.end())
    {
    vtkDebugMacro("RemovePointWidget: Deleting point widget at " << stringID.c_str());
    if (this->DisplayedPointWidgets[stringID]->GetInteractor())
      {
      this->DisplayedPointWidgets[stringID]->EnabledOff();
      this->DisplayedPointWidgets[stringID]->SetInteractor(NULL);
      }
    this->DisplayedPointWidgets[stringID]->Delete();
    this->DisplayedPointWidgets[stringID] = NULL;
    this->DisplayedPointWidgets.erase(stringID);
    }
  else
    {
    vtkDebugMacro("RemovePointWidget: couldn't find point widget for id " << pointID);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemovePointWidgetsForList(vtkMRMLFiducialListNode *flist)
{
  if (flist == NULL || 
      this->DisplayedPointWidgets.size() == 0)
    {
    return;
    }
  int numPoints = flist->GetNumberOfFiducials();
  for (int i = 0; i < numPoints; i++)
    {
    const char *pointID = flist->GetNthFiducialID(i);
    vtkDebugMacro("RemovePointWidgetsForList: removing point widget " << i << " at " << pointID);
    this->RemovePointWidget(pointID);
    }
}

*/

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::AddObserversToFiducialList(vtkMRMLFiducialListNode *flist)
{
  if (flist == NULL)
    {
    vtkWarningMacro("AddObserversToFiducialList: null input list!");
    return;
    }
  
  // watch for modified events from the list
  if (flist->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {
      //flist->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
  // watch for a transform modified event from the list
  if (flist->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {
    flist->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    }
  // observe display changes so can update the representation
  if (flist->HasObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {     
    flist->AddObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
    }
  // watch for indices being swapped, will have to update the widgets
  if (flist->HasObserver ( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {     
    flist->AddObserver ( vtkMRMLFiducialListNode::FiducialIndexModifiedEvent, this->MRMLCallbackCommand );
    }
    
  // watch for a fiducial point being modified
  if (flist->HasObserver ( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand ) == 0)
    {
    flist->AddObserver( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand );
    }
  
  // fiducial point removed?
  if (flist->HasObserver ( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand ) == 0)
    {
    flist->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
    }

  // watch for node added events on the fiducial list (triggered when add a new point)
  if (flist->HasObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand ) == 0)
    {
    flist->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
    }
}

/*
//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveExtraPointWidgets(vtkMRMLFiducialListNode *flistToCheck)
{
  if (flistToCheck == NULL ||
      this->DisplayedPointWidgets.size() == 0 ||
      this->MRMLScene == NULL)
    {
    return;
    }
  // for now, we can't reliably tell which fid list each display point widget
  // is associated with, so check all of them
  std::map< std::string, vtkPointWidget *>::iterator pointIter;
  std::vector<std::string> IDsToRemove;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  for (pointIter = this->DisplayedPointWidgets.begin();
       pointIter != this->DisplayedPointWidgets.end();
       pointIter++) 
    {
    bool inAList = false;
    // for each list
    for (int n=0; n<nnodes; n++)
      {
      vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
      if (flist->GetFiducialIndex(pointIter->first.c_str()) != -1)
        {
        inAList = true;
        break;
        }
      }
    if (!inAList)
      {
      vtkDebugMacro("RemoveExtraPointWidgets: displayed point widget with id " << pointIter->first << " is not in any list, flagging it for removal");
      IDsToRemove.push_back(pointIter->first);
      }
    else
      {
      vtkDebugMacro("RemovePointWidget: found displayed point widget with id " << pointIter->first);
      }
    } // end loop over all the displayed point widgets
  // now remove any that were flagged
  for (unsigned int i = 0; i < IDsToRemove.size(); i++)
    {
    vtkDebugMacro("RemoveExtraPointWidgets: removing " << IDsToRemove[i]);
    RemovePointWidget(IDsToRemove[i].c_str());
    }
}
*/















//---------------------------------------------------------------------------
vtkSlicerSeedWidgetClass * vtkSlicerFiducialListWidget::GetSeedWidget(const char *nodeID, int createFlag)
{
  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin();
       iter != this->SeedWidgets.end();
       iter++)
    {
    if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
      {
      return iter->second;
      }
    }
  // didn't find it, should I create it?
  if (createFlag)
    {
    vtkDebugMacro("No fiducial list widget found for fiducial listnode " << nodeID << ", have " << this->SeedWidgets.size() << " widgets, adding one for this one");
    this->AddSeedWidgetByID(nodeID);
    return this->GetSeedWidget(nodeID);
    }
  else
    {
    return NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::AddSeedWidgetByID(const char *nodeID)
{
  if (nodeID == NULL)
    {
    vtkErrorMacro("AddSeedWidgetByID: null fiducial list node id, can't add a seed widget without one");
    return;
    }
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(nodeID) ;
  if (node == NULL)
    {
    vtkErrorMacro("AddSeedWidgetByID: null fiducial list node, can't find a node in the mrml scene with id " << nodeID);
    return;
    }
  vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(node);
  if (flist != NULL)
    {
    this->AddSeedWidget(flist);
    }
  else
    {
    vtkErrorMacro("AddSeedWidgetByID: unable to get a fiducial list from the scene with id " << nodeID);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::AddSeedWidget(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (!fiducialListNode)
    {
    vtkErrorMacro("AddSeedWidget: null fiducial list node, can't add a seed widget without one");
    return;
    }
  if (this->GetSeedWidget(fiducialListNode->GetID(), 0) != NULL)
    {
    vtkDebugMacro("AddSeedWidget: Already have widgets for fiducial list node " << fiducialListNode->GetID());
    return;
    }

  vtkDebugMacro("AddSeedWidget: Creating new fiducial list widget class for list with id " << fiducialListNode->GetID());
  vtkSlicerSeedWidgetClass *c = vtkSlicerSeedWidgetClass::New();

  // now add call back
  vtkSlicerFiducialsSeedWidgetCallback *myCallback = vtkSlicerFiducialsSeedWidgetCallback::New();
  myCallback->FiducialListNode = fiducialListNode;
  myCallback->SeedWidgetClass = c;

   //
  //--- make seed widget's initial interaction behavior compliant  with mouse mode.
  //--- this may need to go after Update3DWidget...
  //
  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    vtkDebugMacro ( "AddSeedWidget: No interaction node in the scene." );
    return;
    }
  int placePersistence = interactionNode->GetPlaceModePersistence();
  int currentMode = interactionNode->GetCurrentInteractionMode();
  if ( currentMode == vtkMRMLInteractionNode::Place && placePersistence == 1 )
    {
    c->GetWidget()->ProcessEventsOff();
    }

  c->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
  c->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
  c->GetWidget()->AddObserver(vtkCommand::PlacePointEvent, myCallback);
  myCallback->Delete();

  this->SeedWidgets[fiducialListNode->GetID()] = c;
  
  // make sure we're observing the node events
  vtkDebugMacro("AddSeedWidget: adding observers to fid list node");
  this->AddObserversToFiducialList(fiducialListNode);
  //---
  //--- TEST {
  //---
  c->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  c->GetWidget()->AddObserver(vtkCommand::EndInteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  //---
  //--- } END TEST
  //---
  
  // set the interactor first
  this->Update3DWidgetVisibility(fiducialListNode);
  
  // bit of a hack: if the list was loaded from mrml it comes in with fids
  // already in it, no add fiducial event is captured, so add them one by one first
  if (fiducialListNode->GetNumberOfFiducials() != 0)
    {
    int numFids = fiducialListNode->GetNumberOfFiducials();
    vtkDebugMacro("AddSeedWidget: list already has " << numFids << ", so iterating over add seed");
    for (int f = 0; f < numFids; f++)
      {
      this->AddSeed(fiducialListNode, fiducialListNode->GetNthFiducialID(f));
      this->UpdateSeed(fiducialListNode, fiducialListNode->GetNthFiducialID(f));
      }
    }
  
  // now update it
  vtkDebugMacro("AddSeedWidget: calling update on the new widget");
  this->Update3DWidget(fiducialListNode);
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::AddSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID)
{
  if (!fiducialListNode)
    {
    vtkErrorMacro("AddSeed: null fiducial list node, can't add a seed for a point in it");
    return;
    }
 
  // don't create the widget if it's not already there
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID(), 0);
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
      vtkErrorMacro("AddSeed: unable to get the point ID for the added seed.");
      return;
      }
    // get the point index from the point id
    pointIndex = fiducialListNode->GetFiducialIndex(addedPointID);
    if (pointIndex == -1)
      {
      vtkErrorMacro("AddSeed: unable to find index for point id " << (addedPointID == NULL ? "null" : addedPointID) << " in list " << fiducialListNode->GetID());
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
      vtkErrorMacro("AddSeed: unable to get position for point at index " << pointIndex << ", from id " << addedPointID);
      }
    // and now update it
    //vtkDebugMacro("AddSeed: updating one seed " << addedPointID);
    //this->UpdateSeed(fiducialListNode, addedPointID);
// try updating the whole list
    // vtkDebugMacro("AddSeed: updating list " << fiducialListNode->GetID());
    //this->Update3DWidget(fiducialListNode);
  /*
    }
    else
    {
      // we aren't sure which point was added, just add a seed and update the
      // list
      vtkDebugMacro("AddSeed: no point id given, just add a seed and update the whole thing from the fid list node");
      seedWidget->AddSeed(NULL);
      this->Update3DWidget(fiducialListNode);
      }
  */
    }
  else
    {
    vtkErrorMacro("AddSeed: there wasn't a seed widget for the list " << fiducialListNode->GetID() << ", unable to add a seed to it");
    }
}
  
//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID)
{
  if (!fiducialListNode)
    {
    vtkErrorMacro("RemoveSeed: null input list!");
    return;
    }
  /*
  // get the point index from the point id
  // this is going to fail because the list was already updated so that the
  // fiducial in question is gone.
  int pointIndex = fiducialListNode->GetFiducialIndex(pointID);
  if (pointIndex == -1)
    {
    vtkErrorMacro("RemoveSeed: unable to find index for point id " << pointID << " in list " << fiducialListNode->GetID());
    return;
    }
  else
    {
    vtkDebugMacro("RemoveSeed: found point index " << pointIndex << " for point id " << pointID << " in list " << fiducialListNode->GetID() << ", removing that seed");
    }
  */
  // don't create the widget if it's not already there
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID(), 0);
  if (seedWidget != NULL)
    {
    //seedWidget->RemoveSeed(pointIndex);
    seedWidget->RemoveSeedByID(pointID);
    }
  else
    {
    vtkErrorMacro("RemoveSeed: couldn't find a seed widget for this list: " << fiducialListNode->GetID());
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveSeedWidget(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (!fiducialListNode)
    {
    vtkErrorMacro("RemoveSeedWidget: null input list!");
    return;
    }
  // don't create the widget if it's not already there
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID(), 0);
  if (seedWidget != NULL)
    {
    // temp: remove observers
    seedWidget->GetWidget()->RemoveObservers(vtkCommand::InteractionEvent);
    seedWidget->GetWidget()->RemoveObservers(vtkCommand::StartInteractionEvent);
    seedWidget->GetWidget()->RemoveObservers(vtkCommand::PlacePointEvent);
    seedWidget = NULL;
    this->SeedWidgets[fiducialListNode->GetID()]->Delete();
    // really truly erase it we mean it this time so it can't ever be found
    // again, since calling erase(id) lets it be found again later, and is a
    // problem when the scene is closed and new fids are added
    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    iter = this->SeedWidgets.find(fiducialListNode->GetID());
    if (iter != this->SeedWidgets.end())
      {
      this->SeedWidgets.erase(iter);
      }
    else
      {
      vtkDebugMacro("RemoveSeedWidget: unable to find the seed widget class for list " << fiducialListNode->GetID());
      }
    vtkDebugMacro("RemoveSeedWidget: removed seed widget for list " << fiducialListNode->GetID());
    }
  else
    {
    vtkDebugMacro("RemoveSeedWidget: couldn't find a seed widget for this list: " << fiducialListNode->GetID());
    }
  
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveSeedWidgets()
{
  if (this->SeedWidgets.size() == 0)
    {
    // none to remove
    return;
    }
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  vtkDebugMacro("RemoveSeedWidgets: have " << nnodes << " fiducial list nodes in the scene, " << this->SeedWidgets.size() << " widgets defined already");

  if (nnodes == 0)
    {
    // the scene was closed, all the nodes are gone, so do this w/o reference
    // to the nodes
    vtkDebugMacro("RemoveSeedWidgets: no fiducials in scene, removing seeds w/o reference to nodes");
    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    for (iter = this->SeedWidgets.end();
         iter != this->SeedWidgets.end();
         iter++)
      {
      vtkDebugMacro("RemoveSeeds: deleting and erasing " << iter->first);
      iter->second->Delete();
      this->SeedWidgets.erase(iter);
      }
    this->SeedWidgets.clear();
    }
  else
    {
    for (int n=0; n<nnodes; n++)
      {
      vtkMRMLFiducialListNode *rnode = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
      if (rnode)
        {
        this->RemoveSeedWidget(rnode);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetsFromMRML()
{
  vtkErrorMacro("Update3DWidgetsFromMRML: deprecated, returning.");
  return;
  /*
  if (!this->MRMLScene)
    {
    vtkDebugMacro("Update3DWidgetsFromMRML: no mrml scene from which to update!");
    return;
    }

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  vtkDebugMacro("Update3DWidgetsFromMRML: have " << nnodes << " fiducial list nodes in the scene, " << this->SeedWidgets.size() << " widgets defined already");

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *rnode = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    if (rnode)
      {
      // this call will create one if it's missing
      // this->Update3DWidget(rnode);
      // let go of the pointer
      rnode = NULL;
      }
    }

  // now have a widget for each node, check that don't have too many widgets
  if ((int)(this->SeedWidgets.size()) != nnodes)
    {
    vtkDebugMacro("Update3DWidgetsFromMRML: after adding widgets for scene nodes, have " << this->SeedWidgets.size() << " instead of " << nnodes);
    // find ones that aren't in the scene, be careful using an iterator because calling erase gets it messed up
    //int numWidgets = this->SeedWidgets.size();
    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    std::vector<std::string> idsToDelete;
    for (iter = this->SeedWidgets.begin();
         iter != this->SeedWidgets.end();
         iter++)
      {
      if (this->MRMLScene->GetNodeByID(iter->first.c_str()) == NULL)
        {
        vtkDebugMacro("Update3DWidgetsFromMRML: found an extra widget with id " << iter->first.c_str());
        // add it to a list and do delete and erase in a second round
        idsToDelete.push_back(iter->first);
        }
      }
    for (int i = 0; i < (int)(idsToDelete.size()); i++)
      {
      std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator delIter;
      delIter = this->SeedWidgets.find(idsToDelete[i]);
      if (delIter != this->SeedWidgets.end())
        {
        // can't call this->RemoveSeedWidget because we don't have a node!
        this->SeedWidgets[delIter->first.c_str()]->Delete();
        this->SeedWidgets.erase(delIter->first.c_str());
        }
      }
    }
  // request a render
  this->RequestRender();
  */
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetLock(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetLock: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget != NULL)
    {
    vtkDebugMacro("Update3DWidgetLock: this is probably going to make the seeds invisible, setting seeds locked with " << fiducialListNode->GetLocked());
    seedWidget->SetSeedsLocked(fiducialListNode->GetLocked());
    }
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetVisibility(vtkMRMLFiducialListNode *fiducialListNode)
{

  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetVisibility: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL ||
      seedWidget->GetWidget() == NULL)
    {
    vtkDebugMacro("Update3DWidgetVisibility: no widget to update!");
    return;
    }
  if (seedWidget->GetWidget()->GetInteractor() == NULL)
    {
    vtkDebugMacro("Seed widget doens't have an interactor.");
    if (this->GetViewerWidget() &&
        this->GetViewerWidget()->GetMainViewer() &&
        this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor())
      {
      vtkDebugMacro("Update3DWidgetVisibility: setting a widget's interactor, turning it on");
      seedWidget->GetWidget()->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
      }
    else
      {
      vtkWarningMacro("Update3DWidgetVisibility: no interactor found! Seed widget won't work until this is set");
      seedWidget->GetWidget()->SetInteractor(NULL);
      }
    }
  if ( fiducialListNode->GetVisibility() )
    {
    if (seedWidget->GetWidget()->GetInteractor())
      {
      vtkDebugMacro("Update3DWidgetVisibility: turning it on");
      seedWidget->GetWidget()->On();
      // First, get out out of the mode where we are interactively defining seeds
      seedWidget->GetWidget()->CompleteInteraction();

      // now iterate through the seeds to set visibility
      int numFids = fiducialListNode->GetNumberOfFiducials();
      for (int n = 0; n < numFids; n++)
        {
        vtkDebugMacro("Update3DWidgetVisibility: fiducial " << n << " setting indiv visib")
          seedWidget->SetNthSeedVisibility(n, fiducialListNode->GetNthFiducialVisibility(n));
        }
      // at this point the widget is in click to place mode, fool it into
      // thinking that we're done for now
      /*
      int *max;
      max = seedWidget->GetWidget()->GetInteractor()->GetSize();
      double x, y;
      x = (double)max[0];
      y = (double)max[1];
      seedWidget->GetWidget()->GetInteractor()->SetEventPositionFlipY(x*0.25,y*0.25);
      seedWidget->GetWidget()->GetInteractor()->InvokeEvent(vtkCommand::LeftButtonPressEvent);
      seedWidget->GetWidget()->GetInteractor()->InvokeEvent(vtkCommand::LeftButtonReleaseEvent);
      seedWidget->GetWidget()->GetInteractor()->InvokeEvent(vtkCommand::RightButtonPressEvent);
      seedWidget->GetWidget()->GetInteractor()->InvokeEvent(vtkCommand::RightButtonReleaseEvent);
      // now have an extra seed widget, remove it
      vtkWarningMacro("Update3DWidgetVisibility: Tried making a single fake seed, seed widget size = " << seedWidget->GetRepresentation()->GetNumberOfSeeds() << ", flist has " << fiducialListNode->GetNumberOfFiducials() << ", removing seeds from the seed widget");
      fiducialListNode->RemoveAllFiducials();
      vtkWarningMacro("Update3DWidgetVisibility: after removing all fids, seed widget size = " << seedWidget->GetRepresentation()->GetNumberOfSeeds() << ", flist has " << fiducialListNode->GetNumberOfFiducials());
      seedWidget->RemoveAllSeeds();
      vtkWarningMacro("Update3DWidgetVisibility: after removing fake seeds,
      seed widget size = " <<
      seedWidget->GetRepresentation()->GetNumberOfSeeds() << ", flist has " <<
      fiducialListNode->GetNumberOfFiducials());
      */
      }
    }
  else
    {
    vtkDebugMacro("UpdateWidgetVisibility: fiducial list widget off");
    seedWidget->GetWidget()->Off();
    seedWidget->GetWidget()->ProcessEventsOff();
    }
  /*
  // now iterate through the seeds to set visibility
  int numFids = fiducialListNode->GetNumberOfFiducials();
  for (int n = 0; n < numFids; n++)
    {
    vtkWarningMacro("Update3DWidgetVisibility: fiducial " << n << " setting indiv visib")
    seedWidget->SetNthSeedVisibility(n, fiducialListNode->GetNthFiducialVisibility(n));
    //seedWidget->SetNthLabelTextVisibility(n, fiducialListNode->GetNthFiducialVisibility(n));
    }
  */
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetSelected(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetSelected: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkWarningMacro("Update3DWidgetSelected: no seed widget to update! Searched for id " << fiducialListNode->GetID());
    return;
    }
  int numFids = fiducialListNode->GetNumberOfFiducials();
  for (int n = 0; n < numFids; n++)
    {
    vtkDebugMacro("Update3DWidgetSelected: setting " << n << "th seed selected to " << fiducialListNode->GetNthFiducialSelected(n));
    seedWidget->SetNthSeedSelected(n, fiducialListNode->GetNthFiducialSelected(n));
    }
  
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetGlyphType(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetGlyphType: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkDebugMacro("Update3DWidgetGlyphType: no seed widget to update! Checked for id " << fiducialListNode->GetID());
    return;
    }

  int glyphType = fiducialListNode->GetGlyphType();
  vtkDebugMacro("Update3DWidgetGlyphType: using glyph type " << glyphType);
  int retval = -1;
  if (glyphType == vtkMRMLFiducialListNode::Diamond3D)
    {
    retval = seedWidget->SetGlyphToDiamond3D();
    }
  else if (glyphType == vtkMRMLFiducialListNode::Sphere3D)
    {
    retval = seedWidget->SetGlyphToSphere();
    }
  else
    {
    // set it by number as it's a 2d one
    vtkDebugMacro("Update3DWidgetGlyphType: calling seed widget set glyph to 2d with type " << glyphType);
    retval = seedWidget->SetGlyphTo2D(glyphType);
    }
  if (retval == 1)
    {
    // the type changed
    if (fiducialListNode->GetNumberOfFiducials() > 0)
      {
      // delete the current ones so they're reset
      vtkDebugMacro("Update3DWidgetGlyphType: changed the glyph type, removing all seeds then updating");
      seedWidget->RemoveAllSeeds();
      // need to update as this resets the handles
      this->Update3DWidget(fiducialListNode);
      }
    }
  // request a render, done in update
  // this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetText(vtkMRMLFiducialListNode *fiducialListNode)
{
   if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetText: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkDebugMacro("Update3DWidgetText: no seed widget to update! Checked for id " << fiducialListNode->GetID());
    return;
    }
  int numFids = fiducialListNode->GetNumberOfFiducials();
  for (int i = 0; i < numFids; i++)
    {
    vtkDebugMacro("Update3DWidgetText updating " << i << "th label text");
    seedWidget->SetNthLabelText(i, fiducialListNode->GetNthFiducialLabelText(i));
    }

  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetPositions(vtkMRMLFiducialListNode *fiducialListNode)
{
   if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetPositions: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkWarningMacro("Update3DWidgetPositions: no seed widget to update! Checked for id " << fiducialListNode->GetID());
    return;
    }
  
  // loop over the fids
  int numFids = fiducialListNode->GetNumberOfFiducials();
  for (int f = 0; f < numFids; f++)
    {
    double worldxyz[4];
    if (fiducialListNode->GetNthFiducialXYZWorld(f, worldxyz))
      {
      seedWidget->SetNthSeedPosition(f, worldxyz);
      }
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetColour(vtkMRMLFiducialListNode *fiducialListNode)
{
   if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetColour: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkDebugMacro("Update3DWidgetColour: no seed widget to update! Checked for id " << fiducialListNode->GetID());
    return;
    }

  // check if there's a change
  double *widgetColour = seedWidget->GetListColor();
  double *widgetSelectedColour = seedWidget->GetListSelectedColor();
  double *listColour = fiducialListNode->GetColor();
  double *listSelectedColour = fiducialListNode->GetSelectedColor();
  bool updateSelectedState = false;
  if (widgetColour[0] != listColour[0] ||
      widgetColour[1] != listColour[1] ||
      widgetColour[2] != listColour[2])
    {
    seedWidget->SetListColor(listColour);
    updateSelectedState = true;
    }
  if (widgetSelectedColour[0] != listSelectedColour[0] ||
      widgetSelectedColour[1] != listSelectedColour[1] ||
      widgetSelectedColour[2] != listSelectedColour[2])
    {
    seedWidget->SetListSelectedColor(listSelectedColour);
    updateSelectedState = true;
    }
  if (updateSelectedState)
    {
    // use the new colours when set seeds selected
    vtkDebugMacro("Update3DWidgetColour: had a new colour, updating selected on the fids");
    int numfids = fiducialListNode->GetNumberOfFiducials();
    for (int n = 0; n < numfids; n++)
      {
      seedWidget->SetNthSeedSelected(n, fiducialListNode->GetNthFiducialSelected(n));
      }
    }
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetScale(vtkMRMLFiducialListNode *fiducialListNode)
{
   if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetScale: no fiducial list from which to update!");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (seedWidget == NULL)
    {
    vtkDebugMacro("Update3DWidgetScale: no seed widget to update! Checked for id " << fiducialListNode->GetID());
    return;
    }

  vtkDebugMacro("Update3DWidgetScale: setting text scale and glyph scale for list " << fiducialListNode->GetID());

  // glyph scale
  seedWidget->SetGlyphScale(fiducialListNode->GetSymbolScale());
  // text scale
  seedWidget->SetTextScale(fiducialListNode->GetTextScale());
  
  // request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidgetDisplayProperties(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidgetDisplayProperties: passed in fiducial list node is null, returning");
    return;
    }
  //if (this->Updating3DWidget)
  //  {
  //  vtkDebugMacro("Already updating 3d widget");
  //  return;
  //  }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (!seedWidget)
    {
    vtkDebugMacro("Update3DWidgetDisplayProperties: No fiducial list widget found for fiducial listnode " << fiducialListNode->GetID());
    vtkErrorMacro("Error adding a new fiducial list widget for fiducial list node " << fiducialListNode->GetID());
    this->Updating3DWidget = 0;
    return;
    }
  if (seedWidget->GetWidget() == NULL)
    {
    vtkErrorMacro("Update3DWidgetDisplayProperties: fiducial list widget is null");
    return;
    }

  vtkDebugMacro("Update3DWidgetDisplayProperties: updating list visibility, lock, color, scale, glyph type, disp properties");
  
  // this is a subset of methods called when the fiducial list invokes a
  // vtkMRMLFiducialListNode::DisplayModifiedEvent

  // visibility
  this->Update3DWidgetVisibility(fiducialListNode);

  // lock
  this->Update3DWidgetLock(fiducialListNode);
  
  // colour, selected and not
  this->Update3DWidgetColour(fiducialListNode);
  
  // selected - this goes on the individual fids, but after change colour,
  // should do it
//  this->Update3DWidgetSelected(fiducialListNode);
  
 
  // glyph type
  this->Update3DWidgetGlyphType(fiducialListNode);

  // scale, both symbol and text
  this->Update3DWidgetScale(fiducialListNode);
  
  // material properties
  this->SetSeedWidgetMaterialProperties(fiducialListNode);
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Update3DWidget(vtkMRMLFiducialListNode *fiducialListNode)
{
  if (fiducialListNode == NULL)
    {
    vtkDebugMacro("Update3DWidget: passed in fiducial list node is null, returning");
    return;
    }
  if (this->Updating3DWidget)
    {
    vtkWarningMacro("Update3DWidget: Already updating 3d widget");
    return;
    }
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(fiducialListNode->GetID());
  if (!seedWidget)
    {
    vtkDebugMacro("No fiducial list widget found for fiducial listnode " << fiducialListNode->GetID());
    vtkErrorMacro("Error adding a new fiducial list widget for fiducial list node " << fiducialListNode->GetID());
    this->Updating3DWidget = 0;
    return;
    }
  if (seedWidget->GetWidget() == NULL)
    {
    vtkErrorMacro("Update3D widget: fiducial list widget is null");
    return;
    }

  this->Updating3DWidget = 1;

  vtkDebugMacro("Update3DWidget from list " << fiducialListNode->GetID() <<  " calling update display properties for widget, then updating seeds");

//  vtkDebugMacro("Update3DWidget: adding observers to the list");
  // for now, dont' do this, it's done when add a seed widget
//  this->AddObserversToFiducialList(fiducialListNode);

  //feb 19/20, do everything in update seeed
  //this->Update3DWidgetDisplayProperties(fiducialListNode);
  // except for the glyph type
  this->Update3DWidgetGlyphType(fiducialListNode);
  
  // for things that need to be set for each fiducial point
  int numFids = fiducialListNode->GetNumberOfFiducials();
  for (int i = 0; i < numFids; i++)
    {
    this->UpdateSeed(fiducialListNode, fiducialListNode->GetNthFiducialID(i));
    }

  // request a render
  this->RequestRender();
  
  // reset the flag
  this->Updating3DWidget = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Swap(vtkMRMLFiducialListNode *flist, int first, int second)
{
  if (flist == NULL)
    {
    vtkErrorMacro("Swap: input fiducial list is null!");
    return;
    }
  // these calls will get the new ids of the already swapped fids
  const char *firstWasSecondID = flist->GetNthFiducialID(first);
  const char *secondWasFirstID = flist->GetNthFiducialID(second);

  vtkDebugMacro("Swap: first = " << first << ", first id = " << firstWasSecondID << ", second = " << second << ", second id = " << secondWasFirstID);

  // swap the seed widget indices around
  vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(flist->GetID());
  if (!seedWidget)
    {
    vtkErrorMacro("Swap: Error swapping " << first << " and " << second << " fids in list " << flist->GetName());
    return;
    }
  vtkDebugMacro("Swap: calling SwapIndexIDs with  " << first << " and " << second);
  seedWidget->SwapIndexIDs(first, second);
  // update the seeds
  vtkDebugMacro("Swap: updating seed with id " << firstWasSecondID);
  this->UpdateSeed(flist, firstWasSecondID);
  vtkDebugMacro("Swap: updating seed with id " << secondWasFirstID);
  this->UpdateSeed(flist, secondWasFirstID);
  
  return;
  
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateViewNode()
{
  // obsolete as per comment in vtkSlicerViewerWidget::UpdateViewNode
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::ModifyAllWidgetLock(int lockFlag)
{
  std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
  for (iter = this->SeedWidgets.begin(); iter !=  this->SeedWidgets.end(); iter++)
    {
    if (iter->second->GetWidget())
      {
      // get the fiducial list node associated with this widget
      vtkMRMLNode *node = NULL;
      node = this->MRMLScene->GetNodeByID(iter->first.c_str());
      int thisListLock = lockFlag;
      if (node)
        {
        vtkMRMLFiducialListNode *fidListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
        if (fidListNode)
          {
          if (fidListNode->GetLocked() == 1 && lockFlag == 0)
            {
            thisListLock = 1;
            }
          }
        }
      iter->second->GetWidget()->SetProcessEvents(!thisListLock);
      if ( thisListLock )
        {
        }
      }
    }
}
