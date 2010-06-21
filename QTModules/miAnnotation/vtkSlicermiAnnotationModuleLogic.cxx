#include "vtkSlicermiAnnotationModuleLogic.h"

#include "vtkSlicerFiducialsLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMeasurementsGUI.h" 
#include "vtkMRMLMeasurementsAngleNode.h"
#include "vtkMeasurementsAngleWidget.h"
#include "vtkMeasurementsAngleWidgetClass.h"
#include "vtkAngleRepresentation3D.h" 
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"

#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkLineRepresentation.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"

#include "vtkSlicerFiducialListWidget.h" 

// Needed for ruler 
#include "vtkLineWidget2.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkProperty.h"
#include "vtkPolygonalSurfacePointPlacer.h"
#include "vtkAngleWidget.h"
#include "vtkMRMLScene.h"
#include "vtkSphereHandleRepresentation.h"

#include "vtkMRMLAnnotationStickyNode.h"

#include "vtkMRMLAnnotationTextNode.h"
#include "vtkSlicerAnnotationTextManager.h"

#include "vtkMRMLAnnotationROINode.h"
#include "vtkSlicerAnnotationROIManager.h"

#include "vtkSlicerAnnotationBidimensionalManager.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"

#include "vtkSplineWidget.h"
#include "vtkSplineRepresentation.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkSlicerAnnotationSplineManager.h"
#include "vtkMRMLAnnotationSplineNode.h"

#include "vtkSeedRepresentation.h"
#include "vtkMRMLFiducial.h"

#include "vtkSlicerAnnotationRulerManager.h"
#include "vtkSlicerAnnotationAngleManager.h"
#include "vtkSlicerAnnotationFiducialManager.h"
#include "vtkSlicerSeedWidgetClass.h"
#include "vtkSeedWidget.h"

#include <string>
#include <iostream>
#include <sstream>

vtkCxxRevisionMacro(vtkSlicermiAnnotationModuleLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicermiAnnotationModuleLogic);

//-----------------------------------------------------------------------------
// General Functions 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class vtkSlicermiAnnotationModuleLogicPrivate: public ctkPrivate<vtkSlicermiAnnotationModuleLogic>   
{
public:
  vtkSlicermiAnnotationModuleLogicPrivate();
  ~vtkSlicermiAnnotationModuleLogicPrivate();
  //char* FiducialListID;
  char* AngleNodeID;
  int numControlPoints;
  int maxControlPoints;
  int Updating3DFiducial;
  int Updating3DRuler;
  int Updating3DAngle;
 //BTX
  // Description:
  // encapsulated 3d widgets for each ruler node
  std::map<std::string, vtkMeasurementsAngleWidgetClass *> AngleWidgets;
  // Not using it really just as dummy place holder
  std::map<std::string, vtkSlicerFiducialListWidget *> FiducialWidgets;
 //ETX
 
};

//-----------------------------------------------------------------------------
vtkSlicermiAnnotationModuleLogicPrivate::vtkSlicermiAnnotationModuleLogicPrivate()
{
  this->AngleNodeID = 0;
  this->Updating3DFiducial = 0;
  this->Updating3DRuler = 0;
  this->Updating3DAngle = 0;
}

//-----------------------------------------------------------------------------
vtkSlicermiAnnotationModuleLogicPrivate::~vtkSlicermiAnnotationModuleLogicPrivate()
{
  // 3d widgets
  std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter2;
  for (iter2 = this->AngleWidgets.begin();
      iter2 != this->AngleWidgets.end();
      iter2++)
  {
      iter2->second->Delete();
  }
  this->AngleWidgets.clear(); 

}

//-----------------------------------------------------------------------------
vtkSlicermiAnnotationModuleLogic::vtkSlicermiAnnotationModuleLogic()
{
  CTK_INIT_PRIVATE(vtkSlicermiAnnotationModuleLogic);  
  this->m_RulerManager = NULL;
  this->m_AngleManager = NULL;
  this->m_FiducialManager = NULL;
  this->m_ROIManager = NULL;
  this->m_SplineManager = NULL;
  this->m_BidimensionalManager = NULL;
  this->m_TextManager = NULL;
}

//-----------------------------------------------------------------------------
vtkSlicermiAnnotationModuleLogic::~vtkSlicermiAnnotationModuleLogic()
{
  if (this->m_RulerManager)
  {
    this->m_RulerManager->SetParent(NULL);
    this->m_RulerManager->Delete();
    this->m_RulerManager = NULL;
  }
  if (this->m_AngleManager)
  {
    this->m_AngleManager->SetParent(NULL);
    this->m_AngleManager->Delete();
    this->m_AngleManager = NULL;
  }
  if (this->m_FiducialManager)
  {
    this->m_FiducialManager->SetParent(NULL);
    this->m_FiducialManager->Delete();
    this->m_FiducialManager = NULL;
  }
  if (this->m_ROIManager)
  {
    this->m_ROIManager->SetParent(NULL);
    this->m_ROIManager->Delete();
    this->m_ROIManager = NULL;
  }
  if (this->m_SplineManager)
  {
    this->m_SplineManager->SetParent(NULL);
    this->m_SplineManager->Delete();
    this->m_SplineManager = NULL;
  }
  if (this->m_BidimensionalManager)
  {
    this->m_BidimensionalManager->SetParent(NULL);
    this->m_BidimensionalManager->Delete();
    this->m_BidimensionalManager = NULL;
  }
  if (this->m_TextManager)
  {
    this->m_TextManager->SetParent(NULL);
    this->m_TextManager->Delete();
    this->m_TextManager = NULL;
  }

}


