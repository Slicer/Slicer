
// AnnotationModule/VTKWidgets includes
#include "vtkAnnotationRulerRepresentation.h"
#include "vtkAnnotationRulerRepresentation3D.h"
#include "vtkAnnotationRulerWidget.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationRulerWidget);
vtkCxxRevisionMacro (vtkAnnotationRulerWidget, "$Revision: 1.0 $");

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
vtkAnnotationRulerWidget::~vtkAnnotationRulerWidget()
{
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation()
{
  if (!this->Is2DWidget)
    {
    this->CreateDefaultRepresentation3D();
    return;
    }
  if (!this->WidgetRep)
    {
      this->WidgetRep = vtkAnnotationRulerRepresentation::New();
    }
  if (!vtkAnnotationRulerRepresentation::SafeDownCast(this->WidgetRep))
    {
    vtkErrorMacro("CreateDefaultRepresentation: unable to cast widget representation to a vtkAnnotationRulerRepresentation, class = " <<  this->WidgetRep->GetClassName());
    return;
    }
  vtkAnnotationRulerRepresentation::SafeDownCast(this->WidgetRep)->InstantiateHandleRepresentation();
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation3D()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation3D::New();
    }
  if (!vtkAnnotationRulerRepresentation3D::SafeDownCast(this->WidgetRep))
    {
    vtkErrorMacro("CreateDefaultRepresentation3D: unable to cast widget representation to a vtkAnnotationRulerRepresentation3D, class = " <<  this->WidgetRep->GetClassName());
    return;
    }
  vtkAnnotationRulerRepresentation3D::SafeDownCast(this->WidgetRep)->InstantiateHandleRepresentation();
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
