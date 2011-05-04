// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationFiducialDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationNode.h"
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSeedWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>
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
/// \ingroup Slicer_QtModules_Annotation
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


      if (this->m_DisplayableManager->GetSliceNode())
        {

        // if this is a 2D SliceView displayableManager, restrict the widget to the renderer

        // we need the widgetRepresentation
        vtkSeedRepresentation * representation = vtkSeedRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

        double displayCoordinates1[4];

        // first, we get the current displayCoordinates of the points
        representation->GetSeedDisplayPosition(0,displayCoordinates1);

        // second, we copy these to restrictedDisplayCoordinates
        double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};

        // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);

        // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
          {
          representation->SetSeedDisplayPosition(0,restrictedDisplayCoordinates1);
          }

        }

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

  VTK_CREATE(vtkSeedRepresentation, rep);

  // use different representations for 2D and 3D
  if (this->GetSliceNode())
    {
    VTK_CREATE(vtkPointHandleRepresentation2D, handle);
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(10);

    rep->SetHandleRepresentation(handle);
    }
  else
    {
    VTK_CREATE(vtkPointHandleRepresentation3D, handle);
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(10);

    rep->SetHandleRepresentation(handle);
    }


  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep);

  seedWidget->SetInteractor(this->GetInteractor());
  seedWidget->SetCurrentRenderer(this->GetRenderer());

  //seedWidget->ProcessEventsOff();



  // Use tmpPtr to query only one time for the coordinates
  double* tmpPtr = fiducialNode->GetFiducialCoordinates();

  double worldCoordinates[4] = {0.0, 0.0, 0.0, 1.0};
  if (tmpPtr != NULL)
    {
    worldCoordinates[0] = tmpPtr[0];
    worldCoordinates[1] = tmpPtr[1];
    worldCoordinates[2] = tmpPtr[2];
    }

  double position1[4];

  if (this->GetSliceNode())
    {

    bool showWidget = true;
    showWidget = this->IsWidgetDisplayable(this->GetSliceNode(), node);

    // TODO: Is the on two times necessary?
    if (showWidget)
      {
      seedWidget->On();
      seedWidget->CompleteInteraction();
      }

    this->GetWorldToDisplayCoordinates(worldCoordinates,position1);

    vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
    vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(position1);

    if (showWidget)
      {
      seedWidget->On();
      seedWidget->CompleteInteraction();
      }


    }
  else
    {

    // TODO: Is the on two times necessary?
    seedWidget->On();
    seedWidget->CompleteInteraction();

    vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
    vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetWorldPosition(worldCoordinates);

    seedWidget->On();
    seedWidget->CompleteInteraction();

    }

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

  // store the current view
  node->SaveView();

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

  seedWidget->CompleteInteraction();

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

  fiducialNode->SaveView();

  seedWidget->CompleteInteraction();

  // enable processing of modified events
  fiducialNode->DisableModifiedEventOff();

  fiducialNode->Modified();
  fiducialNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, fiducialNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationFiducialDisplayableManager::OnClickInRenderWindow(double x, double y)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: jumping out");
    return;
    }

  // place the seed where the user clicked
  vtkDebugMacro("OnClickInRenderWindow: placing seed at " << x << ", " << y);

  // switch to updating state to avoid events mess
  this->m_Updating = 1;

  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;


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

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

}