//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent) 
{ 
  Superclass::PrintSelf(os, indent); 
};

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::ProcessLogicEvents(vtkObject *caller, unsigned long event, void *callData )
{
    // Check RulerNode
    {
        vtkMRMLAnnotationRulerNode *callerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(caller);
        if (callerNode != NULL) 
        {
            if (event == vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent)
            {
                this->TestReceivedMessage = 1;
                //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
                // need to be sure that the modifeid event isn't coming from the widget moving
                //this->Update3DRuler(callerNode);
            }
            if (event == vtkCommand::ModifiedEvent)
            {
                this->TestReceivedMessage = 1;
                //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
                // need to be sure that the modifeid event isn't coming from the widget moving
                //this->Update3DRuler(callerNode);
            }
            if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
            {
                vtkDebugMacro("ProcessLogicEvents: got a transform modified event");
                //this->Update3DRuler(callerNode);
            }
            // check for a node added event
            if (event == vtkMRMLScene::NodeAddedEvent)
            {
                vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");
                //check to see if it was a angle node    
                vtkMRMLAnnotationAngleNode *addNode = reinterpret_cast<vtkMRMLAnnotationAngleNode*>(callData);
                if (addNode != NULL && addNode->IsA("vtkMRMLAnnotationAngleNode"))
                {
                    vtkDebugMacro("Got a node added event with a angle node " << addNode->GetID());
                    return;
                }
            }
        }
    else
    {
      vtkMRMLAnnotationTextDisplayNode *callerNode = vtkMRMLAnnotationTextDisplayNode::SafeDownCast(caller);
      if (callerNode != NULL)
      {
        if (event == vtkCommand::ModifiedEvent)
        {
          this->TestReceivedMessage = 1;
          //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
          // need to be sure that the modifeid event isn't coming from the widget moving
          //this->Update3DRuler(callerNode);
        }
      }
    }
    }

    // Check for fiducials 
    {
        vtkMRMLFiducialListNode* callerNode = vtkMRMLFiducialListNode::SafeDownCast(caller);
        if (callerNode != NULL) 
        {
            if ( event == vtkMRMLScene::NodeAddedEvent ) 
            {
                return;
            }
            if ( event == vtkMRMLFiducialListNode::FiducialModifiedEvent ) 
            {
                return;
            }

            if ( event == vtkMRMLScene::NodeRemovedEvent ) 
            {
                return;
            }
        }
    }


}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SaveMRMLScene()
{
  vtkSlicerApplicationGUI *appGUI = GetApplicationGUI();
  if ( !appGUI )
   {
     return;
   }
  appGUI->ProcessSaveSceneAsCommand();
}

//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicermiAnnotationModuleLogic::GetViewerWidget()
{
  return this->GetApplicationGUI()->GetActiveViewerWidget();
}

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI* vtkSlicermiAnnotationModuleLogic::GetApplicationGUI()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (vtkSlicerApplication::GetInstance()); 
  if ( !app )
    {
     std::cerr << "GetApplicationGUI: got Null SlicerApplication"  << std::endl;
    return NULL;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
   {
     std::cerr << "GetApplicationGUI: got Null SlicerApplicationGUI"  << std::endl;
    return NULL;
   }
  return appGUI;
 
}


//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::RemoveAnnotationByID(const char* id)
{
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
        this->RemoveFiducial(id);
    }
    else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
        this->RemoveRuler(id);
    }
    else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
        this->RemoveAngle(id);
    }

}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::ModifyPropertiesAndWidget(vtkMRMLNode* node, int type, void* data)
{
    if (!node)
    {
        return;
    }
  if ( vtkMRMLAnnotationLinesNode::SafeDownCast(node) == NULL )
  {
    this->SetAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode::SafeDownCast(node), type, data);
  } 
  else
  {
    this->SetAnnotationLinesProperties(vtkMRMLAnnotationLinesNode::SafeDownCast(node), type, data);
  }
  
  this->GetViewerWidget()->Render();
}

//-----------------------------------------------------------------------------
vtkImageData* vtkSlicermiAnnotationModuleLogic::SaveScreenShot()
{
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (vtkSlicerApplication::GetInstance()); 
    if ( !app )
    {
        std::cerr << "selectLayout: got Null SlicerApplication"  << std::endl;
        return NULL;
    }

    vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
    if ( !appGUI )
    {
        std::cerr << "selectLayout: got Null SlicerApplicationGUI"  << std::endl;
        return NULL;
    }

    vtkSlicerViewerWidget* viewerWidget = appGUI->GetActiveViewerWidget();
    vtkKWRenderWidget* mainViewer = viewerWidget->GetMainViewer();
    vtkRenderWindow* win = mainViewer->GetRenderWindow();

    vtkWindowToImageFilter* renderLargeImage = vtkWindowToImageFilter::New();
    renderLargeImage->SetInput( win );
    renderLargeImage->SetMagnification( 1 );
    
    mainViewer->Render();
    renderLargeImage->Update();

    return renderLargeImage->GetOutput();

}

//-----------------------------------------------------------------------------
// Angle Widget     
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class vtkAnnotationAngleWidgetCallback : public vtkCommand
{
public:
    static vtkAnnotationAngleWidgetCallback *New()
    { return new vtkAnnotationAngleWidgetCallback; }
         
    void SetCoordinates(vtkAngleWidget *angleWidget) 
    {
        if (!angleWidget || !this->AngleNode)
        {
            return;

        } 
        vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
        if (!rep)
        {
            return;
        }
        double p1[3], p2[3], p3[3];
        rep->GetPoint1WorldPosition(p1);
        rep->GetPoint2WorldPosition(p2);
        rep->GetCenterWorldPosition(p3);
        // does the ruler node have a transform?
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

        vtkAngleRepresentation3D* angleRep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
        angleRep->SetPoint1WorldPosition(worldxyz);
        // second point
        xyzw[0] = p2[0];
        xyzw[1] = p2[1];
        xyzw[2] = p2[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->AngleNode->SetPosition2(worldxyz[0], worldxyz[1], worldxyz[2]);
        angleRep->SetPoint2WorldPosition(worldxyz);
        // center point
        xyzw[0] = p3[0];
        xyzw[1] = p3[1];
        xyzw[2] = p3[2];
        xyzw[3] = 1.0;
        transformToWorld->MultiplyPoint(xyzw, worldp);
        this->AngleNode->SetPositionCenter(worldxyz[0], worldxyz[1], worldxyz[2]);
        angleRep->SetCenterWorldPosition(worldxyz);
        transformToWorld->Delete();
        transformToWorld = NULL;
        tnode = NULL;

    }
        
    virtual void Execute (vtkObject *caller, unsigned long event, void*)
    {
        // save node for undo if it's the start of an interaction event

        if (event == vtkCommand::PlacePointEvent)
        {
            count++;
            cout << "point placed\n";
            if ( count == 3 )
            {
                cout << "Angle widget placed\n";
                LogicPointer->AddAngleCompleted();
            }
        }

        if (event == vtkCommand::StartInteractionEvent)
        {
            if (this->AngleNode && this->AngleNode->GetScene())
            {
                this->AngleNode->GetScene()->SaveStateForUndo(this->AngleNode);
            }
        }
        else if (event == vtkCommand::InteractionEvent)
        {
          this->SetCoordinates(reinterpret_cast<vtkAngleWidget*>(caller));
        }                    
         
    }
    //,DistanceRepresentation(0)
    vtkAnnotationAngleWidgetCallback():AngleNode(0),AngleWidgetClass(0) { count = 0; }
    vtkMRMLAnnotationAngleNode *AngleNode;
    vtkSlicermiAnnotationModuleLogic* LogicPointer;
    vtkMeasurementsAngleWidgetClass* AngleWidgetClass;
    int count;
    
    //  std::string RulerID;
    //  vtkLineRepresentation *DistanceRepresentation;
};

//-----------------------------------------------------------------------------
vtkMRMLAnnotationAngleNode* vtkSlicermiAnnotationModuleLogic::GetAngleNodeByID(const char* id)
{
    vtkMRMLAnnotationAngleNode* anglenode = 0;
    anglenode = vtkMRMLAnnotationAngleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( id ));
    return anglenode;
}


