#ifndef __vtkSlicerModuleNavigationIcons_h
#define __vtkSlicerModuleNavigationIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerModuleChoose_ImageData.h"

class vtkSlicerModuleNavigationIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerModuleNavigationIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerModuleNavigationIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro (ModuleNextIcon, vtkKWIcon);
    vtkGetObjectMacro (ModulePrevIcon, vtkKWIcon);
    vtkGetObjectMacro (ModuleHistoryIcon, vtkKWIcon);
    vtkGetObjectMacro (ModuleRefreshIcon, vtkKWIcon);
    vtkGetObjectMacro (ModuleSearchIcon, vtkKWIcon );

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerModuleNavigationIcons ( );
    virtual ~vtkSlicerModuleNavigationIcons ( );

    vtkKWIcon *ModulePrevIcon;
    vtkKWIcon *ModuleNextIcon;
    vtkKWIcon *ModuleHistoryIcon;
    vtkKWIcon *ModuleRefreshIcon;
    vtkKWIcon *ModuleSearchIcon;
    
 private:
    vtkSlicerModuleNavigationIcons (const vtkSlicerModuleNavigationIcons&); // Not implemented
    void operator = ( const vtkSlicerModuleNavigationIcons& ); // Not implemented
    
};

#endif
