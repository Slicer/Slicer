// .NAME vtkSlicerApplication
// .SECTION Description
// Contains slicer's style, application and collection of associated guis.
//


#ifndef __vtkSlicerApplication_h
#define __vtkSlicerApplication_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWApplication.h"
#include "vtkSlicerStyle.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerGUICollection.h"


class vtkSlicerModuleGUI;

// Description:
// Contains slicer's style, application and collection of associated guis.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplication : public vtkKWApplication
{
 public:
    static vtkSlicerApplication* New ( );
    vtkTypeRevisionMacro ( vtkSlicerApplication, vtkKWApplication );

    vtkGetObjectMacro ( SlicerStyle, vtkSlicerStyle );
    vtkSetObjectMacro ( SlicerStyle, vtkSlicerStyle );
    vtkGetObjectMacro ( MainLayout, vtkSlicerGUILayout );
    
    vtkGetObjectMacro ( ModuleGUICollection, vtkSlicerGUICollection );

    // Description:
    // This method collects GUIs added to Slicer.
    virtual void AddModuleGUI ( vtkSlicerModuleGUI *gui );
    virtual vtkSlicerModuleGUI* GetModuleGUIByName ( char *name );

    // Description:
    // These methods manage windows associated with the application
    virtual void CloseAllWindows ( ) ;
    
    // Description:
    // Sets application behavior.
    virtual void ConfigureApplication ( );
    virtual int StartApplication ( );
    
 protected:
    vtkSlicerApplication ( );
    ~vtkSlicerApplication ( );

    vtkSlicerStyle *SlicerStyle;
    vtkSlicerGUILayout *MainLayout;

    // Description:
    // Collections of GUIs
    vtkSlicerGUICollection *ModuleGUICollection;
    
 private:
    vtkSlicerApplication ( const vtkSlicerApplication& ); // Not implemented.
    void operator = ( const vtkSlicerApplication& ); //Not implemented.
}; 

#endif
