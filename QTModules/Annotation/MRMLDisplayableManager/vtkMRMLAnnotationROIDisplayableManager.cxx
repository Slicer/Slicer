// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationROIDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationROINode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>

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
class vtkAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationROIWidgetCallback *New()
  { return new vtkAnnotationROIWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
  }
  vtkAnnotationROIWidgetCallback(){}
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
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return 0;
    }

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

  vtkBoxWidget2* boxWidget = vtkBoxWidget2::New();

  boxWidget->SetInteractor(this->GetInteractor());
  boxWidget->SetCurrentRenderer(this->GetRenderer());

  boxWidget->CreateDefaultRepresentation();

  VTK_CREATE(vtkBoxRepresentation,boxRepresentation);

  boxRepresentation = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  double * origin = roiNode->GetXYZ();
  double * radius = roiNode->GetRadiusXYZ();

  double d = sqrt((origin[0]-radius[0])*(origin[0]-radius[0])+(origin[1]-radius[1])*(origin[1]-radius[1])+(origin[2]-radius[2])*(origin[2]-radius[2]));

  double bounds[6];
  bounds[0]=origin[0]-d;
  bounds[1]=origin[0]+d;
  bounds[2]=origin[1]-d;
  bounds[3]=origin[1]+d;
  bounds[4]=origin[2]-d;
  bounds[5]=origin[2]+d;

  boxRepresentation->PlaceWidget(bounds);


  boxWidget->ProcessEventsOff();

  boxRepresentation->EndWidgetInteraction(origin);

  return boxWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationROIDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // nothing yet
}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationROIDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // nothing yet
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationROIDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    vtkHandleWidget *h1 = this->m_HandleWidgetList[0];
    vtkHandleWidget *h2 = this->m_HandleWidgetList[1];

    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetWorldPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetWorldPosition();

    // create the MRML node
    vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();

    roiNode->SetXYZ(position1);
    roiNode->SetRadiusXYZ(position2);

    roiNode->Initialize(this->GetMRMLScene());

    roiNode->SetName(roiNode->GetScene()->GetUniqueNameByString("AnnotationROI"));

    roiNode->Delete();

    }

  }
