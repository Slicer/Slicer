// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationStickyDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationStickyNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkPNGReader.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkLogoWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkLogoRepresentation.h>


// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationStickyDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationStickyDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationStickyDisplayableManager Callback
class vtkAnnotationStickyWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationStickyWidgetCallback *New()
  { return new vtkAnnotationStickyWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
  }
  vtkAnnotationStickyWidgetCallback(){}
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationStickyDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationStickyDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationStickyDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
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

  vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

  if (!stickyNode)
    {
    vtkErrorMacro("CreateWidget: Could not get sticky node!")
    return 0;
    }

  VTK_CREATE(vtkHandleWidget,h1);
  h1 = this->m_HandleWidgetList[0];

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double x = position1[0];
  double y = position1[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedViewport(x,y);

  VTK_CREATE(vtkPNGReader,r);

  // a hack to get the icon
  std::stringstream s;
  s << std::getenv("TMPDIR") << "sticky.png";
  r->SetFileName(s.str().c_str());

  VTK_CREATE(vtkLogoRepresentation,logoRepresentation);
  logoRepresentation->SetImage(r->GetOutput());
  logoRepresentation->SetPosition(x,y);
  logoRepresentation->SetPosition2(.1, .1);
  logoRepresentation->GetImageProperty()->SetOpacity(.7);

  vtkLogoWidget * logoWidget = vtkLogoWidget::New();
  logoWidget->SetInteractor(this->GetInteractor());
  logoWidget->SetRepresentation(logoRepresentation);

  return logoWidget;

  }


//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationStickyDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
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
void vtkMRMLAnnotationStickyDisplayableManager::OnWidgetCreated()
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
void vtkMRMLAnnotationStickyDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    vtkMRMLAnnotationStickyNode *stickyNode = vtkMRMLAnnotationStickyNode::New();

    stickyNode->Initialize(this->GetMRMLScene());


    stickyNode->Delete();

    }

  }