//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::RemoveAngle(const char* id)
{
    vtkMRMLAnnotationAngleNode *angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID( id ));

    if (angleNode)
    {
        this->GetMRMLScene()->RemoveNode(angleNode);
    }
    this->GetViewerWidget()->Render();
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::AddAngleCompleted()
{
    this->InvokeEvent(vtkSlicermiAnnotationModuleLogic::AddAngleCompletedEvent, NULL);
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SetAnnotationSelectedByIDs(std::vector<const char*> selectedIDs, std::vector<const char*> allIDs)
{
    for (unsigned int i=0; i<allIDs.size(); ++i)
    {
        if ( allIDs[i] == NULL )
        {
            continue;
        }
        this->GetMRMLScene()->GetNodeByID( allIDs[i] )->SetSelected(0);
    }

    for (unsigned int i=0; i<selectedIDs.size(); ++i)
    {
        if ( selectedIDs[i] == NULL )
        {
            continue;
        }
        this->GetMRMLScene()->GetNodeByID( selectedIDs[i] )->SetSelected(1);
    }
    this->GetViewerWidget()->Render();
}

//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddAngle()
{
    if (m_AngleManager == NULL)
    {
        this->m_AngleManager = vtkSlicerAnnotationAngleManager::New();
        this->m_AngleManager->SetMRMLScene( this->GetMRMLScene() );
        if (this->GetApplicationGUI()->GetActiveViewerWidget())
        {
            this->m_AngleManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
        }
        this->m_AngleManager->AddMRMLObservers();
        this->m_AngleManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
        this->m_AngleManager->Create();
    }

    vtkMRMLAnnotationAngleNode *angleNode = vtkMRMLAnnotationAngleNode::New();
    angleNode->Initialize(this->GetMRMLScene());

    if (angleNode->GetScene())
    {
        angleNode->SetName(angleNode->GetScene()->GetUniqueNameByString("AnnotationAngle"));
    }
    else
    {
        angleNode->SetName("AnnotationAngle");
    }

    angleNode->Delete();

    vtkAngleRepresentation3D* rep = vtkAngleRepresentation3D::New();
    this->m_AngleManager->GetAngleWidget( angleNode->GetID() )->GetWidget()->SetRepresentation( rep );
    this->m_AngleManager->GetAngleWidget( angleNode->GetID() )->GetWidget()->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
    this->m_AngleManager->GetAngleWidget( angleNode->GetID() )->GetWidget()->On();

    //vtkAngleRepresentation3D* rep = vtkAngleRepresentation3D::New();
    //this->GetAngleWidget( angleNode->GetID() )->GetWidget()->SetRepresentation( rep );
    //this->GetAngleWidget( angleNode->GetID() )->GetWidget()->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
    //this->GetAngleWidget( angleNode->GetID() )->GetWidget()->On();

    vtkAnnotationAngleWidgetCallback *myCallback = vtkAnnotationAngleWidgetCallback::New();
    myCallback->AngleNode = angleNode;
    myCallback->AngleWidgetClass = this->m_AngleManager->GetAngleWidget( angleNode->GetID() );
    myCallback->LogicPointer = this;
    this->m_AngleManager->GetAngleWidget(angleNode->GetID() )->GetWidget()->AddObserver(vtkCommand::InteractionEvent,myCallback);
    this->m_AngleManager->GetAngleWidget(angleNode->GetID() )->GetWidget()->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
    this->m_AngleManager->GetAngleWidget(angleNode->GetID() )->GetWidget()->AddObserver(vtkCommand::PlacePointEvent, myCallback);
    myCallback->Delete();

    return angleNode->GetID();
}

//-----------------------------------------------------------------------------
// Ruler Widget     
//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddRuler()
{    
    if (m_RulerManager == NULL)
    {
        this->m_RulerManager = vtkSlicerAnnotationRulerManager::New();
        this->m_RulerManager->SetMRMLScene( this->GetMRMLScene() );
        if (this->GetApplicationGUI()->GetActiveViewerWidget())
        {
            this->m_RulerManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
        }
        this->m_RulerManager->AddMRMLObservers();
        this->m_RulerManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
        this->m_RulerManager->Create();
    }
    
    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();
    rulerNode->Initialize(this->GetMRMLScene());

    // need a unique name since the storage node will be named from it
    if (rulerNode->GetScene())
    {
        rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));
    }
    else
    {
        rulerNode->SetName("AnnotationRuler");
    }
    rulerNode->Delete();

  this->GetViewerWidget()->Render();

    return rulerNode->GetID();
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::RemoveRuler(const char* id)
{
    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID( id ));

    if (rulerNode)
    {
        this->GetMRMLScene()->RemoveNode(rulerNode);
    }
    this->GetViewerWidget()->Render();
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode* vtkSlicermiAnnotationModuleLogic::GetRulerNodeByID(const char* id)
{
    vtkMRMLAnnotationRulerNode* rulernode = 0;
    rulernode = vtkMRMLAnnotationRulerNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID( id ));

    return rulernode;
}

//-----------------------------------------------------------------------------
// Fiducial  Widget     
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class vtkAnnotationFiducialWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationFiducialWidgetCallback *New()
  { return new vtkAnnotationFiducialWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::PlacePointEvent)
    {
      cout << "Fiducial point placed\n";
      LogicPointer->AddFiducialPicked();
    }
  }
  vtkAnnotationFiducialWidgetCallback(){}
  vtkSlicermiAnnotationModuleLogic* LogicPointer;
};

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::StartAddingFiducials()
{
  vtkMRMLInteractionNode *interactionNode = this->GetApplicationLogic()->GetInteractionNode();
  if (interactionNode == NULL)
    {
      std::cerr << "no interaction node in the scene, not updating the interaction mode!" << endl;
      return ;
    }

  interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
  interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::Place );
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::StopAddingFiducials()
{
  vtkMRMLInteractionNode *interactionNode = this->GetApplicationLogic()->GetInteractionNode();
  if (interactionNode == NULL)
    {
      std::cerr << "no interaction node in the scene, not updating the interaction mode!" << endl;
      return ;
    }
  
  interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
  interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::PickManipulate );
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* vtkSlicermiAnnotationModuleLogic::GetFiducialNodeByID(const char* id)
{
    return vtkMRMLAnnotationFiducialNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( id ));
}

