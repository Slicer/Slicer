#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerWidget.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkSlicerWidget
{
public:
    static vtkSlicerLabelMapWidget *New();
    vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkSlicerWidget);
    //void PrintSelf(ostream& os, vtkIndent indent);

    protected:
    vtkSlicerLabelMapWidget(void);
    ~vtkSlicerLabelMapWidget(void);
    vtkSlicerLabelMapWidget(const vtkSlicerLabelMapWidget&);
    void operator=(const vtkSlicerLabelMapWidget&);
    virtual void CreateWidget(void);
};
#endif
