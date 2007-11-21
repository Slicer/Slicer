// .NAME vtkSlicerLabelmapElement - Extends vtkSlicerBaseTreeElement with specific label map functionality
// .SECTION Description
// vtkSlicerLabelmapElement extends vtkSlicerBaseTreeElement with specific label map functionality.
// An element consists of the following columns: The color, the opacity (selected and shown by PushButtons) and the
// name of the color
#ifndef __vtkSlicerLabelmapElement_h
#define __vtkSlicerLabelmapElement_h

#include "vtkSlicerBaseTreeElement.h"
#include "vtkVolumeRenderingModule.h"

#include <string>

class vtkKWLabel;
class vtkKWPushButton;


class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerLabelmapElement : public vtkSlicerBaseTreeElement
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerLabelmapElement *New();
    vtkTypeRevisionMacro(vtkSlicerLabelmapElement,vtkSlicerBaseTreeElement);
    virtual void CreateWidget(void);

    //BTX
    //Number of levels for the opacity
    static const int NUMBER_LEVELS=6;
    //ETX


    //BTX
    // Description:
    // Initialize the Labelmap Element. Use this method directly after creating the widget and 
    // before using any other method. 
    // id: Number of the color in the vtkLookupTable,
    // colorName: Name of the color
    // color: rgb values of the color
    // opacityLevel: which level is used for the opacity
    // max: max length of a color name
    void Init(int id,std::string colorName,double color[3] ,int opacityLevel,int max);
    //ETX

    // Description:
    // Change the opacity of this element to the specific stage
    // Invokes vtkCommand::AnyEvent with callData: id, stage as a int[2]
    void ChangeOpacity(int stage);

    // Description:
    // Get the id of the color shown by the element.
    vtkGetMacro(Id,int);

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerLabelmapElement(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerLabelmapElement(void);

    // Description:
    // Number of the color in the vtkLookupTable
    int Id;

    // Description:
    // Name of the color in the ColorNode
    vtkKWLabel *ColorName;

    // Description:
    // Label that is used to show the color. That means the label is a string 
    // consisting of several whitespaces and the background represents the color.
    vtkKWLabel *Color;

    // Description:
    // PushButtons that are used to show and change the opacity of the color.
    vtkKWPushButton *Opacity[vtkSlicerLabelmapElement::NUMBER_LEVELS];

    // Description:
    //CallbackCommand for 
    vtkCallbackCommand* LabelmapCallbackCommand;

    // Description:
    // Flags to avoid event loops.
    int InLabelmapCallbackFlag;


    // Description:
    // Set flag to state that an callback is executed
    void SetInLabelmapCallbackFlag (int flag) 
    {
        this->InLabelmapCallbackFlag = flag;
    }

    // Description:
    // Get if a labelmap callback is executed right now.
    vtkGetMacro(InLabelmapCallbackFlag, int);

    // Description:
    // The labelmap callback. Used to avoid event loops.
    static void LabelmapCallback( vtkObject *__caller,unsigned long eid, void *__clientData, void *callData );

    // Description:
    // Process
    virtual void ProcessLabelmapEvents(vtkObject *caller,unsigned long eid,void *callData);

    // Description:
    // Fill the BUttons to the specified stage without invoking an command.
    void FillButtons(int stage);

private:

    // Description:
    // Caution: Not implemented
    vtkSlicerLabelmapElement(const vtkSlicerLabelmapElement&);//not implemented
    void operator=(const vtkSlicerLabelmapElement&);//not implemented
};
#endif
