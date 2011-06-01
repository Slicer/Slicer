
// AnnotationModule/VTKWidgets includes
#include "VTKWidgets/vtkAnnotationRulerRepresentation.h"
#include "VTKWidgets/vtkAnnotationRulerRepresentation3D.h"
#include "VTKWidgets/vtkAnnotationRulerWidget.h"

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkObjectFactory.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationRulerWidget);
vtkCxxRevisionMacro (vtkAnnotationRulerWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkAnnotationRulerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkAnnotationRulerWidget::vtkAnnotationRulerWidget()
{


}

vtkAnnotationRulerWidget::~vtkAnnotationRulerWidget()
{

}

//----------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation::New();
    }
  reinterpret_cast<vtkAnnotationRulerRepresentation*>(this->WidgetRep)->InstantiateHandleRepresentation();
}

//----------------------------------------------------------------------
void vtkAnnotationRulerWidget::CreateDefaultRepresentation3D()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationRulerRepresentation3D::New();
    }
  reinterpret_cast<vtkAnnotationRulerRepresentation3D*>(this->WidgetRep)->InstantiateHandleRepresentation();
}
