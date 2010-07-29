// AnnotationModule includes
#include "vtkMRMLAnnotationTextDisplayableManager.h"

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
vtkStandardNewMacro (vtkMRMLAnnotationTextDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationTextDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextDisplayableManager Callback
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
// vtkMRMLAnnotationTextDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationTextDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
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

  std::cout << "....11...." << std::endl;
  vtkTextWidget* textWidget = vtkTextWidget::New();
  VTK_CREATE(vtkTextRepresentation, textRep);
  std::cout << "....22...." << std::endl;
  textRep->SetMoving(1);
  std::cout << "....221...." << std::endl;
  if (!textNode->GetText(0))
    {
    vtkErrorMacro("CreateWidget: No text was set for the node!")
    return 0;
    }
  textRep->SetText(textNode->GetText(0));
  std::cout << "....222...." << std::endl;
  textWidget->SetRepresentation(textRep);
  std::cout << "....223...." << std::endl;
  textWidget->SetInteractor(this->GetInteractor());

  std::cout << "....224...." << std::endl;
  if (!textNode->GetTextCoordinates())
    {
    vtkErrorMacro("CreateWidget: Could not get coordinates for widget!")
    return 0;
    }

  textRep->SetPosition(textNode->GetTextCoordinates());
  textWidget->On();
  std::cout << ".....33..." << std::endl;
  // add callback
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
  textWidget->AddObserver(vtkCommand::HoverEvent, myCallback);
  myCallback->Delete();
  std::cout << ".....44..." << std::endl;
  return textWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationTextDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
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

void vtkMRMLAnnotationTextDisplayableManager::OnClickInThreeDRenderWindow(int x, int y)
{
  double coordinates[3];
  coordinates[0]=(double)x;
  coordinates[1]=(double)y;
  coordinates[2]=0;

  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
  textNode->Initialize(this->GetMRMLScene());

  textNode->SetTextCoordinates(coordinates);

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
