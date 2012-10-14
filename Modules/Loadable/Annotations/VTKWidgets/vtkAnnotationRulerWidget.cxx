
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
}

//---------------------------------------------------------------------------
vtkAnnotationRulerWidget::~vtkAnnotationRulerWidget()
{
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation::New();
    }
  vtkAnnotationRulerRepresentation::SafeDownCast(this->WidgetRep)
    ->InstantiateHandleRepresentation();
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation3D()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation3D::New();
    }
  vtkAnnotationRulerRepresentation3D::SafeDownCast(this->WidgetRep)
    ->InstantiateHandleRepresentation();
}
