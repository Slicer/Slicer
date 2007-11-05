#ifndef __vtkSlicerBaseTree_h
#define __vtkSlicerBaseTree_h
#include "vtkKWTreeWithScrollbars.h"
#include "vtkVolumeRenderingModule.h"
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerBaseTree : public vtkKWTreeWithScrollbars
{
public:
    static vtkSlicerBaseTree *New();
    vtkTypeRevisionMacro(vtkSlicerBaseTree,vtkKWTreeWithScrollbars);
        virtual void CreateWidget(void);
protected:
        vtkSlicerBaseTree(void);
    ~vtkSlicerBaseTree(void);
      vtkSlicerBaseTree(const vtkSlicerBaseTree&);//not implemented
    void operator=(const vtkSlicerBaseTree&);//not implemented
};
#endif
