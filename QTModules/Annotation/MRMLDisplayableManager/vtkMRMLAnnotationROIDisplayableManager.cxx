// AnnotationModule/Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// AnnotationModule/MRML includes
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>

// AnnotationModule/DisplayableManager includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkMRMLAnnotationROIDisplayableManager.h"

// AnnotationModule/VTKWidgets includes
#include <vtkAnnotationROIRepresentation.h>
#include <vtkAnnotationROIWidget.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkAbstractWidget.h>

// std includes
#include <string>
#include <math.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationROIDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationROIDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationROIDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationROIWidgetCallback *New()
  { return new vtkAnnotationROIWidgetCallback; }

  vtkAnnotationROIWidgetCallback(){}


  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
  {
    if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {

      // sanity checks
      if (!this->m_DisplayableManager)
        {
        return;
        }
      if (!this->m_Node)
        {
        return;
        }
      if (!this->m_Widget)
        {
        return;
        }
      // sanity checks end

      // the interaction with the widget ended, now propagate the changes to MRML
      vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(m_Node);
      if (roiNode->GetInteractiveMode() || 
           (!roiNode->GetInteractiveMode() && event == vtkCommand::EndInteractionEvent))
        {
        this->m_DisplayableManager->PropagateWidgetToMRML(this->m_Widget, this->m_Node);
        }
    }
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkMRMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkMRMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationROIDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROIDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationROIDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("CreateWidget: Could not get ROI node!")
    return 0;
    }

  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::New();

  boxWidget->SetInteractor(this->GetInteractor());
  boxWidget->SetCurrentRenderer(this->GetRenderer());

  boxWidget->SetRotationEnabled(0);

  boxWidget->CreateDefaultRepresentation();

  vtkAnnotationROIRepresentation *boxRepresentation = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  boxRepresentation->SetPlaceFactor(1.0);

  // we place the widget around 0,0,0
  double bounds[6] = {0,0,0,0,0,0};
  bounds[0]=-1;
  bounds[1]=1;
  bounds[2]=-1;
  bounds[3]=1;
  bounds[4]=-1;
  bounds[5]=1;

  boxRepresentation->PlaceWidget(bounds);

  boxRepresentation->NeedToRenderOn();
  //this->RequestRender();
  // end of starting placement

  // now, we move the widget to the position according to the MRML node
  // this is saved as origin and radius
  // we need to get the transformation matrix of the boxRepresentation
  // and modify it according to MRML

  vtkTransform* transform = vtkTransform::New();
  boxRepresentation->GetTransform(transform);

  vtkMatrix4x4* matrix = vtkMatrix4x4::New();

  //std::cout << "Now coming the matrix from rep before modific." << std::endl;
  //transform->PrintSelf(std::cout,vtkIndent(1));

  // we need to save the origin here in the matrix
  //double origin[3];
  double origin[3];
  int originRetVal = roiNode->GetXYZ(origin);
  double radius[3];
  int radiusRetVal = roiNode->GetRadiusXYZ(radius);

  if(!originRetVal || !radiusRetVal)
    {
      vtkErrorMacro("CreateWidget: Failed to initialize a new ROI node!");
      return NULL;
    }

  matrix->SetElement(0,3,origin[0]);
  matrix->SetElement(1,3,origin[1]);
  matrix->SetElement(2,3,origin[2]);

  // we need to save the radius here in the matrix
  //double radius[3];
  matrix->SetElement(0,0,radius[0]*2.0);
  matrix->SetElement(1,1,radius[1]*2.0);
  matrix->SetElement(2,2,radius[2]*2.0);

  //matrix->DeepCopy(newMatrix);

  //std::cout << "Just the matrix after modific." << std::endl;
  //matrix->PrintSelf(std::cout,vtkIndent(1));

  transform->SetMatrix(matrix);

  //std::cout << "Now coming the transform after new matrix but before setting to rep" << std::endl;
  //transform->PrintSelf(std::cout,vtkIndent(1));

  boxRepresentation->SetTransform(transform);

  //vtkTransform* transformAfterSet = vtkTransform::New();
  //boxRepresentation->GetTransform(transformAfterSet);
  //std::cout << "Now coming the transform from rep" << std::endl;
  //transformAfterSet->PrintSelf(std::cout,vtkIndent(1));

  //boxWidget->ProcessEventsOff();

  boxRepresentation->EndWidgetInteraction(origin);
  boxRepresentation->NeedToRenderOn();

  boxWidget->On();

  matrix->Delete();
  transform->Delete();

  return boxWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationROIDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!")
    return;
    }

  vtkAnnotationROIWidgetCallback *myCallback = vtkAnnotationROIWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  node->SaveView();

}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationROIDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ROI node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());
  
  if (rep) 
    {
    // update widget from mrml
    double xyz[3];
    double rxyz[3];
    roiNode->GetXYZ(xyz);
    roiNode->GetRadiusXYZ(rxyz);

    double bounds[6];
    for (int i=0; i<3; i++)
      {
      bounds[  i] = xyz[i]-rxyz[i];
      bounds[3+i] = xyz[i]+rxyz[i];
      }
    double b[6];
    b[0] = bounds[0];
    b[1] = bounds[3];
    b[2] = bounds[1];
    b[3] = bounds[4];
    b[4] = bounds[2];
    b[5] = bounds[5];

    rep->PlaceWidget(b);
    }

  this->SetParentTransformToWidget(roiNode, boxWidget);

  // re-render the widget
  rep->NeedToRenderOn();
  boxWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationROIDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ROI node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;
  roiNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  double extents[3];
  double center[3];
  rep->GetExtents(extents);
  rep->GetCenter(center);

  this->GetWorldToLocalCoordinates(roiNode, center, center);

  roiNode->SetXYZ(center[0], center[1], center[2] );
  roiNode->SetRadiusXYZ(0.5*extents[0], 0.5*extents[1], 0.5*extents[2] );

  // save the current view
  roiNode->SaveView();

  // enable processing of modified events
  roiNode->DisableModifiedEventOff();
  roiNode->Modified();

  roiNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, roiNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationROIDisplayableManager::OnClickInRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(2))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double origin[4] = {0,0,0,0};
    double radius[4] = {0,0,0,0};

    this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],origin);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],radius);

    double rx = sqrt((origin[0]-radius[0])*(origin[0]-radius[0]));
    if (rx==0)
      {
      rx = 100;
      }
    double ry = sqrt((origin[1]-radius[1])*(origin[1]-radius[1]));
    if (ry==0)
      {
      ry = rx;
      }
    double rz = sqrt((origin[2]-radius[2])*(origin[2]-radius[2]));
    if (rz==0)
      {
      rz = rx;
      }


    // done calculating the bounds

    // create the MRML node
    vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();

    roiNode->SetXYZ(origin);
    roiNode->SetRadiusXYZ(rx,ry,rz);

    roiNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationROI"));

    roiNode->Initialize(this->GetMRMLScene());

    roiNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }
    }

  }

void vtkMRMLAnnotationROIDisplayableManager::SetParentTransformToWidget(vtkMRMLAnnotationNode *node, vtkAbstractWidget *widget)
{
  if (!node || !widget)
    {
    return;
    }

  vtkAnnotationROIRepresentation *rep = vtkAnnotationROIRepresentation::SafeDownCast(widget->GetRepresentation());

  // get the nodes's transform node
  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  if (rep != NULL && tnode != NULL && tnode->IsLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    transformToWorld->Identity();
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);

    //vtkSmartPointer<vtkTransform> xform =  vtkSmartPointer<vtkTransform>::New();
    //xform->Identity();
    //xform->SetMatrix(transformToWorld);

    vtkSmartPointer<vtkPropCollection> actors =  vtkSmartPointer<vtkPropCollection>::New();
    rep->GetActors(actors);
    
    for (int i=0; i<actors->GetNumberOfItems(); i++)
      {
      vtkActor *actor = vtkActor::SafeDownCast(actors->GetItemAsObject(i));
      actor->SetUserMatrix(transformToWorld);
      }

    //rep->SetTransform(xform);

    widget->InvokeEvent(vtkCommand::EndInteractionEvent);
    }
}