//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddFiducial()
{
  if (m_FiducialManager == NULL)
  {
    this->m_FiducialManager = vtkSlicerAnnotationFiducialManager::New();
    this->m_FiducialManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_FiducialManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_FiducialManager->AddMRMLObservers();
    this->m_FiducialManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_FiducialManager->Create();
  }

  //vtkMRMLAnnotationFiducialNode *fiducialNode = vtkMRMLAnnotationFiducialNode::New();
  //fiducialNode->Initialize(this->GetMRMLScene());

  vtkMRMLFiducialListNode* fiducialNode = vtkMRMLFiducialListNode::New();
  this->GetMRMLScene()->AddNode(fiducialNode);
  fiducialNode->Delete();
  
  /*
  vtkAnnotationFiducialWidgetCallback *myCallback = vtkAnnotationFiducialWidgetCallback::New();
  myCallback->LogicPointer = this;
  this->m_FiducialManager->GetSeedWidget(fiducialNode->GetID() )->GetWidget()->AddObserver(vtkCommand::PlacePointEvent, myCallback);
  myCallback->Delete();
  */

  //return fiducialNode->GetID();
  return "";

}

//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddFiducialPicked()
{    
  if (m_FiducialManager == NULL)
  {
    this->m_FiducialManager = vtkSlicerAnnotationFiducialManager::New();
    this->m_FiducialManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_FiducialManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_FiducialManager->AddMRMLObservers();
    this->m_FiducialManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_FiducialManager->Create();
  }

  vtkMRMLAnnotationFiducialNode *fiducialNode = vtkMRMLAnnotationFiducialNode::New();
  fiducialNode->Initialize(this->GetMRMLScene());

  // need a unique name since the storage node will be named from it
  if (fiducialNode->GetScene())
  {
    fiducialNode->SetName(fiducialNode->GetScene()->GetUniqueNameByString("AnnotationFiducial"));
  }
  else
  {
    fiducialNode->SetName("AnnotationFiducial");
  }
  fiducialNode->Delete();

  return fiducialNode->GetID();
}


//-----------------------------------------------------------------------------
double vtkSlicermiAnnotationModuleLogic::GetFiducialValue(const char* cpID)
{

    if (!cpID)
    {
        std::cerr << "GetFiducialValue: no angle with ID " << cpID << " defined !" << endl;
        return 0;
    }
    this->GetViewerWidget()->Render();

    return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::RemoveFiducial(const char* id)
{
    vtkMRMLAnnotationFiducialNode *cpNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID( id ));

    this->RemoveFiducialWidget( cpNode);
    this->GetMRMLScene()->RemoveNode(cpNode);
    this->GetViewerWidget()->Render();

}

//-----------------------------------------------------------------------------
std::vector<double> vtkSlicermiAnnotationModuleLogic::GetFiducialPositionsByNodeID(const char* cpID)
{
    std::vector<double> pvector;

    if (!cpID)
    {
        std::cerr << "GetFiducial: no Fiducial with ID " << cpID << " defined !" << endl;
        return pvector;
    }

    // vtkSlicerFiducialListWidget *cpWidgetClass = this->GetFiducialWidget(cpID);
    // Kilian: This is not working right now - as it is only a place holder i removed it without debugging the code 
    // double *pos = cpWidgetClass->GetFiducialCoordinates();

    pvector.push_back(0);
    pvector.push_back(0);
    pvector.push_back(0);
    return pvector;

}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SetFiducialPositionsByNodeID(const char* cpID, std::vector<double> positions)
{
    if (!cpID)
    {
        std::cerr << "GetFiducial: no fiducial with ID " << cpID << " defined !" << endl;
        return;
    }

    if ( positions.size() < 3)
    {
        return;
    }

    double pos[3];

    pos[0] = positions[0];
    pos[1] = positions[1];
    pos[2] = positions[2];
    this->GetViewerWidget()->Render();
}

//---------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::Update3DFiducial(vtkMRMLAnnotationFiducialNode *activeCPNode)
{
    CTK_D(vtkSlicermiAnnotationModuleLogic);

    if (activeCPNode == NULL)
    {
        vtkDebugMacro("Update3DWidget: passed in fiducial node is null, returning");
        return;
    }

    if (d->Updating3DFiducial)
    {
        vtkDebugMacro("Already updating 3d widget");
        return;
    }



    d->Updating3DFiducial = 1;

    if ( activeCPNode->GetVisible() )
    {

        if ( this->GetViewerWidget() && this->GetViewerWidget()->GetMainViewer())
        {
            // Initialize point
        }
    }
    else
    {
    }

    vtkMRMLTransformNode* tnode = activeCPNode->GetParentTransformNode();
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    if (tnode != NULL && tnode->IsLinear())
    {
        vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
        lnode->GetMatrixTransformToWorld(transformToWorld);
    }


    tnode = NULL;
    transformToWorld->Delete();
    transformToWorld = NULL;

    if (this->GetViewerWidget())
    {
        this->GetViewerWidget()->RequestRender();
    }
    // reset the flag
    d->Updating3DFiducial= 0;
}

