#ifndef __vtkSlicerLabelmapElement_h
#define __vtkSlicerLabelmapElement_h
#include "vtkSlicerBaseTreeElement.h"
#include "vtkVolumeRenderingModule.h"
#include "vtkKWPushButton.h"
#include <string>
class vtkKWLabel;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapElement : public vtkSlicerBaseTreeElement
{
public:
    static vtkSlicerLabelmapElement *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapElement,vtkSlicerBaseTreeElement);
    virtual void CreateWidget(void);
    //BTX
    void Init(std::string colorName,double color[3] ,int opacityLevel,int max);
    //ETX
    void ChangeOpacity(int stage);
protected:
    vtkKWLabel *ColorName;
    vtkKWLabel *Color;
    vtkKWPushButton *Opacity[6];
    vtkSlicerLabelmapElement(void);
    ~vtkSlicerLabelmapElement(void);
    vtkSlicerLabelmapElement(const vtkSlicerLabelmapElement&);//not implemented
    void operator=(const vtkSlicerLabelmapElement&);//not implemented
    void SetInLabelmapCallbackFlag (int flag) 
    {
        this->InLabelmapCallbackFlag = flag;
    }
    vtkGetMacro(InLabelmapCallbackFlag, int);
    vtkCallbackCommand* LabelmapCallbackCommand;
    int InLabelmapCallbackFlag;
    static void LabelmapCallback( vtkObject *__caller,unsigned long eid, void *__clientData, void *callData );
    virtual void ProcessLabelmapEvents(vtkObject *caller,unsigned long eid,void *callData);
        void FillButtons(int stage);

};
#endif
