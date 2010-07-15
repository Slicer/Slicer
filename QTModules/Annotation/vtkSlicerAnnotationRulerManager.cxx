#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationRulerManager.h"

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

#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"

#include "vtkSeedWidget.h"
#include "vtkSeedRepresentation.h"
//#include "vtkSlicerSeedWidgetClass.h"


class vtkSlicerAnnotationRulerManagerCallback : public vtkCommand
{
public:
    static vtkSlicerAnnotationRulerManagerCallback *New()
    { return new vtkSlicerAnnotationRulerManagerCallback; }
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
            vtkIndent ind;
            //this->RulerNode->PrintSelf(cout,ind);
        }

    }
    vtkSlicerAnnotationRulerManagerCallback():RulerNode(0) {}
    vtkMRMLAnnotationRulerNode *RulerNode;
};

//---------------------------------------------------------------------------
//vtkStandardNewMacro (vtkSlicerAnnotationRulerManager );
//vtkCxxRevisionMacro ( vtkSlicerAnnotationRulerManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerAnnotationRulerManager::vtkSlicerAnnotationRulerManager ( )
{
  this->RulerNodeID = NULL;
  //this->ViewerWidget = NULL;

  //this->SetRulerNodeID(NULL);
  
  this->Updating3DRuler = 0;

}


//---------------------------------------------------------------------------
vtkSlicerAnnotationRulerManager::~vtkSlicerAnnotationRulerManager ( )
{
  this->RemoveMRMLObservers();

  // gui elements

  // 3d widgets
  //this->SetRulerNodeID(NULL);
  //this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationRulerManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    //this->vtkObject::PrintSelf ( os, indent );

    //os << indent << "vtkMeasurementsRulerWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
    std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
    for (iter = this->DistanceWidgets.begin(); iter !=  this->DistanceWidgets.end(); iter++)
      {
      os << indent << "Distance Widget: " << iter->first.c_str() << "\n";
      iter->second->GetWidget()->PrintSelf(os, indent);
      }
}*/


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationRulerManager::SetRulerNodeID ( char *id )
{
    if (this->GetRulerNodeID() != NULL &&
        id != NULL &&
        strcmp(id,this->GetRulerNodeID()) == 0)
    {
        //vtkDebugMacro("SetRulerNodeID: no change in id, not doing anything for now: " << id << endl);
        return;
    }

    // set the id properly - see the vtkSetStringMacro
    this->RulerNodeID = id;

    if (id == NULL)
    {
        //vtkDebugMacro("SetRulerNodeID: NULL input id, clearing GUI and returning.\n");
        this->UpdateWidget(NULL);
        return;
    }

    // get the new node
    vtkMRMLAnnotationRulerNode *newRuler = vtkMRMLAnnotationRulerNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetRulerNodeID()));
    // set up observers on the new node - now done in adddistancewidget
    if (newRuler != NULL)
    {
        // set up the GUI
        this->UpdateWidget(newRuler);
    }
    else
    {
        //vtkDebugMacro ("ERROR: unable to get the mrml ruler node to observe!\n");
    }
}*/


//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtStartInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkSlicerAnnotationRulerManager::UpdateInteractionModeAtStartInteraction()
{
/*
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }
  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    //vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
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
*/
}



