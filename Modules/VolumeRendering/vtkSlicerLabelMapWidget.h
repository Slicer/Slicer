#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkKWCompositeWidget.h"
class vtkKWTreeWithScrollbars;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkKWCompositeWidget
{
public:
    static vtkSlicerLabelMapWidget *New();
    //vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      vtkTypeMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
    //void PrintSelf(ostream& os, vtkIndent indent);

    protected:
    vtkKWTreeWithScrollbars *widgets_tree;
    vtkSlicerLabelMapWidget(void);
    ~vtkSlicerLabelMapWidget(void);
    vtkSlicerLabelMapWidget(const vtkSlicerLabelMapWidget&);
    void operator=(const vtkSlicerLabelMapWidget&);
    virtual void CreateWidget(void);
};
#endif
