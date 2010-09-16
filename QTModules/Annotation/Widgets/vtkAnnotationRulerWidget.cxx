#include "Widgets/vtkAnnotationRulerWidget.h"
#include "Widgets/vtkAnnotationRulerRepresentation.h"


// VTK includes
#include <vtkObject.h>
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

