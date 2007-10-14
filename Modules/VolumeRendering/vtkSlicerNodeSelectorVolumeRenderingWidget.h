#ifndef __vtkSlicerNodeSelectorVolumeRenderingWidget_h
#define __vtkSlicerNodeSelectorVolumeRenderingWidget_h

#include "vtkSlicerModuleGUI.h"
#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerNodeSelectorWidget.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerNodeSelectorVolumeRenderingWidget :public vtkSlicerNodeSelectorWidget
{
public:
  static vtkSlicerNodeSelectorVolumeRenderingWidget* New();
  //vtkTypeMacro(vtkSlicerNodeSelectorVolumeRenderingWidget,vtkSlicerNodeSelectorWidget);
  void PrintSelf(ostream& os, vtkIndent indent){}
    //BTX
    void SetCondition(std::string con,bool modeCon)
    {
        this->Condition=con;
        this->ModeCondition=modeCon;
    }
    std::string GetCondition(void)
    {
        return this->Condition;
    }
    void SetAdditionalMRMLScene(vtkMRMLScene *scene)
    {
        this->AdditionalMRMLScene=scene;
    }
    //ETX
protected:
    //BTX
    std::string Condition;
    //ETX
    bool ModeCondition;//if 1 use ==for condition if 0 use != for condition
    vtkMRMLScene *AdditionalMRMLScene;
    vtkSlicerNodeSelectorVolumeRenderingWidget();
    ~vtkSlicerNodeSelectorVolumeRenderingWidget();
    vtkSlicerNodeSelectorVolumeRenderingWidget(const vtkSlicerNodeSelectorVolumeRenderingWidget&);//not implemented
    void operator=(const vtkSlicerNodeSelectorVolumeRenderingWidget&);//not implemented
    
    virtual  bool CheckAdditionalConditions(vtkMRMLNode *node);
    virtual int AddAditionalNodes();
    virtual vtkMRMLNode* GetSelectedInAdditional();
};
#endif
