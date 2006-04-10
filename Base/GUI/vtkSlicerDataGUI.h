#ifndef __vtkSlicerDataGUI_h
#define __vtkSlicerDataGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
//#include "vtkSlicerDataLogic.h"

// Description:
// This class implements Slicer's Data GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerDataGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerDataGUI, vtkSlicerModuleGUI );

    //vtkGetObjectMacro ( DataLogic, vtkSlicerDataLogic);
    //vtkSetObjectMacro ( DataLogic, vtkSlicerDataLogic);

    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( ) ;
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerDataGUI ( );
    ~vtkSlicerDataGUI ( );

    // Logic pointer for the Data module

    //vtkSlicerDataLogic *DataLogic;
    // Widgets for the Data module

 private:
    vtkSlicerDataGUI ( const vtkSlicerDataGUI& ); // Not implemented.
    void operator = ( const vtkSlicerDataGUI& ); //Not implemented.
};


#endif
