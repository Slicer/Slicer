#ifndef __vtkSlicerLabelMapWidget_h
#define __vtkSlicerLabelMapWidget_h
#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerLabelmapTree.h"
class vtkKWTreeWithScrollbars;
class vtkMRMLScalarVolumeNode;
class vtkSlicerLabelmapElement;
class vtkLabelMapPiecewiseFunction;


class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelMapWidget :public vtkSlicerWidget
{
public:
    static vtkSlicerLabelMapWidget *New();
    //vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      vtkTypeMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      //void UpdateGuiElements(void);
      void Init(vtkMRMLScalarVolumeNode *node,vtkLabelMapPiecewiseFunction *piecewise)
      {
          if(this->PiecewiseFunction==piecewise&&this->Node==node)
          {
              return;
          }
          this->PiecewiseFunction=piecewise;
          this->Node=node;
          if(this->Tree!=NULL)
          {
              this->Tree->Init(this->Node,this->PiecewiseFunction);
          }
      }
      virtual void CreateWidget();

      vtkGetObjectMacro(PiecewiseFunction,vtkLabelMapPiecewiseFunction);
      vtkGetObjectMacro(Node,vtkMRMLScalarVolumeNode);

    //void PrintSelf(ostream& os, vtkIndent indent);

      //BTX
    enum
    {
        NeedForRenderEvent=30000,
    };
    //ETX
    protected:
    vtkLabelMapPiecewiseFunction *PiecewiseFunction;
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
