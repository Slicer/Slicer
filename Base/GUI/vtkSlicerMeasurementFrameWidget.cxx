#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMeasurementFrameWidget.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMeasurementFrameWidget);
vtkCxxRevisionMacro (vtkSlicerMeasurementFrameWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------
vtkSlicerMeasurementFrameWidget::vtkSlicerMeasurementFrameWidget(void)
  {
  
  }

//---------------------------------------------------------------------------
vtkSlicerMeasurementFrameWidget::~vtkSlicerMeasurementFrameWidget(void)
  {
 
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::AddWidgetObservers ( )
  {    
  
  }
//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::RemoveWidgetObservers( )
  {
  
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerMeasurementFrameWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::ProcessMRMLEvents (vtkObject *caller,unsigned long event, void *callData)
  {
  //if (this->Matrix == vtkMatrix4x4::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent)
  //  {
  //  //vtkErrorMacro(" modified" << this->GetMRMLScene()->GetClassName());
  //  //this->GetMRMLScene()->Undo();
  //  }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {


  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::CreateWidget( )
  {
  
  } 