//---------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::AddFiducialWidget(vtkMRMLAnnotationFiducialNode *cpNode)
{
    CTK_D(vtkSlicermiAnnotationModuleLogic);
    if (!cpNode)
    {
        return;
    }
    if (this->GetFiducialWidget(cpNode->GetID()) != NULL)
    {
        vtkDebugMacro("Already have widgets for ruler node " << cpNode->GetID());
        return;
    }

    // Just do this as a hack right now - should be the widget class 
    vtkSlicerFiducialListWidget  *c =  vtkSlicerFiducialListWidget::New();

    d->FiducialWidgets[cpNode->GetID()] = c;
    // watch for control point modified events
    if (cpNode->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->LogicCallbackCommand) != 1)
    {
        cpNode->AddObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->LogicCallbackCommand);
    }
    // make sure we're observing the node for transform changes
    if (cpNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->LogicCallbackCommand) != 1)
    {
        cpNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->LogicCallbackCommand);
    }
    if (cpNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCallbackCommand) != 1)
    {
        cpNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget* vtkSlicermiAnnotationModuleLogic::GetFiducialWidget(const char *nodeID)
{
    CTK_D(vtkSlicermiAnnotationModuleLogic);
    // This hsould be the widget not mrml class - just do it right now bc widget does not extist
    std::map<std::string, vtkSlicerFiducialListWidget *>::iterator iter;
    for (iter = d->FiducialWidgets.begin();
        iter != d->FiducialWidgets.end();
        iter++)
    {
        if (iter->first.c_str() && !strcmp(iter->first.c_str(), nodeID))
        {
            return iter->second;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::RemoveFiducialWidget(vtkMRMLAnnotationFiducialNode *cpNode)
{
    CTK_D(vtkSlicermiAnnotationModuleLogic);
    if (!cpNode)
    {
        return;
    }
    if (this->GetFiducialWidget(cpNode->GetID()) != NULL)
    {
        d->FiducialWidgets[cpNode->GetID()]->Delete();
        d->FiducialWidgets.erase(cpNode->GetID());
    }
}

//-----------------------------------------------------------------------------
// MRML Related Set/Get Functions     
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SetAnnotationLinesProperties(vtkMRMLAnnotationLinesNode* node, int type, void* data)
{
    if (!node) 
    {
        return;
    }
    this->SetAnnotationControlPointsProperties(node, type, data);

    // Line Display Properties
    if (node->GetAnnotationLineDisplayNode()==NULL)
    {
    }
    node->CreateAnnotationLineDisplayNode();
    this->SetAnnotationLineDisplayProperties(node->GetAnnotationLineDisplayNode(), type, data);
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SetAnnotationProperties(vtkMRMLAnnotationNode* node, int type, void* data)
{
    if (!node) 
    {
        return;
    }

    switch(type)
    {
    case TEXT:
        {
            node->SetText(0, (char*)data, 0, 1);
            break;
        }
    default:
        {
            if (node->GetAnnotationTextDisplayNode()==NULL)
            {
            }
            node->CreateAnnotationTextDisplayNode();
            this->SetAnnotationTextDisplayProperties( node->GetAnnotationTextDisplayNode(), type, data);
            break;
        }
    }

}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::SetAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode* node, int type, void* data)
{
    if ((type < TEXT_COLOR) || (type > TEXT_SCALE))
    {
        return 0;
    }

    if (!node) 
    {
        return 0;
    }
  
    switch(type)
    {
    case TEXT_SCALE:
        {
            node->SetTextScale( *(double*)data );
            return 1;
        }
    default:
        { 
            return this->SetAnnotationDisplayProperties(node, type, data);
        }
    }
}

//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::SetAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode* node, int type, void* data)
{
    if (!node) 
    {
        return;
    }

    this->SetAnnotationProperties(node, type, data);
    if (node->GetAnnotationPointDisplayNode()==NULL)
    {
    }
    node->CreateAnnotationPointDisplayNode();
    this->SetAnnotationPointDisplayProperties(node->GetAnnotationPointDisplayNode(), type, data);
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::SetAnnotationPointDisplayProperties(vtkMRMLAnnotationPointDisplayNode* node, int type, void* data)
{
    if ((type < POINT_COLOR) || (type > POINT_SIZE))
    {
        return 0;
    }


    if (!node) 
    {
        return 0;
    }

    switch(type)
    {
    case POINT_SIZE:
        {
            //ToDo: set Point Size
            return 1;
        }
    default:
        return this->SetAnnotationDisplayProperties(node,type,data);  
    }
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::SetAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode* node, int type, void* data)
{
  if ((type < LINE_COLOR) || (type > LINE_WIDTH))
    {
      return 0;
    }


  if (!node) 
    {
      return 0;
    }

  switch(type)
    {
    case LINE_WIDTH:
      {
    node->SetLineThickness( *(double*)data );
    return 1;
      }
    default:
      return this->SetAnnotationDisplayProperties(node,type,data);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::SetAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode* node, int type, void* data)
{
    if (!node)
    {
        return 0;
    }

    switch(type)
    {
    case TEXT_OPACITY:
    case POINT_OPACITY:
    case LINE_OPACITY:
        node->SetOpacity( *(double*)data );
        return 1;
    case TEXT_AMBIENT:
    case POINT_AMBIENT:
    case LINE_AMBIENT:
        node->SetAmbient( *(double*)data );
        return 1;
    case TEXT_DIFFUSE:
    case POINT_DIFFUSE:
    case LINE_DIFFUSE:
        node->SetDiffuse( *(double*)data );
        return 1;
    case TEXT_SPECULAR:
    case POINT_SPECULAR:
    case LINE_SPECULAR:
        node->SetSpecular( *(double*)data );
        return 1;
    case TEXT_COLOR:
    case LINE_COLOR:
    case POINT_COLOR:
        {
            double* color = (double*)data;
            node->SetColor(color[0], color[1], color[2]);
            return 1;
        }
    case TEXT_SELECTED_COLOR:
    case POINT_SELECTED_COLOR:
    case LINE_SELECTED_COLOR:
        {
            double* color = (double*)data;
            node->SetSelectedColor(color[0], color[1], color[2]);
            return 1;
        }
    default:
        {
            std::cerr << "Property Type not found!" << endl;
            return 0;
        }
    }

}


//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetNumberOfControlPoints(vtkMRMLNode* mrmlnode)
{
  vtkMRMLAnnotationControlPointsNode* node = vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);
  if (!node) 
    {
      return -1;
    }
  return node->GetNumberOfControlPoints();
}

//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::GetIconName(vtkMRMLNode* node, bool isEdit)
{
    if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
        if (isEdit)
        {
            return ":/Icons/AnnotationEditPoint.png";
        } 
        else
        {
            return ":/Icons/AnnotationPoint.png";
        }
    }
  else if (node->IsA("vtkMRMLFiducialListNode"))
  {
    if (isEdit)
    {
      return ":/Icons/AnnotationEditPoint.png";
    } 
    else
    {
      return ":/Icons/AnnotationPoint.png";
    }
  }
    else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
        if (isEdit)
        {
            return ":/Icons/AnnotationEditDistance.png";
        } 
        else
        {
            return ":/Icons/AnnotationDistance.png";
        }
    }
    else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
        if (isEdit)
        {
            return ":/Icons/AnnotationEditAngle.png";
        } 
        else
        {
            return ":/Icons/AnnotationAngle.png";
        }
    }
    else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {
        if (isEdit)
        {
            return ":/Icons/AnnotationEditNote.png";
        } 
        else
        {
            return ":/Icons/AnnotationNote.png";
        }
    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
  {
    if (isEdit)
    {
      return ":/Icons/AnnotationEditText.png";
    } 
    else
    {
      return ":/Icons/AnnotationText.png";
    }
  }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
  {
    if (isEdit)
    {
      return ":/Icons/AnnotationEditROI.png";
    } 
    else
    {
      return ":/Icons/AnnotationROI.png";
    }
  }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
  {
    if (isEdit)
    {
      return ":/Icons/AnnotationEditPolyline.png";
    } 
    else
    {
      return ":/Icons/AnnotationPolyline.png";
    }
  }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
  {
    if (isEdit)
    {
      return ":/Icons/AnnotationEditSpline.png";
    } 
    else
    {
      return ":/Icons/AnnotationSpline.png";
    }
  }
  

  
    return NULL;
} 

//-----------------------------------------------------------------------------
vtkStdString vtkSlicermiAnnotationModuleLogic::GetAnnotationTextProperty(vtkMRMLNode* node)
{
    vtkMRMLAnnotationNode *aNode = vtkMRMLAnnotationNode::SafeDownCast(node);
    if (!aNode)
    {
        return "";
    } 
    return aNode->GetText(0);
}

const char* vtkSlicermiAnnotationModuleLogic::GetAnnotationTextFormatProperty(vtkMRMLNode* node)
{
    if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    return " ";
    }
  if (node->IsA("vtkMRMLFiducialListNode"))
  {
    return " ";
  }
    else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
        return vtkMRMLAnnotationRulerNode::SafeDownCast(node)->GetDistanceAnnotationFormat();
    }
    else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
        return vtkMRMLAnnotationAngleNode::SafeDownCast(node)->GetLabelFormat();
    }
    else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {
        return " ";
    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
  {
    return " ";
  }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
  {
    return vtkMRMLAnnotationROINode::SafeDownCast(node)->GetROIAnnotationFormat();
  }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
  {
    return vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetAnnotationFormat();
  }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
  {
    return vtkMRMLAnnotationSplineNode::SafeDownCast(node)->GetDistanceAnnotationFormat();
  }

    return NULL;
}

//-----------------------------------------------------------------------------
std::vector<double> vtkSlicermiAnnotationModuleLogic::GetAnnotationMeasurement(vtkMRMLNode* node)
{
  std::vector<double> measurements;

  if (node->IsA("vtkMRMLAnnotationFiducialNode"))
  {
    measurements.push_back(0);
  }
  if (node->IsA("vtkMRMLFiducialListNode"))
  {
    measurements.push_back(0);
  }
  else if (node->IsA("vtkMRMLAnnotationRulerNode"))
  {
    measurements.push_back(vtkMRMLAnnotationRulerNode::SafeDownCast(node)->GetDistanceMeasurement());
  }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
  {
    measurements.push_back(vtkMRMLAnnotationAngleNode::SafeDownCast(node)->GetAngleMeasurement());
  }
  else if (node->IsA("vtkMRMLAnnotationStickyNode"))
  {
    measurements.push_back(0);
  }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
  {
    measurements.push_back(0);
  }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
  {
    measurements = vtkMRMLAnnotationROINode::SafeDownCast(node)->GetROIMeasurement();
  }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
  {
    measurements = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement();
  }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
  {
    measurements.push_back(vtkMRMLAnnotationSplineNode::SafeDownCast(node)->GetSplineMeasurement());
  }

    return measurements;
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, vtkIdType coordId)
{
    vtkMRMLAnnotationControlPointsNode* node = vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);

    if (!node || (coordId >= node->GetNumberOfControlPoints()))
    {
        return NULL;
    }

    return node->GetControlPointCoordinates(coordId);
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::SetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, double* pos, vtkIdType coordId)
{
    vtkMRMLAnnotationControlPointsNode* node = vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);

    if (!node || (coordId >= node->GetNumberOfControlPoints()))
    {
        return NULL;
    }

    node->SetControlPoint(coordId, pos, 0, 1);

    return 1;
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationLinesPropertiesDouble(vtkMRMLNode* node, int type, double &result)
{
    if (!node)
    {
        return NULL;
    }
    vtkMRMLAnnotationLinesNode* lnode = vtkMRMLAnnotationLinesNode::SafeDownCast(node);
    if (!lnode)
    {
        return NULL;
    }

     if (this->GetAnnotationControlPointsPropertiesDouble((vtkMRMLAnnotationControlPointsNode*)lnode, type, result) )
    {
        return 1;
    } 

    lnode->CreateAnnotationLineDisplayNode();
    if (this->GetAnnotationLineDisplayPropertiesDouble(lnode->GetAnnotationLineDisplayNode(), type, result) )
    {
        return 1;
    } 
    return 0;
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationControlPointsPropertiesDouble(vtkMRMLAnnotationControlPointsNode* node, int type, double &result)
{
    if (!node)
    {
        return NULL;
    }

    if ( this->GetAnnotationPropertiesDouble(node, type, result) )
    {
        return 1;
    }

    node->CreateAnnotationPointDisplayNode();
    if (this->GetAnnotationPointDisplayPropertiesDouble(node->GetAnnotationPointDisplayNode(), type, result) )
    {
        return 1;
    } 
    return NULL;

}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationLineDisplayPropertiesDouble(vtkMRMLAnnotationLineDisplayNode* node, int type, double& result)
{
    if ((type < LINE_COLOR) || (type > LINE_WIDTH))
    {
        return 0;
    }

    if (!node) 
    {
        return 0;
    }

    switch(type)
    {
    case LINE_WIDTH:
        {
            result = node->GetLineThickness();
            return 1;
        }
    default: 
        return this->GetAnnotationDisplayPropertiesDouble(node, type, result);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationDisplayPropertiesDouble(vtkMRMLAnnotationDisplayNode* node, int type, double& result)
{
    if (!node)
    {
        return 0;
    }

    switch(type)
    {
    case POINT_OPACITY:
    case LINE_OPACITY:
    case TEXT_OPACITY:
        result = node->GetOpacity();
        return 1;
    case POINT_AMBIENT:
    case LINE_AMBIENT:
    case TEXT_AMBIENT:
        result = node->GetAmbient();
        return 1;
    case POINT_DIFFUSE:
    case LINE_DIFFUSE:
    case TEXT_DIFFUSE:
        result = node->GetDiffuse();
        return 1;
    case POINT_SPECULAR:
    case LINE_SPECULAR:
    case TEXT_SPECULAR:
        result = node->GetSpecular();
        return 1;
    default:
        {
            std::cerr << "Property Type not found!" << endl;
            return 0;
        }
    }
}


//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationPropertiesDouble(vtkMRMLAnnotationNode* node, int type, double& result)
{
    if (!node) 
    {
        return NULL;
    }

    node->CreateAnnotationTextDisplayNode();
    if ( this->GetAnnotationTextDisplayPropertiesDouble(node->GetAnnotationTextDisplayNode(), type, result) )
    {
        return 1;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationTextDisplayPropertiesDouble(vtkMRMLAnnotationTextDisplayNode* node, int type, double& result)
{
 if ((type < TEXT_COLOR) || (type > TEXT_SCALE))
    {
      return 0;
    }

 if (!node) 
   {
     return 0;
   }
 
 switch(type)
   {
   case TEXT_SCALE:
     result = node->GetTextScale();
     return 1;
   default:
     return this->GetAnnotationDisplayPropertiesDouble(node, type, result);    
   }
}

//-----------------------------------------------------------------------------
int vtkSlicermiAnnotationModuleLogic::GetAnnotationPointDisplayPropertiesDouble(vtkMRMLAnnotationPointDisplayNode* node, int type, double& result)
{
    if ((type < POINT_COLOR) || (type > POINT_SIZE))
    {
        return 0;
    }

    if (!node) 
    {
        return 0;
    }

    switch(type)
    {
    case POINT_SIZE:
        {
            //ToDo: get Point Size
            return 1;
        }
    default:
        return this->GetAnnotationDisplayPropertiesDouble(node,type,result);
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationLinesPropertiesColor(vtkMRMLNode* mrmlnode, int type)
{
    vtkMRMLAnnotationLinesNode* lnode = vtkMRMLAnnotationLinesNode::SafeDownCast(mrmlnode);
    if (!lnode) 
    {
        return 0;
    }

    switch(type)
    {
    case LINE_COLOR:
    case LINE_SELECTED_COLOR:
        {
            lnode->CreateAnnotationLineDisplayNode();
            return this->GetAnnotationLineDisplayPropertiesColor(lnode->GetAnnotationLineDisplayNode(), type);
        }
    default:
        {
            return this->GetAnnotationControlPointsPropertiesColor((vtkMRMLAnnotationControlPointsNode*)lnode, type);
        }
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationLineDisplayPropertiesColor(vtkMRMLAnnotationLineDisplayNode* node, int type)
{
    if ((type != LINE_COLOR) && (type !=LINE_SELECTED_COLOR))
    {
        return NULL;
    }

    if (!node) 
    {
        return NULL;
    }
    return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationControlPointsPropertiesColor(vtkMRMLAnnotationControlPointsNode* node, int type)
{
    if (!node)
    {
        return NULL;
    }

    switch(type)
    {
    case POINT_COLOR:
    case POINT_SELECTED_COLOR:
        {
            node->CreateAnnotationPointDisplayNode();
            return this->GetAnnotationPointDisplayPropertiesColor(node->GetAnnotationPointDisplayNode(), type);
        }
    default:
        {
          return this->GetAnnotationPropertiesColor(node, type);
        }
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationPropertiesColor(vtkMRMLAnnotationNode* node, int type)
{
    if (!node) 
    {
        return NULL;
    }
    node->CreateAnnotationTextDisplayNode();
    return this->GetAnnotationTextDisplayPropertiesColor(node->GetAnnotationTextDisplayNode(), type);
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationTextDisplayPropertiesColor(vtkMRMLAnnotationTextDisplayNode* node, int type)
{
    if ((type != TEXT_COLOR) && (type !=TEXT_SELECTED_COLOR))
    {
        return NULL;
    }
    if (!node) 
    {
        return NULL;
    }
    return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationPointDisplayPropertiesColor(vtkMRMLAnnotationPointDisplayNode* node, int type)
{
    if ((type != POINT_COLOR) && (type !=POINT_SELECTED_COLOR))
    {
        return NULL;
    }
    if (!node) 
    {
        return NULL;
    }
    return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicermiAnnotationModuleLogic::GetAnnotationDisplayPropertiesColor(vtkMRMLAnnotationDisplayNode* node, int type)
{
    if (!node) 
    {
        return NULL;
    }
    switch(type)
    {
    case TEXT_COLOR:
    case POINT_COLOR:
    case LINE_COLOR:
        {
            return node->GetColor();
        }
    case TEXT_SELECTED_COLOR:
    case POINT_SELECTED_COLOR:
    case LINE_SELECTED_COLOR:
        {
            return node->GetSelectedColor();
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------
// Sticky Node
//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddStickyNode()
{    
    vtkMRMLAnnotationStickyNode *stickyNode = vtkMRMLAnnotationStickyNode::New();
    stickyNode->Initialize(this->GetMRMLScene());

    // need a unique name since the storage node will be named from it
    if (stickyNode->GetScene())
    {
        stickyNode->SetName(stickyNode->GetScene()->GetUniqueNameByString("AnnotationStickyNode"));
    }
    else
    {
        stickyNode->SetName("AnnotationRuler");
    }
    stickyNode->Delete();

    return stickyNode->GetID();
}

//-----------------------------------------------------------------------------
// Text Node
//-----------------------------------------------------------------------------
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    // save node for undo if it's the start of an interaction event

    if (event == vtkCommand::PlacePointEvent)
    {
      cout << "Text Node Point Placed\n";
      LogicPointer->AddTextNodeCompleted();
    }
  }
  //,DistanceRepresentation(0)
  vtkAnnotationTextWidgetCallback():textNode(0), textWidget(0) {}
  vtkMRMLAnnotationTextNode *textNode;
  vtkTextWidget* textWidget;
  vtkSlicermiAnnotationModuleLogic* LogicPointer;
};

const char* vtkSlicermiAnnotationModuleLogic::AddTextNode()
{    
  if (m_TextManager == NULL)
  {
    this->m_TextManager = vtkSlicerAnnotationTextManager::New();
    this->m_TextManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_TextManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_TextManager->AddMRMLObservers();
    this->m_TextManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_TextManager->Create();
  }

  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
  textNode->Initialize(this->GetMRMLScene());

  // need a unique name since the storage node will be named from it
  if (textNode->GetScene())
  {
    textNode->SetName(textNode->GetScene()->GetUniqueNameByString("AnnotationText"));
  }
  else
  {
    textNode->SetName("AnnotationText");
  }
  textNode->Delete();

  /*
  if ( node->IsA("vtkMRMLAnnotationTextNode") )
  {
    vtkTextWidget* textWidget = this->GetTextWidget(node->GetID());
    vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation())->SetText((char*)data);
    this->GetViewerWidget()->Render();
    return;
  }
  */

  /*
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
  myCallback->textNode = textNode;
  myCallback->textWidget = textWidget;
  myCallback->LogicPointer = this;
  textWidget->AddObserver(vtkCommand::PlacePointEvent, myCallback);
  myCallback->Delete();
  */
  return textNode->GetID();
}

//---------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::AddTextNodeCompleted()
{
  this->InvokeEvent(vtkSlicermiAnnotationModuleLogic::AddTextNodeCompletedEvent, NULL);
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationTextNode* vtkSlicermiAnnotationModuleLogic::GetTextNodeByID(const char* id)
{
  vtkMRMLAnnotationTextNode* textNode = 0;
  textNode = vtkMRMLAnnotationTextNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( id ));
  return textNode;
}


//-----------------------------------------------------------------------------
// ROI Node
//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddROINode()
{   
  //vtkSlicerROIGUI* roi = vtkSlicerROIGUI::SafeDownCast(vtkSlicerApplication::GetInstance()->GetModuleGUIByName("ROI"));
  //vtkSlicerROIDisplayWidget* roiWidget = vtkSlicerROIDisplayWidget::SafeDownCast(roi->GetApplicationGUI()->GetActiveViewerWidget());
  //vtkMRMLROINode *roiNode = vtkMRMLROINode::New();

  if (m_ROIManager == NULL)
  {
    this->m_ROIManager = vtkSlicerAnnotationROIManager::New();
    this->m_ROIManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_ROIManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_ROIManager->AddMRMLObservers();
    this->m_ROIManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_ROIManager->Create();
  }
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();
  roiNode->Initialize(this->GetMRMLScene());

  // need a unique name since the storage node will be named from it
  if (roiNode->GetScene())
  {
    roiNode->SetName(roiNode->GetScene()->GetUniqueNameByString("AnnotationROINode"));
  }
  else
  {
    roiNode->SetName("AnnotationROI");
  }
  //roiNode->SetLabelText("");
  roiNode->Delete();
  //roiWidget->SetROINode(roiNode);

  //vtkSlicerROIDisplayWidget* ROIWidget = vtkSlicerROIDisplayWidget::New();
  //ROIWidget->SetROINode(roiNode);
  //roiWidget->SetParent(this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent());
  //roiWidget->Create();

  return roiNode->GetID();
}

//-----------------------------------------------------------------------------
// BiDimentional Line Node
//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddBidLineNode()
{
  /*
  vtkBiDimensionalRepresentation2D* bidRep = vtkBiDimensionalRepresentation2D::New();
  vtkBiDimensionalWidget* bidWidget = vtkBiDimensionalWidget::New();
  bidWidget->SetRepresentation(bidRep);
  bidWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
  bidWidget->On();
  vtkMRMLOrthogonalLinePlotNode *node = vtkMRMLOrthogonalLinePlotNode::New();
  this->GetMRMLScene()->AddNode(node);
  */

  if (m_BidimensionalManager == NULL)
  {
    this->m_BidimensionalManager = vtkSlicerAnnotationBidimensionalManager::New();
    this->m_BidimensionalManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_BidimensionalManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_BidimensionalManager->AddMRMLObservers();
    this->m_BidimensionalManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_BidimensionalManager->Create();
  }

  vtkMRMLAnnotationBidimensionalNode *node = vtkMRMLAnnotationBidimensionalNode::New();
  node->Initialize(this->GetMRMLScene());

  // need a unique name since the storage node will be named from it
  if (node->GetScene())
  {
    node->SetName(node->GetScene()->GetUniqueNameByString("AnnotationPolylineNode"));
  }
  else
  {
    node->SetName("AnnotationROI");
  }
  node->Delete();

  return node->GetID();
}

//-----------------------------------------------------------------------------
// Spline Node
//-----------------------------------------------------------------------------
const char* vtkSlicermiAnnotationModuleLogic::AddSplineNode()
{   
  /*
  vtkSplineRepresentation* sRep = vtkSplineRepresentation::New();
  vtkSplineWidget* sWidget = vtkSplineWidget::New();
  sWidget->PlaceWidget();
  sWidget->ProjectToPlaneOn();
  sWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
  sWidget->On();
  vtkMRMLBSplineTransformNode *node = vtkMRMLBSplineTransformNode::New();
  this->GetMRMLScene()->AddNode(node);
  */

  if (m_SplineManager == NULL)
  {
    this->m_SplineManager = vtkSlicerAnnotationSplineManager::New();
    this->m_SplineManager->SetMRMLScene( this->GetMRMLScene() );
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
      this->m_SplineManager->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
    this->m_SplineManager->AddMRMLObservers();
    this->m_SplineManager->SetParent ( this->GetApplicationGUI()->GetActiveViewerWidget()->GetParent() );
    this->m_SplineManager->Create();
  }

  vtkMRMLAnnotationSplineNode *node = vtkMRMLAnnotationSplineNode::New();
  node->Initialize(this->GetMRMLScene());

  // need a unique name since the storage node will be named from it
  if (node->GetScene())
  {
    node->SetName(node->GetScene()->GetUniqueNameByString("AnnotationSpline"));
  }
  else
  {
    node->SetName("AnnotationSpline");
  }
  node->Delete();

  return node->GetID();
}


//-----------------------------------------------------------------------------
// This is for testing right now 
//-----------------------------------------------------------------------------
void vtkSlicermiAnnotationModuleLogic::AddRulerNodeObserver(vtkMRMLAnnotationRulerNode* rnode) 
{
    if (rnode->HasObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->LogicCallbackCommand) != 1)
    {
        rnode->AddObserver(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, (vtkCommand *)this->LogicCallbackCommand);
    }
    if (rnode->GetAnnotationTextDisplayNode()==NULL)
    {
        rnode->CreateAnnotationTextDisplayNode();
    }
    if (rnode->GetAnnotationTextDisplayNode()->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCallbackCommand) != 1)
    {
    rnode->GetAnnotationTextDisplayNode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCallbackCommand);
    }
}
