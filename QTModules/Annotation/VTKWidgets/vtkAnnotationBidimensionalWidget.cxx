
// AnnotationModule/VTKWidgets includes
#include "VTKWidgets/vtkAnnotationBidimensionalWidget.h"
#include "VTKWidgets/vtkAnnotationBidimensionalRepresentation.h"

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkObjectFactory.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationBidimensionalWidget);
vtkCxxRevisionMacro (vtkAnnotationBidimensionalWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkAnnotationBidimensionalWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkAnnotationBidimensionalWidget::vtkAnnotationBidimensionalWidget()
{


}

vtkAnnotationBidimensionalWidget::~vtkAnnotationBidimensionalWidget()
{

}

//----------------------------------------------------------------------
void vtkAnnotationBidimensionalWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationBidimensionalRepresentation::New();
    }
  reinterpret_cast<vtkAnnotationBidimensionalRepresentation*>(this->WidgetRep)->InstantiateHandleRepresentation();
}

