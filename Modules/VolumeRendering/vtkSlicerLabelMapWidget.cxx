#include "vtkSlicerLabelMapWidget.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"

vtkSlicerLabelMapWidget* vtkSlicerLabelMapWidget::New(void)
{
     // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerLabelMapWidget");
  if(ret)
    {
      return (vtkSlicerLabelMapWidget*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerLabelMapWidget;
}
vtkSlicerLabelMapWidget::vtkSlicerLabelMapWidget(void)
{
}

vtkSlicerLabelMapWidget::~vtkSlicerLabelMapWidget(void)
{
}
