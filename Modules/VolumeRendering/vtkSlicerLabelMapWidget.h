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
    void UpdateGuiElements(void);
    static vtkSlicerLabelMapWidget *New();
    //vtkTypeRevisionMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      vtkTypeMacro(vtkSlicerLabelMapWidget,vtkKWCompositeWidget);
      //void UpdateGuiElements(void);
      void Init(vtkMRMLScalarVolumeNode *node,vtkMRMLVolumeRenderingNode *vrnode)
      {
          if(this->VolumeRenderingNode==vrnode&&this->Node==node)
          {
              vtkErrorMacro("Init already called, call UpdateGUIElements instead");
              return;
          }
          this->VolumeRenderingNode=vrnode;
          this->Node=node;
          if(this->Tree!=NULL)
          {
              this->Tree->Init(this->Node,this->VolumeRenderingNode);
          }
      }
      virtual void CreateWidget();

      void UpdateVolumeRenderingNode(vtkMRMLVolumeRenderingNode *vrNode)
      {
          if(this->Tree!=NULL)
          {
              this->Tree->UpdateVolumeRenderingNode(vrNode);
          }
          else
          {
              vtkErrorMacro("Init has to be used before Update is called");
          }
      }
          //TODO
      vtkGetObjectMacro(VolumeRenderingNode,vtkMRMLVolumeRenderingNode);
      vtkGetObjectMacro(Node,vtkMRMLScalarVolumeNode);

    //void PrintSelf(ostream& os, vtkIndent indent);

      //BTX
    enum
    {
        NeedForRenderEvent=30000,
    };
    //ETX
    protected:

    vtkMRMLVolumeRenderingNode *VolumeRenderingNode;
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
