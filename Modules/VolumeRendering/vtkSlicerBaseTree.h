// .NAME vtkSlicerBaseTree - Extends vtkKWTreeWithScrollbars for Slicer specific issues
// .SECTION Description
// vtkSlicerBaseTree extends vtkKWTreeWithScrollbars vor Slicer specific issues. This especially means
// the use of own Callbacks so far.
#ifndef __vtkSlicerBaseTree_h
#define __vtkSlicerBaseTree_h
#include "vtkKWTreeWithScrollbars.h"
#include "vtkVolumeRenderingModule.h"
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerBaseTree : public vtkKWTreeWithScrollbars
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerBaseTree *New();
    vtkTypeRevisionMacro(vtkSlicerBaseTree,vtkKWTreeWithScrollbars);
    virtual void CreateWidget(void);

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerBaseTree(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerBaseTree(void);

    // Description:
    // Set/Get flag while executing callback
    // Creates possibility to detect recursive callbacks
    void SetInBaseTreeCallbackFlag (int flag) 
    {
        this->InBaseTreeCallbackFlag = flag;
    }
    vtkGetMacro(InBaseTreeCallbackFlag, int);
    // Description:
    // static method for BasTreeEvents
    static void BaseTreeCallback( vtkObject *__caller,unsigned long eid, void *__clientData, void *callData );
    // Description:
    // Will be executed everytime a BaseTreeEvent occurs
    virtual void ProcessBaseTreeEvents(vtkObject *caller,unsigned long eid,void *callData);

    // Description:
    // Callback command for Base Tree Events
    vtkCallbackCommand* BaseTreeCallbackCommand;
    
    //Description:
    //Flag is activated during 
    int InBaseTreeCallbackFlag;

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerBaseTree(const vtkSlicerBaseTree&);//not implemented
    void operator=(const vtkSlicerBaseTree&);//not implemented
};
#endif
