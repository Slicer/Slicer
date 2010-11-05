// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationFiducialDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationNode.h"
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSeedWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkAbstractWidget.h>
#include <vtkMatrix4x4.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationFiducialDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationFiducialDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationFiducialDisplayableManager Callback
class vtkAnnotationFiducialWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationFiducialWidgetCallback *New()
  { return new vtkAnnotationFiducialWidgetCallback; }

  vtkAnnotationFiducialWidgetCallback(){}

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
      this->m_DisplayableManager->PropagateWidgetToMRML(this->m_Widget, this->m_Node);

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
// vtkMRMLAnnotationFiducialDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationFiducialDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("CreateWidget: Could not get fiducial node!")
    return 0;
    }

  VTK_CREATE(vtkPointHandleRepresentation2D, handle);
  handle->GetProperty()->SetColor(1,0,0);
  handle->SetHandleSize(10);

  VTK_CREATE(vtkSeedRepresentation, rep);
  rep->SetHandleRepresentation(handle);

  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep);

  seedWidget->SetInteractor(this->GetInteractor());
  seedWidget->SetCurrentRenderer(this->GetRenderer());

  //seedWidget->ProcessEventsOff();
  seedWidget->CompleteInteraction();

  seedWidget->On();

  double worldCoordinates[4];
  worldCoordinates[0] = fiducialNode->GetFiducialCoordinates()[0];
  worldCoordinates[1] = fiducialNode->GetFiducialCoordinates()[1];
  worldCoordinates[2] = fiducialNode->GetFiducialCoordinates()[2];
  worldCoordinates[3] = 1;

  double position1[4];

  if (this->GetSliceNode())
    {

    this->GetWorldToDisplayCoordinates(worldCoordinates,position1);

    VTK_CREATE(vtkHandleWidget, newhandle);
    newhandle = seedWidget->CreateNewHandle();
    vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(position1);

    }
  else
    {

    VTK_CREATE(vtkHandleWidget, newhandle);
    newhandle = seedWidget->CreateNewHandle();
    vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetWorldPosition(worldCoordinates);

    }

  seedWidget->CompleteInteraction();
  seedWidget->On();

  //seedWidget->CompleteInteraction();

  return seedWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationFiducialDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  // add the callback
  vtkAnnotationFiducialWidgetCallback *myCallback = vtkAnnotationFiducialWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  //this->m_Updating = 0;
  //this->PropagateWidgetToMRML(widget, node);
}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationFiducialDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get seed widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get fiducial node!")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  double displayCoordinates1[4];
  double displayCoordinatesBuffer1[4];

  seedRepresentation->GetSeedDisplayPosition(0,displayCoordinatesBuffer1);

  this->GetWorldToDisplayCoordinates(fiducialNode->GetControlPointCoordinates(0),displayCoordinates1);

  if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
    {
    // only update when really changed
    seedRepresentation->SetSeedDisplayPosition(0,displayCoordinates1);
    }

  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;


}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationFiducialDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get seed widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get fiducial node!")
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;
  fiducialNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  double worldCoordinates1[4];

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    double displayCoordinates1[4];
    seedRepresentation->GetSeedDisplayPosition(0,displayCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

    }
  else
    {

    seedRepresentation->GetSeedWorldPosition(0,worldCoordinates1);

    }

  fiducialNode->SetFiducialCoordinates(worldCoordinates1);

  // enable processing of modified events
  this->m_Updating = 0;
  fiducialNode->DisableModifiedEventOff();

  fiducialNode->Modified();
  fiducialNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, fiducialNode);

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationFiducialDisplayableManager::OnClickInRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(1))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkHandleWidget *h1 = this->GetSeed(0);

    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4];

    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

    // create the MRML node
    vtkMRMLAnnotationFiducialNode *fiducialNode = vtkMRMLAnnotationFiducialNode::New();

    fiducialNode->SetFiducialCoordinates(worldCoordinates1);

    fiducialNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationFiducial"));

    fiducialNode->Initialize(this->GetMRMLScene());

    fiducialNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
