
// AnnotationModule/VTKWidgets includes
#include "vtkAnnotationRulerRepresentation.h"
#include "vtkAnnotationRulerRepresentation3D.h"
#include "vtkAnnotationRulerWidget.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationRulerWidget);

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkAnnotationRulerWidget::vtkAnnotationRulerWidget()
{
  this->Is2DWidget = true;
}

//---------------------------------------------------------------------------
vtkAnnotationRulerWidget::~vtkAnnotationRulerWidget() = default;

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
    {
    if (this->Is2DWidget)
      {
      this->WidgetRep = vtkAnnotationRulerRepresentation::New();
      }
    else
      {
      this->WidgetRep = vtkAnnotationRulerRepresentation3D::New();
      }
    }
  if (!vtkDistanceRepresentation::SafeDownCast(this->WidgetRep))
    {
    vtkErrorMacro("CreateDefaultRepresentation: unable to cast widget representation to a vtkDistanceRepresentation, class = " <<  this->WidgetRep->GetClassName());
    return;
    }
  vtkDistanceRepresentation::SafeDownCast(this->WidgetRep)->InstantiateHandleRepresentation();
}

//---------------------------------------------------------------------------
bool vtkAnnotationRulerWidget::GetIs2DWidget()
{
  return this->Is2DWidget;
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::SetIs2DWidget(int value)
{
  if (this->WidgetRep)
    {
    vtkErrorMacro( << "SetIs2DWidget: Setting Is2DWidget after a representation has been built is a no-op !");
    return;
    }
  this->Is2DWidget = value;
}
