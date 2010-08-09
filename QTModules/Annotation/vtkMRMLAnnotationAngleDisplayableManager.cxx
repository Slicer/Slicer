// AnnotationModule includes
#include "vtkMRMLAnnotationAngleDisplayableManager.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationTextNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>


// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationAngleDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationAngleDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationAngleDisplayableManager Callback
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
  }
  vtkAnnotationTextWidgetCallback(){}
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

  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
    {
    vtkErrorMacro("CreateWidget: Could not get text node!")
    return 0;
    }

  vtkTextWidget* textWidget = vtkTextWidget::New();
  VTK_CREATE(vtkTextRepresentation, textRep);

  textRep->SetMoving(1);

  if (textNode->GetTextLabel())
    {
    textRep->SetText(textNode->GetTextLabel());
    }
  else
    {
    textRep->SetText("New text");
    }

  textWidget->SetRepresentation(textRep);

  textWidget->SetInteractor(this->GetInteractor());


  if (textNode->GetTextCoordinates())
    {

    textRep->SetPosition(textNode->GetTextCoordinates()[0],textNode->GetTextCoordinates()[1]);
    //textRep->SetPosition(0.01, 0.01);

    }

  textWidget->On();

  // add callback
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
  textWidget->AddObserver(vtkCommand::HoverEvent, myCallback);
  myCallback->Delete();

  InvokeEvent(vtkSlicerAnnotationModuleLogic::AddTextNodeCompletedEvent,0);

  return textWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationAngleDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  vtkAbstractWidget* textWidget = this->GetWidget(textNode);
  if (!textWidget) {
    vtkErrorMacro("Widget was not found!");
    return;
  }

  // Obtain associated representation
  vtkTextRepresentation* textRepr =
      vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  // Update Text
  textRepr->SetText(textNode->GetText(0));
}

void vtkMRMLAnnotationAngleDisplayableManager::OnClickInThreeDRenderWindow(float x, float y)
{

  if (this->m_ClickCounter->HasEnoughClicks(3))
    {

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

    }

  }
