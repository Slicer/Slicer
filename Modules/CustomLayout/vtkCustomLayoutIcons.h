#ifndef __vtkCustomLayoutIcons_h
#define __vtkCustomLayoutIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

#include "vtkSlicerIcons.h"

#include "vtkCustomLayoutWin32Header.h"
#include "vtkCustomLayout.h"
#include "./Resources/vtkCustomLayout_ImageData.h"

class VTK_CUSTOMLAYOUT_EXPORT vtkCustomLayoutIcons : public vtkSlicerIcons
{
 public:
    static vtkCustomLayoutIcons* New ( );
    vtkTypeRevisionMacro ( vtkCustomLayoutIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( ContentAssignedIcon, vtkKWIcon );
    vtkGetObjectMacro ( ContentUnassignedIcon, vtkKWIcon );
    
    void AssignImageDataToIcons ( );
    
 protected:
    vtkCustomLayoutIcons ( );
    ~vtkCustomLayoutIcons ( );
    vtkKWIcon *ContentAssignedIcon;
    vtkKWIcon *ContentUnassignedIcon;

    
 private:
    vtkCustomLayoutIcons (const vtkCustomLayoutIcons&); // Not implemented
    void operator = ( const vtkCustomLayoutIcons& ); // Not implemented
    
};

#endif
