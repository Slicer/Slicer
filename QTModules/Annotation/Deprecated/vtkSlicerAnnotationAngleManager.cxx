#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkMath.h"

#include "vtkSlicerAnnotationAngleManager.h"

#include "vtkAngleWidget.h"
#include "vtkSphereHandleRepresentation.h"
#include "vtkAngleRepresentation3D.h"
#include "vtkPolygonalSurfacePointPlacer.h"
//#include "vtkMeasurementsAngleWidgetClass.h"

//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerApplication.h"
//#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"

#include "vtkPointHandleRepresentation3D.h"

class vtkSlicerAnnotationAngleManagerCallback : public vtkCommand
{
public:
    static vtkSlicerAnnotationAngleManagerCallback *New()
    { return new vtkSlicerAnnotationAngleManagerCallback; }
    virtual void Execute (vtkObject *caller, unsigned long event, void*)
    {
        // save node for undo if it's the start of an interaction event
        if (event == vtkCommand::StartInteractionEvent)
        {
            if (this->AngleNode && this->AngleNode->GetScene())
            {
                this->AngleNode->GetScene()->SaveStateForUndo(this->AngleNode);
            }
        }
        else if (event == vtkCommand::InteractionEvent)
        {
            vtkAngleWidget *angleWidget = reinterpret_cast<vtkAngleWidget*>(caller);
            if (angleWidget)
            {
                if (angleWidget->GetRepresentation())
                {
                    vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
                    if (rep)
                    {
                        double p1[3], p2[3], p3[3];
                        rep->GetPoint1WorldPosition(p1);
                        rep->GetPoint2WorldPosition(p2);
                        rep->GetCenterWorldPosition(p3);
                        if (this->AngleNode)
                        {
                            // does the angle node have a transform?
                            vtkMRMLTransformNode* tnode = this->AngleNode->GetParentTransformNode();
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
                            this->AngleNode->SetPosition1(worldxyz[0], worldxyz[1], worldxyz[2]);
                            // second point
                            xyzw[0] = p2[0];
                            xyzw[1] = p2[1];
                            xyzw[2] = p2[2];
                            xyzw[3] = 1.0;
                            transformToWorld->MultiplyPoint(xyzw, worldp);
                            this->AngleNode->SetPosition2(worldxyz[0], worldxyz[1], worldxyz[2]);
                            // center point
                            xyzw[0] = p3[0];
                            xyzw[1] = p3[1];
                            xyzw[2] = p3[2];
                            xyzw[3] = 1.0;
                            transformToWorld->MultiplyPoint(xyzw, worldp);
                            this->AngleNode->SetPositionCenter(worldxyz[0], worldxyz[1], worldxyz[2]);

                            transformToWorld->Delete();
                            transformToWorld = NULL;
                            tnode = NULL;
                        }
                    }
                }
            }
        }

    }
    vtkSlicerAnnotationAngleManagerCallback():AngleNode(0) {}
    vtkMRMLAnnotationAngleNode *AngleNode;
};

//---------------------------------------------------------------------------
//vtkStandardNewMacro (vtkSlicerAnnotationAngleManager );
//vtkCxxRevisionMacro ( vtkSlicerAnnotationAngleManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerAnnotationAngleManager::vtkSlicerAnnotationAngleManager ( )
{
  this->AngleNodeID = NULL;
  this->ViewerWidget = NULL;

  //this->SetAngleNodeID(NULL);
  
  this->Updating3DAngle = 0;

}


//---------------------------------------------------------------------------
vtkSlicerAnnotationAngleManager::~vtkSlicerAnnotationAngleManager ( )
{
  this->RemoveMRMLObservers();

  // gui elements

  // 3d widgets
  //this->SetAngleNodeID(NULL);
  //this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationAngleManager::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkMeasurementsAngleWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
    std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
    for (iter = this->AngleWidgets.begin(); iter !=  this->AngleWidgets.end(); iter++)
      {
      os << indent << "Distance Widget: " << iter->first.c_str() << "\n";
      iter->second->GetWidget()->PrintSelf(os, indent);
      }
}*/


//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationAngleManager::SetAngleNodeID ( char *id )
{

    if (this->GetAngleNodeID() != NULL &&
        id != NULL &&
        strcmp(id,this->GetAngleNodeID()) == 0)
    {
        //vtkDebugMacro("SetAngleNodeID: no change in id, not doing anything for now: " << id << endl);
        return;
    }

    // set the id properly - see the vtkSetStringMacro
    this->AngleNodeID = id;

    if (id == NULL)
    {
        //vtkDebugMacro("SetAngleNodeID: NULL input id, clearing GUI and returning.\n");
        this->UpdateWidget(NULL);
        return;
    }

    // get the new node
    vtkMRMLAnnotationAngleNode *newAngle = vtkMRMLAnnotationAngleNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetAngleNodeID()));
    // set up observers on the new node - now done in addistancewidget
    if (newAngle != NULL)
    {
        // set up the GUI
        this->UpdateWidget(newAngle);
    }
    else
    {
        //vtkDebugMacro ("ERROR: unable to get the mrml Angle node to observe!\n");
    }
}*/


