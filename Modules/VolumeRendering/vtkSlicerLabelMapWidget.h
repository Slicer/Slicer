#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkKWCompositeWidget.h"
#include "vtkSlicerLabelmapTree.h"
class vtkKWTreeWithScrollbars;
class vtkMRMLScalarVolumeNode;


class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkKWCompositeWidget
{
public:
    static vtkSlicerLabelMapWidget *New();
    //vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      vtkTypeMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      void Init(vtkMRMLScalarVolumeNode *node)
      {
          this->Node=node;
          if(this->Tree!=NULL)
          {
              this->Tree->Init(this->Node);
          }
      }
    //void PrintSelf(ostream& os, vtkIndent indent);

    protected:
    vtkMRMLScalarVolumeNode *Node;
    vtkSlicerLabelmapTree *Tree;
    vtkSlicerLabelMapWidget(void);
    ~vtkSlicerLabelMapWidget(void);
    vtkSlicerLabelMapWidget(const vtkSlicerLabelMapWidget&);
    void operator=(const vtkSlicerLabelMapWidget&);
    virtual void CreateWidget(void);
};
#endif