//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtEndInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkSlicerAnnotationRulerManager::UpdateInteractionModeAtEndInteraction()
{
  /*if ( this->MRMLScene == NULL )
    {
    //vtkErrorMacro ( "UpdateInteractionModeAtStartInteraction: NULL scene.");
    return;
    }

  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass( 0, "vtkMRMLInteractionNode"));
  if ( interactionNode == NULL )
    {
    ////vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
    return;
    }

  int pickPersistence = interactionNode->GetPickModePersistence();
  int placePersistence = interactionNode->GetPlaceModePersistence ();
  if ( pickPersistence == 0 && placePersistence == 0 )
    {
    interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
    vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

    // the scene was closed, don't get node removed events so clear up here
    if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
        // the lists are already gone from the scene, so need to clear out all the
        // widget properties, can't call remove with a node
        this->RemoveDistanceWidgets();
        return;
    }
  
    // first check to see if there was a ruler list node deleted
    /*if (callScene != NULL && callScene == this->MRMLScene && event == vtkMRMLScene::NodeRemovedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
        // check to see if it was the current node that was deleted
        if (callData != NULL)
        {
            vtkMRMLAnnotationRulerNode *delNode = reinterpret_cast<vtkMRMLAnnotationRulerNode*>(callData);
            if (delNode != NULL && delNode->IsA("vtkMRMLAnnotationRulerNode"))
            {
                //vtkDebugMacro("A ruler node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
                this->RemoveDistanceWidget(delNode);
                this->RemoveTextDisplayNodeID(delNode);
                this->RemovePointDisplayNodeID(delNode);
                this->RemoveLineDisplayNodeID(delNode);
            }
        }
    }

    vtkMRMLAnnotationRulerNode *node = vtkMRMLAnnotationRulerNode::SafeDownCast(caller);
    vtkMRMLAnnotationRulerNode *activeRulerNode = (vtkMRMLAnnotationRulerNode *)this->MRMLScene->GetNodeByID(this->GetRulerNodeID());

    // check for a node added to the scene event
    if (callScene != NULL && callScene == this->MRMLScene && callData != NULL && event == vtkMRMLScene::NodeAddedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");

        // check to see if it was a ruler node    
        vtkMRMLAnnotationRulerNode *addNode = reinterpret_cast<vtkMRMLAnnotationRulerNode*>(callData);
        if (addNode != NULL && addNode->IsA("vtkMRMLAnnotationRulerNode"))
        { 
            //vtkDebugMacro("Got a node added event with a ruler node " << addNode->GetID());
            // is it currently the active one?
            if (addNode == activeRulerNode)
            {
                //vtkDebugMacro("Calling Update widget to set up the ui since this is the active one");
                this->UpdateWidget(addNode);
            }
            // if it's not the current one, just update the 3d widget
            //vtkDebugMacro("Calling Update 3D widget to set up a new distance widget");
            this->Update3DWidget(addNode);
            // for now, since missing some of the add calls when open a scene, make sure we're current with the scene
            this->Update3DWidgetsFromMRML();
            return;
        }
    }

    // check for a ruler node added event
    if (event == vtkMRMLAnnotationRulerNode::RulerNodeAddedEvent)
    {
        // check to see if it was a ruler node    
        if (node != NULL && node->IsA("vtkMRMLAnnotationRulerNode"))
        {
            //vtkDebugMacro("Got a node added event with a ruler node " << node->GetID());
            // is it currently the active one?
            if (node == activeRulerNode)
            {
                //vtkDebugMacro("Calling Update widget to set up the ui since this is the active one");
                this->UpdateWidget(node);
            }
            // if it's not the current one, just update the 3d widget
            //vtkDebugMacro("Calling Update 3D widget to set up a new distance widget");
            this->Update3DWidget(node);
            this->UpdateDisplayNode(node);
            // for now, since missing some of the add calls when open a scene, make sure we're current with the scene
            this->Update3DWidgetsFromMRML();
            return;
        }
    }

    if (vtkMRMLAnnotationTextDisplayNode::SafeDownCast(caller) != NULL && event == vtkCommand::ModifiedEvent)
    {
        // it's a modified event on a text display node
        this->UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode::SafeDownCast(caller));
    }
    else if (vtkMRMLAnnotationPointDisplayNode::SafeDownCast(caller) != NULL && event == vtkCommand::ModifiedEvent)
    {
        // it's a modified event on a point display node
        this->UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode::SafeDownCast(caller));
    }
    else if (vtkMRMLAnnotationLineDisplayNode::SafeDownCast(caller) != NULL && event == vtkCommand::ModifiedEvent)
    {
        // it's a modified event on a line display node
        this->UpdateLineDisplayProperty(vtkMRMLAnnotationLineDisplayNode::SafeDownCast(caller));
    }
    else if (node == activeRulerNode)
    {
        if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeAddedEvent 
            || event == vtkMRMLScene::NodeRemovedEvent)
        {
            //vtkDebugMacro("Modified or node added or removed event on the ruler node.\n");
            if (node == NULL)
            {
                //vtkDebugMacro("\tBUT: the node is null\n");
                // check to see if the id used to get the node is not null, if it's
                // a valid string, means that the node was deleted
                if (this->GetRulerNodeID() != NULL)
                {
                    this->SetRulerNodeID(NULL);
                }
                return;
            }
            //vtkDebugMacro("ProcessMRMLEvents: \t\tUpdating the GUI\n");
            // update the gui
            UpdateWidget(activeRulerNode);
            return;
        }
    } // end of events on the active ruler node
    else if (node != NULL && event == vtkCommand::ModifiedEvent)
    {
        // it's a modified event on a ruler node that's not being displayed in the
        // 2d gui, so update the 3d widget
        this->Update3DWidget(node);
    }
    // Process event when one of the two points of the ruler has coordinate change 
    else if (node != NULL && event == vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent)
    {
        // it's a modified event on a ruler node that's not being displayed in the
        // 2d gui, so update the 3d widget
        this->Updating3DRuler = 0;
        this->Update3DWidget(node);
        this->UpdateDistanceMeasurement(node);
    }  
    else if (node != NULL && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
        //vtkDebugMacro("Got transform modified event on node " << node->GetID());
        this->Update3DWidget(node);
    }
     else if (node != NULL && event == vtkMRMLAnnotationNode::LockModifiedEvent)
     {
          this->UpdateLockUnlock(node);
     }
     */
}