//---------------------------------------------------------------------------
// Description:
// the Update InteractionModeAtStartInteraction is a function
// that ensures that Slicer's mouse modes are in agreement
// with direct manipulation-type interactions with widgets in
// the scene.
void vtkSlicerAnnotationAngleManager::UpdateInteractionModeAtStartInteraction()
{
/*
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
void vtkSlicerAnnotationAngleManager::UpdateInteractionModeAtEndInteraction()
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
    //vtkDebugMacro ( "UpdateInteractionModeAtStartInteraction: No interaction node in the scene." );
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
void vtkSlicerAnnotationAngleManager::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
    /*
    vtkMRMLScene *callScene = vtkMRMLScene::SafeDownCast(caller);

    // the scene was closed, don't get node removed events so clear up here
    if (callScene != NULL && event == vtkMRMLScene::SceneClosedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a scene close event");
        // the lists are already gone from the scene, so need to clear out all the
        // widget properties, can't call remove with a node
        this->RemoveAngleWidgets();
        return;
    }
  
    // first check to see if there was a Angle list node deleted
    if (callScene != NULL && callScene == this->MRMLScene && event == vtkMRMLScene::NodeRemovedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a node deleted event on scene");
        // check to see if it was the current node that was deleted
        if (callData != NULL)
        {
            vtkMRMLAnnotationAngleNode *delNode = reinterpret_cast<vtkMRMLAnnotationAngleNode*>(callData);
            if (delNode != NULL && delNode->IsA("vtkMRMLAnnotationAngleNode"))
            {
                //vtkDebugMacro("A Angle node got deleted " << (delNode->GetID() == NULL ? "null" : delNode->GetID()));
                this->RemoveAngleWidget(delNode);
                this->RemoveTextDisplayNodeID(delNode);
                this->RemovePointDisplayNodeID(delNode);
                this->RemoveLineDisplayNodeID(delNode);
            }
        }
    }

    vtkMRMLAnnotationAngleNode *node = vtkMRMLAnnotationAngleNode::SafeDownCast(caller);
    vtkMRMLAnnotationAngleNode *activeAngleNode = (vtkMRMLAnnotationAngleNode *)this->MRMLScene->GetNodeByID(this->GetAngleNodeID());

    // check for a node added to the scene event
    if (callScene != NULL && callScene == this->MRMLScene && callData != NULL && event == vtkMRMLScene::NodeAddedEvent)
    {
        //vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");
        // check to see if it was a Angle node    
        vtkMRMLAnnotationAngleNode *addNode = reinterpret_cast<vtkMRMLAnnotationAngleNode*>(callData);
        if (addNode != NULL && addNode->IsA("vtkMRMLAnnotationAngleNode"))
        {
            //vtkDebugMacro("Got a node added event with a Angle node " << addNode->GetID());
            // is it currently the active one?
            if (addNode == activeAngleNode)
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

    // check for a Angle node added event
    if (event == vtkMRMLAnnotationAngleNode::AngleNodeAddedEvent)
    {
        // check to see if it was a Angle node    
        if (node != NULL && node->IsA("vtkMRMLAnnotationAngleNode"))
        {
            //vtkDebugMacro("Got a node added event with a Angle node " << node->GetID());
            // is it currently the active one?
            if (node == activeAngleNode)
            {
                //vtkDebugMacro("Calling Update widget to set up the ui since this is the active one");
                this->UpdateWidget(node);
            }
            // if it's not the current one, just update the 3d widget
            //vtkDebugMacro("Calling Update 3D widget to set up a new distance widget");
            this->Updating3DAngle = 0;

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
    else if (node == activeAngleNode)
    {
        if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeAddedEvent 
            || event == vtkMRMLScene::NodeRemovedEvent)
        {
            //vtkDebugMacro("Modified or node added or removed event on the Angle node.\n");
            if (node == NULL)
            {
                //vtkDebugMacro("\tBUT: the node is null\n");
                // check to see if the id used to get the node is not null, if it's
                // a valid string, means that the node was deleted
                if (this->GetAngleNodeID() != NULL)
                {
                    this->SetAngleNodeID(NULL);
                }
                return;
            }
            //vtkDebugMacro("ProcessMRMLEvents: \t\tUpdating the GUI\n");
            // update the gui
            UpdateWidget(activeAngleNode);
            return;
        }
    } // end of events on the active Angle node
    else if (node != NULL && event == vtkCommand::ModifiedEvent)
    {
        // it's a modified event on a Angle node that's not being displayed in the
        // 2d gui, so update the 3d widget
        this->Updating3DAngle = 0;
        this->Update3DWidget(node);
    }
    // Process event when one of the two points of the Angle has coordinate change 
    else if (node != NULL && event == vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent)
    {
        // it's a modified event on a Angle node that's not being displayed in the
        // 2d gui, so update the 3d widget
        //this->Updating3DAngle = 0;
        this->Update3DWidget(node);
        this->UpdateAngleMeasurement(node);
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
void vtkSlicerAnnotationAngleManager::UpdateWidget(vtkMRMLAnnotationAngleNode *activeAngleNode)
{ 
    //vtkDebugMacro("UpdateWidget: active Angle node is " << (activeAngleNode == NULL ? "null" : activeAngleNode->GetName()));

    // if the passed node is null, clear out the widget
    if (activeAngleNode == NULL)
    {
        // don't need to do anything yet, especially don't set the node selector to
        // null, as it causes a crash
        //vtkDebugMacro("UpdateWidget: The passed in node is null, returning.");
        return;
    }

    //vtkDebugMacro("UpdateWidget: updating the gui and 3d elements");

   
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::Update3DWidget(vtkMRMLAnnotationAngleNode *activeAngleNode)
{
    /*
    if (activeAngleNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in Angle node is null, returning");
        return;
    }

    if (this->Updating3DAngle)
    {
        //vtkDebugMacro("Already updating 3d widget");
        return;
    }

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(activeAngleNode->GetID());
    if (!angleWidgetClass)
    {
        cout << "No distance widget found, adding a distance widget for this one" << endl;
        this->AddAngleWidget(activeAngleNode);
        angleWidgetClass = this->GetAngleWidget(activeAngleNode->GetID());
        if (!angleWidgetClass)
        {
            //vtkErrorMacro("Error adding a new distance widget for Angle node " << activeAngleNode->GetID());
            this->Updating3DAngle = 0;
            return;
        }
    }
    vtkAngleWidget *angleWidget = angleWidgetClass->GetWidget();
    if (angleWidget == NULL)
    {
        //vtkWarningMacro("Update3D widget: distance widget is null");
        return;
    }
    if (angleWidget->GetRepresentation() == NULL)
    {
        //vtkWarningMacro("Update3D widget: angle representation is null");
        return;
    }

    vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
    if (angleRepresentation == NULL)
    {
        //vtkErrorMacro("Update3DWidget: angle representation is not a 3d one");
        this->Updating3DAngle = 0;
        return;
    }
    this->Updating3DAngle = 1;

    // set up call back
    // temp: remove observers
    angleWidget->RemoveObservers(vtkCommand::InteractionEvent);
    angleWidget->RemoveObservers(vtkCommand::StartInteractionEvent);

    // now add call back
    vtkSlicerAnnotationAngleManagerCallback *myCallback = vtkSlicerAnnotationAngleManagerCallback::New();
    myCallback->AngleNode = activeAngleNode;
    angleWidget->AddObserver(vtkCommand::InteractionEvent,myCallback);
    angleWidget->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
    myCallback->Delete();

    vtkPointHandleRepresentation3D *rep1 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint1Representation());
    vtkPointHandleRepresentation3D *rep2 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint2Representation());
    vtkPointHandleRepresentation3D *rep3 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetCenterRepresentation());

    if ( rep1==NULL || rep2==NULL || rep3==NULL)
    {
        std::cout << "Angle Node Point Representations are not available" << std::endl;
        return;
    }
    
    // visibility
    this->Update3DWidgetVisibility(activeAngleNode);

    if (angleRepresentation)
    {

        // position
        // get any transform on the node
        vtkMRMLTransformNode* tnode = activeAngleNode->GetParentTransformNode();
        vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
        transformToWorld->Identity();
        if (tnode != NULL && tnode->IsLinear())
        {
            vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
            lnode->GetMatrixTransformToWorld(transformToWorld);
        }
        double *p = activeAngleNode->GetPosition1();
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
            angleRepresentation->SetPoint1WorldPosition(worldp);
        }
        p =  activeAngleNode->GetPosition2();
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
            angleRepresentation->SetPoint2WorldPosition(worldp);
        }
        p =  activeAngleNode->GetPositionCenter();
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
            angleRepresentation->SetCenterWorldPosition(worldp);
        }
        tnode = NULL;
        transformToWorld->Delete();
        transformToWorld = NULL;

        // sub component visibility
        angleRepresentation->SetRay1Visibility(activeAngleNode->GetRay1Visibility());
        angleRepresentation->SetRay2Visibility(activeAngleNode->GetRay2Visibility());
        angleRepresentation->SetArcVisibility(activeAngleNode->GetArcVisibility());

        // distance annotation
        angleRepresentation->SetLabelFormat(activeAngleNode->GetLabelFormat());

        this->UpdateTextDisplayProperty(activeAngleNode->GetAnnotationTextDisplayNode());
        this->UpdatePointDisplayProperty(activeAngleNode->GetAnnotationPointDisplayNode());
        this->UpdateLineDisplayProperty(activeAngleNode->GetAnnotationLineDisplayNode());
    }
    else 
    {
        //vtkErrorMacro("LineRepresentation is not fully defined");
        return; 
    }
    
    // first point constraint
    if (activeAngleNode->GetModelID1())
    {
        // get the model node
        vtkMRMLModelNode *model = 
            vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelID1()));
        vtkMRMLSliceNode *slice =
            vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelID1()));
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
                    //vtkWarningMacro("The " <<  slice->GetName() << " slice is not visible, you won't be able to move the end point.");
                }
                else
                {
                    //vtkWarningMacro("The " <<  model->GetName() << " model is not visible, you won't be able to move the end point");
                }
            }
            vtkProp *prop = vtkProp::SafeDownCast(this->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()));
            // is it already set to constrain the point placer?
            if (prop &&
                !angleWidgetClass->GetModel1PointPlacer()->HasProp(prop))
            {
                // clear out any others
                angleWidgetClass->GetModel1PointPlacer()->RemoveAllProps();
                // add this one
                angleWidgetClass->GetModel1PointPlacer()->AddProp(prop);
                angleRepresentation->GetPoint1Representation()->ConstrainedOff();
                angleRepresentation->GetPoint1Representation()->SetPointPlacer(angleWidgetClass->GetModel1PointPlacer());
            }
        }
        else
        {
            angleWidgetClass->GetModel1PointPlacer()->RemoveAllProps();
            angleRepresentation->GetPoint1Representation()->SetPointPlacer(NULL);
        }
    }
    else
    {
        // make sure it's not constrained
        angleWidgetClass->GetModel1PointPlacer()->RemoveAllProps();
        angleRepresentation->GetPoint1Representation()->SetPointPlacer(NULL);
    }

    // second point constraint
    if (activeAngleNode->GetModelID2())
    {
        // get the model node
        vtkMRMLModelNode *model = 
            vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelID2()));
        vtkMRMLSliceNode *slice =
            vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelID2()));
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
                !angleWidgetClass->GetModel2PointPlacer()->HasProp(prop))
            {
                // clear out any others
                angleWidgetClass->GetModel2PointPlacer()->RemoveAllProps();
                // add this one
                angleWidgetClass->GetModel2PointPlacer()->AddProp(prop);
                angleRepresentation->GetPoint2Representation()->ConstrainedOff();
                angleRepresentation->GetPoint2Representation()->SetPointPlacer(angleWidgetClass->GetModel2PointPlacer());
            }
        }
        else
        {
            angleWidgetClass->GetModel2PointPlacer()->RemoveAllProps();
            angleRepresentation->GetPoint2Representation()->SetPointPlacer(NULL);
        }
    }
    else
    {
        // make sure it's not constrained
        angleWidgetClass->GetModel2PointPlacer()->RemoveAllProps();
        angleRepresentation->GetPoint2Representation()->SetPointPlacer(NULL);
    }

    // center point constraint
    if (activeAngleNode->GetModelIDCenter())
    {
        // get the model node
        vtkMRMLModelNode *model = 
            vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelIDCenter()));
        vtkMRMLSliceNode *slice =
            vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeAngleNode->GetModelIDCenter()));
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
                !angleWidgetClass->GetModelCenterPointPlacer()->HasProp(prop))
            {
                // clear out any others
                angleWidgetClass->GetModelCenterPointPlacer()->RemoveAllProps();
                // add this one
                angleWidgetClass->GetModelCenterPointPlacer()->AddProp(prop);
                angleRepresentation->GetCenterRepresentation()->ConstrainedOff();
                angleRepresentation->GetCenterRepresentation()->SetPointPlacer(angleWidgetClass->GetModelCenterPointPlacer());
            }
        }
        else
        {
            angleWidgetClass->GetModelCenterPointPlacer()->RemoveAllProps();
            angleRepresentation->GetCenterRepresentation()->SetPointPlacer(NULL);
        }
    }
    else
    {
        // make sure it's not constrained
        angleWidgetClass->GetModelCenterPointPlacer()->RemoveAllProps();
        angleRepresentation->GetCenterRepresentation()->SetPointPlacer(NULL);
    }

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
    // reset the flag
    this->Updating3DAngle = 0;
    cout << "vtkSlicerAnnotationModuleLogic::Update3DAngle End: Successful" << endl;
    */
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::UpdateDisplayNode(vtkMRMLAnnotationAngleNode* AngleNode)
{
    cout << "vtkSlicerAnnotationAngleManager::UpdateDisplayNode Start" << endl;
/*
    if (AngleNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in Angle node is null, returning");
        return;
    }

    std::string textDisplayNodeID = this->GetTextDisplayNodeID(AngleNode->GetID());
    if (textDisplayNodeID.empty())
    {
        cout << "No text display node found, adding a new text display node for this one" << endl;
        this->AddTextDiaplayNodeID(AngleNode);
        textDisplayNodeID = this->GetTextDisplayNodeID(AngleNode->GetID());
        if (textDisplayNodeID.empty())
        {
            //vtkErrorMacro("Error adding a new text display node for Angle node " << AngleNode->GetID());
            return;
        }
    }

    std::string pointDisplayNodeID = this->GetPointDisplayNodeID(AngleNode->GetID());
    if (pointDisplayNodeID.empty())
    {
        cout << "No point display node found, adding a new text display node for this one" << endl;
        this->AddPointDiaplayNodeID(AngleNode);
        pointDisplayNodeID = this->GetPointDisplayNodeID(AngleNode->GetID());
        if (pointDisplayNodeID.empty())
        {
            //vtkErrorMacro("Error adding a new point display node for Angle node " << AngleNode->GetID());
            return;
        }
    }

    std::string lineDisplayNodeID = this->GetLineDisplayNodeID(AngleNode->GetID());
    if (lineDisplayNodeID.empty())
    {
        cout << "No line display node found, adding a new text display node for this one" << endl;
        this->AddLineDiaplayNodeID(AngleNode);
        lineDisplayNodeID = this->GetLineDisplayNodeID(AngleNode->GetID());
        if (lineDisplayNodeID.empty())
        {
            //vtkErrorMacro("Error adding a new line display node for Angle node " << AngleNode->GetID());
            return;
        }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node)
{
    cout << "vtkSlicerAnnotationAngleManager::Update Text Display Property" << endl;
    /*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in text display node is null, returning");
        return;
    }

    std::string AngleNodeID = this->GetAngleNodeIDFromTextDisplayNode(node->GetID());

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(AngleNodeID.c_str());
    vtkAngleRepresentation3D *angleRepresentation  = vtkAngleRepresentation3D::SafeDownCast(angleWidgetClass->GetWidget()->GetRepresentation());

    // text color
    double* color;
     if (this->GetMRMLScene()->GetNodeByID(AngleNodeID)->GetSelected())
     {
          color = node->GetColor();
     }
     else
     {
          color = node->GetSelectedColor();
     }
    angleRepresentation->GetTextActor()->GetProperty()->SetColor(color[0], color[1], color[2]);

    // Text Scale
    double scale = node->GetTextScale();
    double scaleVec[3] = {scale,scale,scale};
    if (scale)
    {
        angleRepresentation->SetTextActorScale(scaleVec);
    }

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node)
{
    cout << "vtkSlicerAnnotationAngleManager::Update Point Display Property" << endl;
    /*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in point display node is null, returning");
        return;
    }

    std::string AngleNodeID = this->GetAngleNodeIDFromPointDisplayNode(node->GetID());
    if (AngleNodeID.empty())
    {
        cout << "No Angle found" << endl;
        return;
    }

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(AngleNodeID.c_str());
    if (!angleWidgetClass)
    {
        //vtkErrorMacro("Error adding a new angle widget for Angle node " << AngleNodeID);
        return;
    }
    vtkAngleWidget *angleWidget = angleWidgetClass->GetWidget();
    if (angleWidget == NULL)
    {
        //vtkWarningMacro("Update3D widget: angle widget is null");
        return;
    }
    vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
    if (angleRepresentation == NULL)
    {
        //vtkWarningMacro("Update3D widget: angle representation is null");
        return;
    }

    vtkPointHandleRepresentation3D *rep1 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint1Representation());
    vtkPointHandleRepresentation3D *rep2 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint2Representation());
    vtkPointHandleRepresentation3D *rep3 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetCenterRepresentation());

    if ( rep1==NULL || rep2==NULL || rep3==NULL)
    {
        std::cout << "Angle Node Point Representations are not available" << std::endl;
        return;
    }

    // end point color
    double *color;
     if (this->GetMRMLScene()->GetNodeByID(AngleNodeID)->GetSelected())
     {
          color = node->GetColor();
     }
     else
     {
          color = node->GetSelectedColor();
     }

    rep1->GetProperty()->SetColor(color[0], color[1], color[2]);
    rep2->GetProperty()->SetColor(color[0], color[1], color[2]);
    rep3->GetProperty()->SetColor(color[0], color[1], color[2]);
    color = node->GetSelectedColor();
    rep1->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);
    rep2->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);
    rep3->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);

    // Point Size
    // ToDo: get point size from Node

    // Point Opacity
    rep1->GetProperty()->SetOpacity(node->GetOpacity());
    rep2->GetProperty()->SetOpacity(node->GetOpacity());
    rep3->GetProperty()->SetOpacity(node->GetOpacity());

    // Point Ambient
    rep1->GetProperty()->SetAmbient(node->GetAmbient());
    rep2->GetProperty()->SetAmbient(node->GetAmbient());
    rep3->GetProperty()->SetAmbient(node->GetAmbient());

    // Point Diffuse
    rep1->GetProperty()->SetDiffuse(node->GetDiffuse());
    rep2->GetProperty()->SetDiffuse(node->GetDiffuse());
    rep3->GetProperty()->SetDiffuse(node->GetDiffuse());

    // Point Specular
    rep1->GetProperty()->SetSpecular(node->GetSpecular());
    rep2->GetProperty()->SetSpecular(node->GetSpecular());
    rep3->GetProperty()->SetSpecular(node->GetSpecular());

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::UpdateLineDisplayProperty(vtkMRMLAnnotationLineDisplayNode *node)
{
    cout << "vtkSlicerAnnotationAngleManager::Update Line Display Property" << endl;
    /*
    if (node == NULL)
    {
        //vtkDebugMacro("UpdateTextDisplayProperty: passed in line display node is null, returning");
        return;
    }

    std::string AngleNodeID = this->GetAngleNodeIDFromLineDisplayNode(node->GetID());
    if (AngleNodeID.empty())
    {
        cout << "No Angle found" << endl;
        return;
    }

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(AngleNodeID.c_str());
    if (!angleWidgetClass)
    {
        //vtkErrorMacro("Error adding a new angle widget for Angle node " << AngleNodeID);
        return;
    }
    vtkAngleWidget *angleWidget = angleWidgetClass->GetWidget();
    if (angleWidget == NULL)
    {
        //vtkWarningMacro("Update3D widget: angle widget is null");
        return;
    }
    vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
    if (angleRepresentation == NULL)
    {
        //vtkWarningMacro("Update3D widget: angle representation is null");
        return;
    }

    // line color
    double* color;
    if (this->GetMRMLScene()->GetNodeByID(AngleNodeID)->GetSelected())
    {
        color = node->GetColor();
    }
    else
    {
        color = node->GetSelectedColor();
    }
    angleRepresentation->GetRay1()->GetProperty()->SetColor(color[0], color[1], color[2]);
    angleRepresentation->GetRay2()->GetProperty()->SetColor(color[0], color[1], color[2]);
     angleRepresentation->GetArc()->GetProperty()->SetColor(color[0], color[1], color[2]);

    // Line Width
    angleRepresentation->GetRay1()->GetProperty()->SetLineWidth(node->GetLineThickness());
    angleRepresentation->GetRay2()->GetProperty()->SetLineWidth(node->GetLineThickness());
     angleRepresentation->GetArc()->GetProperty()->SetLineWidth(node->GetLineThickness());

    // Line Opacity
    angleRepresentation->GetRay1()->GetProperty()->SetOpacity(node->GetOpacity());
    angleRepresentation->GetRay2()->GetProperty()->SetOpacity(node->GetOpacity());
     angleRepresentation->GetArc()->GetProperty()->SetOpacity(node->GetOpacity());

    // Line Ambient
    angleRepresentation->GetRay1()->GetProperty()->SetAmbient(node->GetAmbient());
    angleRepresentation->GetRay2()->GetProperty()->SetAmbient(node->GetAmbient());
     angleRepresentation->GetArc()->GetProperty()->SetAmbient(node->GetAmbient());

    // Line Diffuse
    angleRepresentation->GetRay1()->GetProperty()->SetDiffuse(node->GetDiffuse());
    angleRepresentation->GetRay2()->GetProperty()->SetDiffuse(node->GetDiffuse());
     angleRepresentation->GetArc()->GetProperty()->SetDiffuse(node->GetDiffuse());

    // Line Specular
    angleRepresentation->GetRay1()->GetProperty()->SetSpecular(node->GetSpecular());
    angleRepresentation->GetRay2()->GetProperty()->SetSpecular(node->GetSpecular());
     angleRepresentation->GetArc()->GetProperty()->SetSpecular(node->GetSpecular());

    // request a render
    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::AddMRMLObservers ( )
{/*
  // the widget as a whole needs to keep track of Angle nodes in the scene
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
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemoveMRMLObservers ( )
{/*
    // remove observers on the Angle nodes
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationAngleNode");
    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationAngleNode *AngleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationAngleNode"));
        if (AngleNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            AngleNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
        if (AngleNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
            AngleNode->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
        AngleNode = NULL;
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
void vtkSlicerAnnotationAngleManager::UpdateAngleWidgetInteractors()
{
  /*
  bool isNull = false;
  if (this->GetViewerWidget() == NULL ||
      this->GetViewerWidget()->GetMainViewer() == NULL ||
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() == NULL)
    {
    //vtkDebugMacro("UpdateAngleWidgetInteractors: viewer widget or main viewer are null, resetting to null");
    isNull = true;
    }

  std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
  for (iter = this->AngleWidgets.begin(); iter !=  this->AngleWidgets.end(); iter++)
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
      // now update the visibility for the Angle
      vtkMRMLAnnotationAngleNode *AngleNode = NULL;
      if (this->GetMRMLScene())
        {
        vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(iter->first.c_str());
        if (node)
          {
          AngleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);
          }
        }
      if (AngleNode != NULL)
        {
        this->Update3DWidgetVisibility(AngleNode);
        }
      }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::AddAngleWidget(vtkMRMLAnnotationAngleNode *AngleNode)
{
    /*
    if (!AngleNode)
    {
        return;
    }
    if (this->GetAngleWidget(AngleNode->GetID()) != NULL)
    {
        //vtkDebugMacro("Already have widgets for Angle node " << AngleNode->GetID());
        return;
    }

    vtkMeasurementsAngleWidgetClass *c = vtkMeasurementsAngleWidgetClass::New();
    this->AngleWidgets[AngleNode->GetID()] = c;
    // watch for control point modified events
    if (AngleNode->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->AddObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (AngleNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (AngleNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    if (AngleNode->HasObserver(vtkMRMLAnnotationAngleNode::AngleNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->AddObserver(vtkMRMLAnnotationAngleNode::AngleNodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
     if (AngleNode->HasObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
     {
          AngleNode->AddObserver(vtkMRMLAnnotationNode::LockModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
     }
     */
}

//---------------------------------------------------------------------------
vtkMeasurementsAngleWidgetClass* vtkSlicerAnnotationAngleManager::GetAngleWidget(const char *nodeID)
{/*
    std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
    for (iter = this->AngleWidgets.begin(); iter != this->AngleWidgets.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }*/
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemoveAngleWidget(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (this->GetAngleWidget(AngleNode->GetID()) != NULL)
    {
        this->AngleWidgets[AngleNode->GetID()]->Delete();
        this->AngleWidgets.erase(AngleNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetTextDisplayNodeID(const char *nodeID)
{
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->TextDisplayNodes.begin(); iter != this->TextDisplayNodes.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }
    return tempstring;

return NULL;}*/

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetAngleNodeIDFromTextDisplayNode(const char *nodeID)
{
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
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::AddTextDiaplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{
    /*
    if (!AngleNode)
    {
        return;
    }
    if (AngleNode->GetAnnotationTextDisplayNode() == NULL)
    {
        AngleNode->CreateAnnotationTextDisplayNode();
    }

    this->TextDisplayNodes[AngleNode->GetID()] = AngleNode->GetAnnotationTextDisplayNode()->GetID();
    if (AngleNode->GetAnnotationTextDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->GetAnnotationTextDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemoveTextDisplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (this->GetTextDisplayNodeID(AngleNode->GetID()).empty())
    {
        this->TextDisplayNodes.erase(AngleNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetPointDisplayNodeID(const char *nodeID)
{
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->PointDisplayNodes.begin(); iter != this->PointDisplayNodes.end(); iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }
    return tempstring; return NULL;
}*/

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetAngleNodeIDFromPointDisplayNode(const char *nodeID)
{
    std::string tempstring;
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->PointDisplayNodes.begin(); iter != this->PointDisplayNodes.end(); iter++)
    {
        if (iter->second.c_str() && !strcmp(iter->second.c_str(), nodeID))
        {
            return iter->first;
        }
    }
    return tempstring;
 return NULL;}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::AddPointDiaplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (AngleNode->GetAnnotationPointDisplayNode() == NULL)
    {
        AngleNode->CreateAnnotationPointDisplayNode();
    }

    this->PointDisplayNodes[AngleNode->GetID()] = AngleNode->GetAnnotationPointDisplayNode()->GetID();
    if (AngleNode->GetAnnotationPointDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->GetAnnotationPointDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemovePointDisplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (this->GetPointDisplayNodeID(AngleNode->GetID()).empty())
    {
        this->PointDisplayNodes.erase(AngleNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetLineDisplayNodeID(const char *nodeID)
{
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
}*/

//---------------------------------------------------------------------------
/*std::string vtkSlicerAnnotationAngleManager::GetAngleNodeIDFromLineDisplayNode(const char *nodeID)
{
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
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::AddLineDiaplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (AngleNode->GetAnnotationLineDisplayNode() == NULL)
    {
        AngleNode->CreateAnnotationLineDisplayNode();
    }

    this->LineDisplayNodes[AngleNode->GetID()] = AngleNode->GetAnnotationLineDisplayNode()->GetID();
    if (AngleNode->GetAnnotationLineDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
        AngleNode->GetAnnotationLineDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
*/}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemoveLineDisplayNodeID(vtkMRMLAnnotationAngleNode *AngleNode)
{/*
    if (!AngleNode)
    {
        return;
    }
    if (this->GetLineDisplayNodeID(AngleNode->GetID()).empty())
    {
        this->LineDisplayNodes.erase(AngleNode->GetID());
    }*/
}

//---------------------------------------------------------------------------
/*void vtkSlicerAnnotationAngleManager::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
    if (this->ViewerWidget != NULL)
    {
        // TODO: figure out if this is necessary
        this->RemoveAngleWidgets();
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
    this->UpdateAngleWidgetInteractors();
}*/

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::Update3DWidgetVisibility(vtkMRMLAnnotationAngleNode *activeAngleNode)
{/*
    if (activeAngleNode == NULL)
    {
        //vtkDebugMacro("Update3DWidget: passed in Angle node is null, returning");
        return;
    }
    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(activeAngleNode->GetID());
    if (!angleWidgetClass)
    {
        //vtkErrorMacro("Update3DWidgetVisibility: no widget to update!");
        return;
    }

    vtkAngleWidget *angleWidget = angleWidgetClass->GetWidget();

    if (angleWidget->GetInteractor() == NULL)
    {
        if (this->GetViewerWidget() &&
            this->GetViewerWidget()->GetMainViewer() &&
            this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor())
        {
            angleWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());

            // make new handle properties so can have each handle a different colour
            angleWidgetClass->MakeNewHandleProperties();

            //vtkDebugMacro("Update3DWidgetVisibility: little hack to make the angle widget think that it's done being placed.");
            double *p1 = activeAngleNode->GetPosition1();
            double *p2 = activeAngleNode->GetPosition2();
            double *pCenter = activeAngleNode->GetPositionCenter();
            vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
            if (angleRepresentation)
            {
                angleRepresentation->SetPoint1WorldPosition(p1);
                angleRepresentation->SetPoint2WorldPosition(p2);
                angleRepresentation->SetCenterWorldPosition(pCenter);
            }
        }
        else
        {
            //vtkWarningMacro("Update3DWidgetVisibility: no interactor found! Angle widget won't work until this is set");
            angleWidget->SetInteractor(NULL);
        }
    }

    if (activeAngleNode->GetVisibility())
    {

        if (angleWidget->GetInteractor() != NULL)
        {
            angleWidget->On();
            // make sure have new handle properties so can have each handle a different colour
            angleWidgetClass->MakeNewHandleProperties();
            angleWidget->ProcessEventsOn();
        }

    }
    else
    {
        //vtkDebugMacro("UpdateWidget: angle widget off");
        angleWidget->Off();
        angleWidget->ProcessEventsOff();
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::RemoveAngleWidgets()
{/*
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationAngleNode");
    //vtkDebugMacro("RemoveAngleWidgets: have " << nnodes << " Angle  nodes in the scene, " << this->AngleWidgets.size() << " widgets defined already");

    if (nnodes == 0)
    {
        // the scene was closed, all the nodes are gone, so do this w/o reference
        // to the nodes
        //vtkDebugMacro("RemoveDistanceWidgets: no Angle nodes in scene, removing Angle widgets w/o reference to nodes");
        std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
        for (iter = this->AngleWidgets.end();
            iter != this->AngleWidgets.end();
            iter++)
        {
            //vtkDebugMacro("RemoveDistanceWidgets: deleting and erasing " << iter->first);
            iter->second->Delete();
            this->AngleWidgets.erase(iter);
        }
        this->AngleWidgets.clear();
    }
    else
    {
        for (int n=0; n<nnodes; n++)
        {
            vtkMRMLAnnotationAngleNode *rnode = vtkMRMLAnnotationAngleNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationAngleNode"));
            if (rnode)
            {
                this->RemoveAngleWidget(rnode);
            }
        }
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::Update3DWidgetsFromMRML()
{/*
    if (!this->MRMLScene)
    {
        //vtkDebugMacro("UpdateFromMRML: no mrml scene from which to update!");
        return;
    }

    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLAnnotationAngleNode");
    //vtkDebugMacro("UpdateFromMRML: have " << nnodes << " Angle nodes in the scene, " << this->AngleWidgets.size() << " widgets defined already");

    for (int n=0; n<nnodes; n++)
    {
        vtkMRMLAnnotationAngleNode *rnode = vtkMRMLAnnotationAngleNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLAnnotationAngleNode"));
        if (rnode)
        {
            // this call will create one if it's missing
            this->Update3DWidget(rnode);
            // let go of the pointer
            rnode = NULL;
        }
    }

    // now have a widget for each node, check that don't have too many widgets
    if ((int)(this->AngleWidgets.size()) != nnodes)
    {
        //vtkDebugMacro("UpdateFromMRML: after adding widgets for scene nodes, have " << this->AngleWidgets.size() << " instead of " << nnodes);
        // find ones that aren't in the scene, be careful using an iterator because calling erase gets it messed up
        //int numWidgets = this->DistanceWidgets.size();
        std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
        std::vector<std::string> idsToDelete;
        for (iter = this->AngleWidgets.begin();
            iter != this->AngleWidgets.end();
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
            std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator delIter;
            delIter = this->AngleWidgets.find(idsToDelete[i]);
            if (delIter != this->AngleWidgets.end())
            {
                // can't call this->RemoveDistanceWidget because we don't have a node!
                this->AngleWidgets[delIter->first.c_str()]->Delete();
                this->AngleWidgets.erase(delIter->first.c_str());
            }
        }
    }*/
}

void vtkSlicerAnnotationAngleManager::UpdateCamera()
{/*
    vtkCamera *cam = NULL;
    cam = this->GetActiveCamera();

    if (cam == NULL)
    {
        //vtkErrorMacro("UpdateCamera: unable to get active camera");
        return;
    }

    std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter;
    for (iter = this->AngleWidgets.begin(); iter !=  this->AngleWidgets.end(); iter++)
    {
        iter->second->SetCamera(cam);
    }*/
}

//---------------------------------------------------------------------------
vtkCamera *vtkSlicerAnnotationAngleManager::GetActiveCamera()
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

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::SetWidgetColor(vtkMRMLAnnotationAngleNode* node, int type, double* color)
{/*
    if (!node)
    {
        return;
    }

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(node->GetID());
    if (!angleWidgetClass)
    {
        return;
    }
    vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidgetClass->GetWidget()->GetRepresentation());

    switch( type )
    {
    case vtkSlicerAnnotationModuleLogic::TEXT_COLOR:
        angleRepresentation->GetTextActor()->GetProperty()->SetColor(color[0], color[1], color[2]);
        break;
    case vtkSlicerAnnotationModuleLogic::POINT_COLOR:
        {
            vtkPointHandleRepresentation3D *rep1 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint1Representation());
            vtkPointHandleRepresentation3D *rep2 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint2Representation());
            vtkPointHandleRepresentation3D *rep3 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetCenterRepresentation());
            if ( rep1==NULL || rep2==NULL || rep3==NULL)
            {
                std::cout << "Angle Node Point Representations are not available" << std::endl;
                return;
            }
            rep1->GetProperty()->SetColor(color[0], color[1], color[2]);
            rep2->GetProperty()->SetColor(color[0], color[1], color[2]);
            rep3->GetProperty()->SetColor(color[0], color[1], color[2]);
            break;
        }
    case vtkSlicerAnnotationModuleLogic::POINT_SELECTED_COLOR:
        {
            vtkPointHandleRepresentation3D *rep1 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint1Representation());
            vtkPointHandleRepresentation3D *rep2 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetPoint2Representation());
            vtkPointHandleRepresentation3D *rep3 = vtkPointHandleRepresentation3D::SafeDownCast(angleRepresentation->GetCenterRepresentation());
            if ( rep1==NULL || rep2==NULL || rep3==NULL)
            {
                std::cout << "Angle Node Point Representations are not available" << std::endl;
                return;
            }
            rep1->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);
            rep2->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);
            rep3->GetSelectedProperty()->SetColor(color[0], color[1], color[2]);
            break;
        }
    case vtkSlicerAnnotationModuleLogic::LINE_COLOR:
        angleRepresentation->GetRay1()->GetProperty()->SetColor(color[0], color[1], color[2]);
        angleRepresentation->GetRay1()->GetProperty()->SetColor(color[0], color[1], color[2]);
        break;
    case vtkSlicerAnnotationModuleLogic::LINE_SELECTED_COLOR:
        //ToDo
        break;
    default:
        std::cerr << "Property Type not found!" << endl;
        break;
    }*/
}

void vtkSlicerAnnotationAngleManager::UpdateAngleMeasurement(vtkMRMLAnnotationAngleNode* angleNode)
{
    /*if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->Render();
    }

    vtkMeasurementsAngleWidgetClass *angleWidgetClass = GetAngleWidget(angleNode->GetID());
    vtkAngleRepresentation3D *angleRepresentation = vtkAngleRepresentation3D::SafeDownCast(angleWidgetClass->GetWidget()->GetRepresentation());
    if (!angleRepresentation)
    {
        std::cerr << "GetAngleMeasurement: no 3D representation of angle" << endl;
        return;
    }

    if (angleNode->GetAngleMeasurement() != angleRepresentation->GetAngle() / 3.14159265 * 180.0 )
    {
        angleNode->SetAngleMeasurement(angleRepresentation->GetAngle() / 3.14159265 * 180.0);
    }*/
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationAngleManager::UpdateLockUnlock(vtkMRMLAnnotationAngleNode* angleNode)
{
     if (angleNode == NULL)
     {
          return;
     }

     vtkMeasurementsAngleWidgetClass *angleWidgetClass = this->GetAngleWidget(angleNode->GetID());
     if (!angleWidgetClass)
     {
          cout << "No distance widget found, adding a distance widget for this one" << endl;
          this->AddAngleWidget(angleNode);
          angleWidgetClass = this->GetAngleWidget(angleNode->GetID());
          if (!angleWidgetClass)
          {
               return;
          }
     }

     /*vtkAngleWidget *angleWidget = angleWidgetClass->GetWidget();
     if (angleWidget == NULL)
     {
          return;
     }

     if ( angleNode->GetLocked() )
     {
          angleWidget->ProcessEventsOff();
     } 
     else
     {
          angleWidget->ProcessEventsOn();
     }*/
}
