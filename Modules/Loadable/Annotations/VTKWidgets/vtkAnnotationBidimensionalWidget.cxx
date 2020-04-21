
// Annotations includes
#include "vtkAnnotationBidimensionalWidget.h"
#include "vtkAnnotationBidimensionalRepresentation.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationBidimensionalWidget);

//---------------------------------------------------------------------------
void vtkAnnotationBidimensionalWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkAnnotationBidimensionalWidget::vtkAnnotationBidimensionalWidget() = default;

//---------------------------------------------------------------------------
vtkAnnotationBidimensionalWidget::~vtkAnnotationBidimensionalWidget() = default;

//---------------------------------------------------------------------------
void vtkAnnotationBidimensionalWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationBidimensionalRepresentation::New();
    }
  vtkAnnotationBidimensionalRepresentation::SafeDownCast(this->WidgetRep)
    ->InstantiateHandleRepresentation();
}

