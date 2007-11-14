// .NAME vtkSlicerBaseTreeElement - Extends vtkKWCompositeWidget for Slicer specific issues, especially 
// to us it together with vtkSlicerBaseTree
// .SECTION Description
// vtkSlicerBaseTreeElements is intended to use it together with vtkSlicerBaseTree. Use the SetNodeWindow in
// vtkSlicerBaseTree to set a tree element into a node.

#ifndef __vtkSlicerBaseTreeElement_h
#define __vtkSlicerBaseTreeElement_h
#include "vtkKWCompositeWidget.h"
#include "vtkVolumeRenderingModule.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerBaseTreeElement : public vtkKWCompositeWidget
{
public:

    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerBaseTreeElement *New();
    vtkTypeRevisionMacro(vtkSlicerBaseTreeElement,vtkKWCompositeWidget);
    void CreateWidget(void);

protected:

    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerBaseTreeElement(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerBaseTreeElement(void);
private:
    // Description:
    // Caution: Not implemented
    vtkSlicerBaseTreeElement(const vtkSlicerBaseTreeElement&);//not implemented
    void operator=(const vtkSlicerBaseTreeElement&);//not implemented
};
#endif