//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateWidget(vtkMRMLAnnotationRulerNode *activeRulerNode)
{ 
    //vtkDebugMacro("UpdateWidget: active ruler node is " << (activeRulerNode == NULL ? "null" : activeRulerNode->GetName()));

    // if the passed node is null, clear out the widget
    if (activeRulerNode == NULL)
    {
        // don't need to do anything yet, especially don't set the node selector to
        // null, as it causes a crash
        //vtkDebugMacro("UpdateWidget: The passed in node is null, returning.");
        return;
    }

    //vtkDebugMacro("UpdateWidget: updating the gui and 3d elements");

   
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::Update3DWidget(vtkMRMLAnnotationRulerNode *activeRulerNode)
{
    /*
    
    if (activeRulerNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in ruler node is null, returning");
        return;
    }

    if (this->Updating3DRuler)
    {
        //vtkDebugMacro("Already updating 3d widget");
        return;
    }

    vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(activeRulerNode->GetID());
    if (!distanceWidget)
    {
        cout << "No distance widget found, adding a distance widget for this one" << endl;
        this->AddDistanceWidget(activeRulerNode);
        distanceWidget = this->GetDistanceWidget(activeRulerNode->GetID());
        if (!distanceWidget)
        {
            vtkErrorMacro("Error adding a new distance widget for ruler node " << activeRulerNode->GetID());
            this->Updating3DRuler = 0;
            return;
        }
    }
    if (distanceWidget->GetWidget() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance widget is null");
        return;
    }
    if (distanceWidget->GetWidget()->GetRepresentation() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance representation is null");
        return;
    }

    this->Updating3DRuler = 1;

    // visibility
    this->Update3DWidgetVisibility(activeRulerNode);

    vtkLineRepresentation *lineRep  = vtkLineRepresentation::SafeDownCast(distanceWidget->GetWidget()->GetRepresentation());
    if (!lineRep)
    {
        vtkErrorMacro("LineRepresenation not defined for distance widget");
        return;
    }

    // set up call back
    // temp: remove observers
    distanceWidget->GetWidget()->RemoveObservers(vtkCommand::InteractionEvent);
    distanceWidget->GetWidget()->RemoveObservers(vtkCommand::StartInteractionEvent);

    // now add call back
    vtkSlicerAnnotationRulerManagerCallback *myCallback = vtkSlicerAnnotationRulerManagerCallback::New();
    myCallback->RulerNode = activeRulerNode;
    distanceWidget->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
    distanceWidget->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
    myCallback->Delete();

    if (lineRep)
    {
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
            lineRep->SetPoint1WorldPosition(worldp);
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
            lineRep->SetPoint2WorldPosition(worldp);
        }
        tnode = NULL;
        transformToWorld->Delete();
        transformToWorld = NULL;

        // distance annotation
        lineRep->SetDistanceAnnotationVisibility(activeRulerNode->GetDistanceAnnotationVisibility());
        lineRep->SetDistanceAnnotationFormat(activeRulerNode->GetDistanceAnnotationFormat());

        this->UpdateTextDisplayProperty(activeRulerNode->GetAnnotationTextDisplayNode());
        this->UpdatePointDisplayProperty(activeRulerNode->GetAnnotationPointDisplayNode());
        this->UpdateLineDisplayProperty(activeRulerNode->GetAnnotationLineDisplayNode());
    }
    else {
        vtkErrorMacro("LineRepresentation is not fully defined");
        return; 
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
            //vtkDebugMacro("Update3DWidget: Have a slice node, id = " << slice->GetID());
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
                lineRep->GetPoint1Representation()->ConstrainedOff();
                lineRep->GetPoint1Representation()->SetPointPlacer(distanceWidget->GetModel1PointPlacer());
            }
        }
        else
        {
            distanceWidget->GetModel1PointPlacer()->RemoveAllProps();
            lineRep->GetPoint1Representation()->SetPointPlacer(NULL);
        }
    }
    else
    {
        // make sure it's not constrained
        distanceWidget->GetModel1PointPlacer()->RemoveAllProps();
        lineRep->GetPoint1Representation()->SetPointPlacer(NULL);
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
            //vtkDebugMacro("Update3DWidget: Have a slice node, id = " << slice->GetID());
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
                lineRep->GetPoint2Representation()->ConstrainedOff();
                lineRep->GetPoint2Representation()->SetPointPlacer(distanceWidget->GetModel2PointPlacer());
            }
        }
        else
        {
            distanceWidget->GetModel2PointPlacer()->RemoveAllProps();
            lineRep->GetPoint2Representation()->SetPointPlacer(NULL);
        }
    }
    else
    {
        // make sure it's not constrained
        distanceWidget->GetModel2PointPlacer()->RemoveAllProps();
        lineRep->GetPoint2Representation()->SetPointPlacer(NULL);
    }


    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
    // reset the flag
    this->Updating3DRuler = 0;
    */
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateDisplayNode(vtkMRMLAnnotationRulerNode* rulerNode)
{
    cout << "vtkSlicerAnnotationRulerManager::UpdateDisplayNode Start" << endl;
/*
    if (rulerNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in ruler node is null, returning");
        return;
    }

    std::string textDisplayNodeID = this->GetTextDisplayNodeID(rulerNode->GetID());
    if (textDisplayNodeID.empty())
    {
        cout << "No text display node found, adding a new text display node for this one" << endl;
        this->AddTextDiaplayNodeID(rulerNode);
        textDisplayNodeID = this->GetTextDisplayNodeID(rulerNode->GetID());
        if (textDisplayNodeID.empty())
        {
            vtkErrorMacro("Error adding a new text display node for ruler node " << rulerNode->GetID());
            return;
        }
    }

    std::string pointDisplayNodeID = this->GetPointDisplayNodeID(rulerNode->GetID());
    if (pointDisplayNodeID.empty())
    {
        cout << "No point display node found, adding a new text display node for this one" << endl;
        this->AddPointDiaplayNodeID(rulerNode);
        pointDisplayNodeID = this->GetPointDisplayNodeID(rulerNode->GetID());
        if (pointDisplayNodeID.empty())
        {
            vtkErrorMacro("Error adding a new point display node for ruler node " << rulerNode->GetID());
            return;
        }
    }

    std::string lineDisplayNodeID = this->GetLineDisplayNodeID(rulerNode->GetID());
    if (lineDisplayNodeID.empty())
    {
        cout << "No line display node found, adding a new text display node for this one" << endl;
        this->AddLineDiaplayNodeID(rulerNode);
        lineDisplayNodeID = this->GetLineDisplayNodeID(rulerNode->GetID());
        if (lineDisplayNodeID.empty())
        {
            vtkErrorMacro("Error adding a new line display node for ruler node " << rulerNode->GetID());
            return;
        }
    }
*/}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node)
{
    cout << "vtkSlicerAnnotationRulerManager::Update Text Display Property" << endl;
/*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in text display node is null, returning");
        return;
    }

    std::string rulerNodeID = this->GetRulerNodeIDFromTextDisplayNode(node->GetID());
    if (rulerNodeID.empty())
    {
        cout << "No ruler found" << endl;
        return;
    }

    vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(rulerNodeID.c_str());
    if (!distanceWidget)
    {
        vtkErrorMacro("Error adding a new distance widget for ruler node " << rulerNodeID);
        return;
    }
    if (distanceWidget->GetWidget() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance widget is null");
        return;
    }
    if (distanceWidget->GetWidget()->GetRepresentation() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance representation is null");
        return;
    }

    vtkLineRepresentation *lineRep  = vtkLineRepresentation::SafeDownCast(distanceWidget->GetWidget()->GetRepresentation());

    if (!lineRep)
    {
        vtkErrorMacro("LineRepresenation not defined for distance widget");
        return;
    }

    // text color
    double* rgb1 = node->GetSelectedColor();
    if (!rgb1)
    {
        vtkErrorMacro("active ruler node has no text color defined!"); 
        return;
    }
     if (this->GetMRMLScene()->GetNodeByID(rulerNodeID)->GetSelected())
     {
          rgb1 = node->GetColor();
     } 

     lineRep->GetDistanceAnnotationProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // Text Scale
    double scale = node->GetTextScale();
    double scaleVec[3] = {scale,scale,scale};
    if (scale)
    {
        lineRep->SetDistanceAnnotationScale(scaleVec);
    }

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
    */
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node)
{
    cout << "vtkSlicerAnnotationRulerManager::Update Point Display Property" << endl;
/*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in point display node is null, returning");
        return;
    }

    std::string rulerNodeID = this->GetRulerNodeIDFromPointDisplayNode(node->GetID());
    if (rulerNodeID.empty())
    {
        cout << "No ruler found" << endl;
        return;
    }

    vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(rulerNodeID.c_str());
    if (!distanceWidget)
    {
        vtkErrorMacro("Error adding a new distance widget for ruler node " << rulerNodeID);
        return;
    }
    if (distanceWidget->GetWidget() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance widget is null");
        return;
    }
    if (distanceWidget->GetWidget()->GetRepresentation() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance representation is null");
        return;
    }

    vtkLineRepresentation *lineRep  = vtkLineRepresentation::SafeDownCast(distanceWidget->GetWidget()->GetRepresentation());

    if (!lineRep)
    {
        vtkErrorMacro("LineRepresenation not defined for distance widget");
        return;
    }

    // end point color
    double *rgb1 = node->GetSelectedColor();
    if (!rgb1)
    {
        vtkErrorMacro("active ruler node has no text color defined!"); 
        return;
    }
     if (this->GetMRMLScene()->GetNodeByID(rulerNodeID)->GetSelected())
     {
          rgb1 = node->GetColor();
     } 

    lineRep->GetPoint1Representation()->GetProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);
    lineRep->GetPoint2Representation()->GetProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);
    rgb1 = node->GetSelectedColor();
    lineRep->GetPoint1Representation()->GetSelectedProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);
    lineRep->GetPoint2Representation()->GetSelectedProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // Point Size
    // ToDo: get point size from Node

    // Point Opacity
    lineRep->GetPoint1Representation()->GetProperty()->SetOpacity(node->GetOpacity());
    lineRep->GetPoint2Representation()->GetProperty()->SetOpacity(node->GetOpacity());

    // Point Ambient
    lineRep->GetPoint1Representation()->GetProperty()->SetAmbient(node->GetAmbient());
    lineRep->GetPoint2Representation()->GetProperty()->SetAmbient(node->GetAmbient());

    // Point Diffuse
    lineRep->GetPoint1Representation()->GetProperty()->SetDiffuse(node->GetDiffuse());
    lineRep->GetPoint2Representation()->GetProperty()->SetDiffuse(node->GetDiffuse());

    // Point Specular
    lineRep->GetPoint1Representation()->GetProperty()->SetSpecular(node->GetSpecular());
    lineRep->GetPoint2Representation()->GetProperty()->SetSpecular(node->GetSpecular());

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateLineDisplayProperty(vtkMRMLAnnotationLineDisplayNode *node)
{

    cout << "vtkSlicerAnnotationRulerManager::Update Line Display Property" << endl;
/*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in line display node is null, returning");
        return;
    }

    std::string rulerNodeID = this->GetRulerNodeIDFromLineDisplayNode(node->GetID());
    if (rulerNodeID.empty())
    {
        cout << "No ruler found" << endl;
        return;
    }

    vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(rulerNodeID.c_str());
    if (!distanceWidget)
    {
        vtkErrorMacro("Error adding a new distance widget for ruler node " << rulerNodeID);
        return;
    }
    if (distanceWidget->GetWidget() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance widget is null");
        return;
    }
    if (distanceWidget->GetWidget()->GetRepresentation() == NULL)
    {
        vtkWarningMacro("Update3D widget: distance representation is null");
        return;
    }

    vtkLineRepresentation *lineRep  = vtkLineRepresentation::SafeDownCast(distanceWidget->GetWidget()->GetRepresentation());

    if (!lineRep)
    {
        vtkErrorMacro("LineRepresenation not defined for distance widget");
        return;
    }

    // line color
    double *rgb1 = node->GetSelectedColor();
    if (!rgb1)
    {
        vtkErrorMacro("active ruler node has no text color defined!"); 
        return;
    }

    if (this->GetMRMLScene()->GetNodeByID(rulerNodeID)->GetSelected())
    {
        rgb1 = node->GetColor();
    } 
    lineRep->GetLineProperty()->SetColor(rgb1[0], rgb1[1], rgb1[2]);

    // Line Width
    lineRep->GetLineProperty()->SetLineWidth(node->GetLineThickness());

    // Line Opacity
    lineRep->GetLineProperty()->SetOpacity(node->GetOpacity());

    // Line Ambient
    lineRep->GetLineProperty()->SetAmbient(node->GetAmbient());

    // Line Diffuse
    lineRep->GetLineProperty()->SetDiffuse(node->GetDiffuse());

    // Line Specular
    lineRep->GetLineProperty()->SetSpecular(node->GetSpecular());

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::AddMRMLObservers ( )
{
    /*
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
    }
    */
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveMRMLObservers ( )
{
/*
    // remove observers on the ruler nodes
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode");
    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationRulerNode"));
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
        //vtkDebugMacro("RemoveMRMLObservers: stopping watching for node removed, added, scene close events on the scene");
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateRulerWidgetInteractors()
{
/*
    bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
      this->GetViewerWidget()->GetMainViewer() == NULL ||
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
    {
    //vtkDebugMacro("UpdateRulerWidgetInteractors: viewer widget or main viewer are null, resetting to null");
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
      vtkMRMLAnnotationRulerNode *rulerNode = NULL;
      if (this->GetMRMLScene())
        {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
          {
          rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
          }
        }
      if (rulerNode != NULL)
        {
        this->Update3DWidgetVisibility(rulerNode);
        }
      }
    }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::AddDistanceWidget(vtkMRMLAnnotationRulerNode *rulerNode)
{
    /*
    if (!rulerNode)
    {
        return;
    }
    if (this->GetDistanceWidget(rulerNode->GetID()) != NULL)
    {
        //vtkDebugMacro("Already have widgets for ruler node " << rulerNode->GetID());
        return;
    }

    vtkMeasurementsDistanceWidgetClass *c = vtkMeasurementsDistanceWidgetClass::New();
    this->DistanceWidgets[rulerNode->GetID()] = c;
    // watch for control point modified events
    if (rulerNode->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->AddObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (rulerNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (rulerNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (rulerNode->HasObserver(vtkMRMLAnnotationRulerNode::RulerNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->AddObserver(vtkMRMLAnnotationRulerNode::RulerNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
     if (rulerNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
     {
          rulerNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
     }
     */
}

//---------------------------------------------------------------------------
/*vtkMeasurementsDistanceWidgetClass* vtkSlicerAnnotationRulerManager::GetDistanceWidget(const char *nodeID)
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
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveDistanceWidget(vtkMRMLAnnotationRulerNode *rulerNode)
{
    if (!rulerNode)
    {
        return;
    }
/*
    if (this->GetDistanceWidget(rulerNode->GetID()) != NULL)
    {
        this->DistanceWidgets[rulerNode->GetID()]->Delete();
        this->DistanceWidgets.erase(rulerNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationRulerManager::GetTextDisplayNodeID(const char *nodeID)
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
    return tempstring;*/return NULL;
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationRulerManager::GetRulerNodeIDFromTextDisplayNode(const char *nodeID)
{
    /*
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->TextDisplayNodes.begin();    iter != this->TextDisplayNodes.end(); iter++)
    {
        if (iter->second.c_str() && !strcmp(iter->second.c_str(), nodeID))
        {
            return iter->first;
        }
    }
    return tempstring;
*/ return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::AddTextDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{
/*    if (!rulerNode)
    {
        return;
    }
    if (rulerNode->GetAnnotationTextDisplayNode() == NULL)
    {
        rulerNode->CreateAnnotationTextDisplayNode();
    }

    this->TextDisplayNodes[rulerNode->GetID()] = rulerNode->GetAnnotationTextDisplayNode()->GetID();
    if (rulerNode->GetAnnotationTextDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->GetAnnotationTextDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveTextDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{
    /*if (!rulerNode)
    {
        return;
    }
    if (this->GetTextDisplayNodeID(rulerNode->GetID()).empty())
    {
        this->TextDisplayNodes.erase(rulerNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationRulerManager::GetPointDisplayNodeID(const char *nodeID)
{
/*    std::string tempstring;
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
std::string vtkSlicerAnnotationRulerManager::GetRulerNodeIDFromPointDisplayNode(const char *nodeID)
{
    /*
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
void vtkSlicerAnnotationRulerManager::AddPointDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{
    /*
    if (!rulerNode)
    {
        return;
    }
    if (rulerNode->GetAnnotationPointDisplayNode() == NULL)
    {
        rulerNode->CreateAnnotationPointDisplayNode();
    }

    this->PointDisplayNodes[rulerNode->GetID()] = rulerNode->GetAnnotationPointDisplayNode()->GetID();
    if (rulerNode->GetAnnotationPointDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->GetAnnotationPointDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemovePointDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{
    if (!rulerNode)
    {
        return;
    }
    /*if (this->GetPointDisplayNodeID(rulerNode->GetID()).empty())
    {
        this->PointDisplayNodes.erase(rulerNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
std::string vtkSlicerAnnotationRulerManager::GetLineDisplayNodeID(const char *nodeID)
{
    /*
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->LineDisplayNodes.begin(); iter != this->LineDisplayNodes.end(); iter++)
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
std::string vtkSlicerAnnotationRulerManager::GetRulerNodeIDFromLineDisplayNode(const char *nodeID)
{
    /*
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->LineDisplayNodes.begin(); iter != this->LineDisplayNodes.end(); iter++)
    {
        if (iter->second.c_str() && !strcmp(iter->second.c_str(), nodeID))
        {
            return iter->first;
        }
    }
    return tempstring;
    */
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::AddLineDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{ /*
    if (!rulerNode)
    {
        return;
    }
    if (rulerNode->GetAnnotationLineDisplayNode() == NULL)
    {
        rulerNode->CreateAnnotationLineDisplayNode();
    }

    this->LineDisplayNodes[rulerNode->GetID()] = rulerNode->GetAnnotationLineDisplayNode()->GetID();
    if (rulerNode->GetAnnotationLineDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        rulerNode->GetAnnotationLineDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveLineDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode)
{
    /*
    if (!rulerNode)
    {
        return;
    }
    if (this->GetLineDisplayNodeID(rulerNode->GetID()).empty())
    {
        this->LineDisplayNodes.erase(rulerNode->GetID());
    }
    */
}
/*
//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
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

    //vtkDebugMacro("SetViewerWidget: Updating any widget interactors");
    this->UpdateRulerWidgetInteractors();
}
*/
//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::Update3DWidgetVisibility(vtkMRMLAnnotationRulerNode *activeRulerNode)
{
    if (activeRulerNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in ruler node is null, returning");
        return;
    }
    /*
    vtkMeasurementsDistanceWidgetClass *distanceWidgetClass = this->GetDistanceWidget(activeRulerNode->GetID());
    if (!distanceWidgetClass)
    {
        //vtkErrorMacro("Update3DWidgetVisibility: no widget to update!");
        return;
    }
    //vtkLineWidget2 *distanceWidget = distanceWidgetClass->GetWidget();

  if (distanceWidget->GetInteractor() == NULL)
    {
        if (this->GetViewerWidget() &&
            this->GetViewerWidget()->GetMainViewer() &&
            this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor())
        {
            distanceWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
            double* p1 = new double(3);
            double* p2 = new double(3);
            p1 = activeRulerNode->GetPosition1();
            p2 = activeRulerNode->GetPosition2();
            if ( p1 != NULL && p2 != NULL)
            {
                vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetPoint1WorldPosition(p1);
                vtkLineRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetPoint2WorldPosition(p2);
            }
            delete p1;
            delete p2;
        }
        else
        {
            vtkWarningMacro("Update3DWidgetVisibility: no interactor found! Ruler widget won't work until this is set");
            distanceWidget->SetInteractor(NULL);
        }
    }

    if (activeRulerNode->GetVisibility())
    {
        if (distanceWidget->GetInteractor() != NULL)
        {
            //vtkDebugMacro("Update3DWidgetVisibility: distance widget on");
            distanceWidget->On();
        }
        else
        {
            vtkWarningMacro("Update3DWidgetVisibility: no interactor set");
        }
    }
    else
    {
        //vtkDebugMacro("Update3DWidgetVisibility: distance widget off");
        distanceWidget->Off();
        distanceWidget->ProcessEventsOff();
    }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveDistanceWidgets()
{
    //int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode");
    //vtkDebugMacro("RemoveDistanceWidgets: have " << nnodes << " ruler  nodes in the scene, " << this->DistanceWidgets.size() << " widgets defined already");
    /*
    if (nnodes == 0)
    {
        // the scene was closed, all the nodes are gone, so do this w/o reference
        // to the nodes
        //vtkDebugMacro("RemoveDistanceWidgets: no ruler nodes in scene, removing ruler widgets w/o reference to nodes");
        std::map<std::string, vtkMeasurementsDistanceWidgetClass *>::iterator iter;
        for (iter = this->DistanceWidgets.end();
            iter != this->DistanceWidgets.end();
            iter++)
        {
            //vtkDebugMacro("RemoveDistanceWidgets: deleting and erasing " << iter->first);
            iter->second->Delete();
            this->DistanceWidgets.erase(iter);
        }
        this->DistanceWidgets.clear();
    }
    else
    {
        for (int n=0; n<nnodes; n++)
        {
            vtkMRMLAnnotationRulerNode *rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationRulerNode"));
            if (rnode)
            {
                this->RemoveDistanceWidget(rnode);
            }
        }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::Update3DWidgetsFromMRML()
{
    /*if (!this->MRMLScene)
    {
        //vtkDebugMacro("UpdateFromMRML: no mrml scene from which to update!");
        return;
    }

    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode");
    //vtkDebugMacro("UpdateFromMRML: have " << nnodes << " ruler nodes in the scene, " << this->DistanceWidgets.size() << " widgets defined already");

    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationRulerNode *rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationRulerNode"));
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
        //vtkDebugMacro("UpdateFromMRML: after adding widgets for scene nodes, have " << this->DistanceWidgets.size() << " instead of " << nnodes);
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
                //vtkDebugMacro("UpdateFromMRML: found an extra widget with id " << iter->first.c_str());
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
    }*/
}

void vtkSlicerAnnotationRulerManager::UpdateCamera()
{/*
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
    }*/
}

//---------------------------------------------------------------------------
vtkCamera *vtkSlicerAnnotationRulerManager::GetActiveCamera()
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

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateDistanceMeasurement(vtkMRMLAnnotationRulerNode* rulerNode)
{
    /*if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->Render();
    }



    vtkMeasurementsDistanceWidgetClass *distanceWidgetClass = this->GetDistanceWidget(rulerNode->GetID());
    vtkLineRepresentation *distanceRepresentation = vtkLineRepresentation::SafeDownCast(distanceWidgetClass->GetWidget()->GetRepresentation());
    if (!distanceRepresentation)
    {
        std::cerr << "GetDistanceMeasurement: no 3D representation of line" << endl;
        return;
    }

    if (distanceRepresentation->GetDistance() != rulerNode->GetDistanceMeasurement())
    {
        rulerNode->SetDistanceMeasurement(distanceRepresentation->GetDistance());
    }*/
}

//---------------------------------------------------------------------------
vtkSlicerSeedWidgetClass * vtkSlicerAnnotationRulerManager::GetSeedWidget(const char *nodeID)
{/*
    std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
    for (iter = this->SeedWidgets.begin(); iter != this->SeedWidgets.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }*/
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::AddSeedWidget(vtkMRMLAnnotationRulerNode* rulerNode)
{
    if (!rulerNode)
    {
        //vtkErrorMacro("AddSeedWidget: null node, can't add a seed widget without one");
        return;
    }
    /*if (this->GetSeedWidget(rulerNode->GetID()) != NULL)
    {
        //vtkDebugMacro("AddSeedWidget: Already have widgets ");
        return;
    }

    vtkSlicerSeedWidgetClass *c = vtkSlicerSeedWidgetClass::New();
    this->SeedWidgets[rulerNode->GetID()] = c;

    // set the interactor first
    this->Update3DWidgetVisibility(rulerNode);
    this->Update3DWidget(rulerNode);
*/}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::RemoveSeedWidget(vtkMRMLAnnotationRulerNode* rulerNode)
{/*
    if (!rulerNode)
    {
        //vtkErrorMacro("RemoveSeedWidget: null input list!");
        return;
    }
    // don't create the widget if it's not already there
    vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(rulerNode->GetID());
    if (seedWidget != NULL)
    {
        seedWidget = NULL;
        this->SeedWidgets[rulerNode->GetID()]->Delete();
        std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
        iter = this->SeedWidgets.find(rulerNode->GetID());
        if (iter != this->SeedWidgets.end())
        {
            this->SeedWidgets.erase(iter);
        }
    }
*/}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationRulerManager::UpdateLockUnlock(vtkMRMLAnnotationRulerNode* rulerNode)
{
     if (rulerNode == NULL)
     {
          return;
     }

     /*vtkMeasurementsDistanceWidgetClass *distanceWidget = this->GetDistanceWidget(rulerNode->GetID());
     if (!distanceWidget)
     {
          cout << "No distance widget found, adding a distance widget for this one" << endl;
          this->AddDistanceWidget(rulerNode);
          distanceWidget = this->GetDistanceWidget(rulerNode->GetID());
          if (!distanceWidget)
          {
               return;
          }
     }

     if (distanceWidget->GetWidget() == NULL)
     {
          return;
     }

     if ( rulerNode->GetLocked() )
     {
          distanceWidget->GetWidget()->ProcessEventsOff();
     } 
     else
     {
          distanceWidget->GetWidget()->ProcessEventsOn();
     }*/
}
