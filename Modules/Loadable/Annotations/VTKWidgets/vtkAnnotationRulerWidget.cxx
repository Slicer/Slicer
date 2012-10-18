
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
  if ( ! this->WidgetRep)
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation::New();
    }
  if (!vtkDistanceRepresentation::SafeDownCast(this->WidgetRep))
    {
    vtkErrorMacro("CreateDefaultRepresentation: unable to cast widget representation to a vtkDistanceRepresentation, class = " <<  this->WidgetRep->GetClassName());
    return;
    }
  vtkDistanceRepresentation::SafeDownCast(this->WidgetRep)->InstantiateHandleRepresentation();
}

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation3D()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation3D::New();
    }
  if (!vtkDistanceRepresentation::SafeDownCast(this->WidgetRep))
    {
    vtkErrorMacro("CreateDefaultRepresentation3D: unable to cast widget representation to a vtkDistanceRepresentation, class = " <<  this->WidgetRep->GetClassName());
    return;
    }
  
  vtkDistanceRepresentation::SafeDownCast(this->WidgetRep)->InstantiateHandleRepresentation();
}
