#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerLabelmapTree.h"
class vtkKWTreeWithScrollbars;
class vtkMRMLScalarVolumeNode;
class vtkSlicerLabelmapElement;


class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkSlicerWidget
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
      virtual void CreateWidget();
    //void PrintSelf(ostream& os, vtkIndent indent);

    protected:
    vtkMRMLScalarVolumeNode *Node;
    vtkSlicerLabelmapTree *Tree;
    vtkSlicerLabelmapElement *ChangeAll;
    vtkSlicerLabelMapWidget(void);
    ~vtkSlicerLabelMapWidget(void);
    vtkSlicerLabelMapWidget(const vtkSlicerLabelMapWidget&);
    void operator=(const vtkSlicerLabelMapWidget&);
    virtual void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);
};
#endif
