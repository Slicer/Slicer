#include "vtkSlicerVolumePropertyWidget.h"

vtkSlicerVolumePropertyWidget::vtkSlicerVolumePropertyWidget(void)
{
}

vtkSlicerVolumePropertyWidget::~vtkSlicerVolumePropertyWidget(void)
{
}
vtkSlicerVolumePropertyWidget* vtkSlicerVolumePropertyWidget::New()
{
 // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerVolumePropertyWidget");
  if(ret)
    {
      return (vtkSlicerVolumePropertyWidget*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerVolumePropertyWidget;
}
