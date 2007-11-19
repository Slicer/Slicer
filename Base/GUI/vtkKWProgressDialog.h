// .NAME vtkKWProgressDialog - show a dialog with progress on top of the parent window
// .SECTION Description
// Creates a dialog that will be shown on top of the parent window.
// Attention: The display method processes all pending events to allow a proper display on all platforms.
// Keep also in main that the progress gauge executes all idle events.
// Example how to use it: Slicer3\Modules\VolumeRendering\vtkSlicerVRLabelmapHelper.cxx

#ifndef __vtkKWProgressDialog_h
#define __vtkKWProgressDialog_h


#include "vtkKWTopLevel.h"

#include <string>

#include "vtkSlicerBaseGUI.h"

class vtkKWLabel;
class vtkKWProgressGauge;

class VTK_SLICER_BASE_GUI_EXPORT vtkKWProgressDialog : public vtkKWTopLevel
{
public:
    // Description:
    // Usual vtk methods: go to www.vtk.org for more details
    static vtkKWProgressDialog *New();
    vtkTypeRevisionMacro(vtkKWProgressDialog, vtkKWTopLevel);   
    void PrintSelf(ostream& os, vtkIndent indent);

    //Description:
    //Update the progress of the ProgressDialog. Valid values are between 0 and 1.
    void UpdateProgress(float progress);

    //Description:
    //Set the message text which will be shown.
    void SetMessageText(const char* messageText);

    //Description:
    //Display the widget
    //Attention: All pending events will be executed for proper Display
    virtual void Display();

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkKWProgressDialog(void);
    // Description:
    // Use ->Delete() to delete object
    ~vtkKWProgressDialog(void);

    // Description:
    // Create the widget.
    virtual void CreateWidget();


    //BTX
    std::string MessageText;
    //ETX
    
    //Description:
    //Message that is shown.
    vtkKWLabel *Message;

    //Description:
    //ProgressGauge that is shown.
    vtkKWProgressGauge *Progress;

private:
    // Description:
    // Caution: Not implemented
    vtkKWProgressDialog(const vtkKWProgressDialog&); // Not implemented
    void operator=(const vtkKWProgressDialog&); // Not implemented
};
#endif
