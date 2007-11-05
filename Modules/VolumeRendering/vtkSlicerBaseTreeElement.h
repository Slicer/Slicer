#ifndef __vtkSlicerBaseTreeElement_h
#define __vtkSlicerBaseTreeElement_h
#include "vtkKWCompositeWidget.h"
#include "vtkVolumeRenderingModule.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerBaseTreeElement : public vtkKWCompositeWidget
{
public:
    static vtkSlicerBaseTreeElement *New();
    vtkTypeRevisionMacro(vtkSlicerBaseTreeElement,vtkKWCompositeWidget);
    void CreateWidget(void);

protected:
    vtkSlicerBaseTreeElement(void);
    ~vtkSlicerBaseTreeElement(void);
    vtkSlicerBaseTreeElement(const vtkSlicerBaseTreeElement&);//not implemented
    void operator=(const vtkSlicerBaseTreeElement&);//not implemented
};
#endif
