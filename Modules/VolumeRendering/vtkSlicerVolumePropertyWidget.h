#ifndef __vtkSlicerVolumePropertyWidget_h
#define __vtkSlicerVolumePropertyWidget_h

#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerWidget.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVolumePropertyWidget :public vtkSlicerWidget
{
public:
     static vtkSlicerVolumePropertyWidget *New();
    //void PrintSelf(ostream& os, vtkIndent indent);
   
public:
     vtkSlicerVolumePropertyWidget(void);
    ~vtkSlicerVolumePropertyWidget(void);
    vtkSlicerVolumePropertyWidget(const vtkSlicerVolumePropertyWidget&);
    void operator=(const vtkSlicerVolumePropertyWidget&);
};
#endif
