// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationAngleDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLSelectionNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkAngleRepresentation3D.h>
#include <vtkAngleWidget.h>
#include <vtkHandleWidget.h>
#include <vtkHandleRepresentation.h>


// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationAngleDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationAngleDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationAngleDisplayableManager Callback
class vtkAnnotationAngleWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationAngleWidgetCallback *New()
  { return new vtkAnnotationAngleWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
  }
  vtkAnnotationAngleWidgetCallback(){}
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationAngleDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationAngleDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{
  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLSelectionNode"));
  if ( selectionNode == NULL )
    {
    vtkErrorMacro ( "OnClickInThreeDRenderWindow: No selection node in the scene." );
    return 0;
    }


  if (!strcmp(selectionNode->GetActiveAnnotationID(), "vtkMRMLAnnotationAngleNode"))
    {

    vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

    if (!angleNode)
      {
      vtkErrorMacro("CreateWidget: Could not get angle node!")
      return 0;
      }


    vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::New();
    rep->InstantiateHandleRepresentation();


    vtkHandleWidget* h1 = this->m_HandleWidgetList[0];
    vtkHandleWidget* h2 = this->m_HandleWidgetList[1];
    vtkHandleWidget* h3 = this->m_HandleWidgetList[2];

    double * position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    rep->SetPoint1DisplayPosition(position1);

    double * position2 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();
    rep->SetPoint2DisplayPosition(position2);

    double * position3 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();
    rep->SetCenterDisplayPosition(position3);

    vtkAngleWidget *angleWidget = vtkAngleWidget::New();
    angleWidget->CreateDefaultRepresentation();
    angleWidget->SetRepresentation(rep);
    angleWidget->SetInteractor(this->GetInteractor());


    return angleWidget;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationAngleDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  vtkAbstractWidget* angleWidget = this->GetWidget(angleNode);
  if (!angleWidget) {
    vtkErrorMacro("Widget was not found!");
    return;
  }


}

void vtkMRMLAnnotationAngleDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLSelectionNode"));
  if ( selectionNode == NULL )
    {
    vtkErrorMacro ( "OnClickInThreeDRenderWindow: No selection node in the scene." );
    return;
    }

  if (!strcmp(selectionNode->GetActiveAnnotationID(), "vtkMRMLAnnotationAngleNode"))
    {

    if (this->m_ClickCounter->HasEnoughClicks(1))
      {

      //this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
      //this->GetRenderer()->NormalizedDisplayToViewport(x,y);

      this->PlaceSeed(x,y);
      vtkMRMLAnnotationAngleNode *angleNode = vtkMRMLAnnotationAngleNode::New();
      angleNode->Initialize(this->GetMRMLScene());


/*
      this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
      this->GetRenderer()->NormalizedDisplayToViewport(x,y);
      this->GetRenderer()->ViewportToNormalizedViewport(x,y);

      double coordinates[3];
      coordinates[0]=(double)x;
      coordinates[1]=(double)y;
      coordinates[2]=0;

      vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
      textNode->SetTextCoordinates(coordinates);
      textNode->SetTextLabel("New text");

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
*/
      } // has enough clicks
    else
      {
        this->PlaceSeed(x,y);
      }

    } // selection Node GetActiveAnnotationID


}
