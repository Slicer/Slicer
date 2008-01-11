#ifndef __vtkSlicerVisibilityIcons_h
#define __vtkSlicerVisibilityIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerVisibility_ImageData.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVisibilityIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerVisibilityIcons* New ( );
    vtkTypeRevisionMacro (vtkSlicerVisibilityIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( VisibleIcon, vtkKWIcon );
    vtkGetObjectMacro ( InvisibleIcon, vtkKWIcon );

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerVisibilityIcons ( );
    virtual ~vtkSlicerVisibilityIcons ( );
    vtkKWIcon *VisibleIcon;
    vtkKWIcon *InvisibleIcon;
    
 private:
    vtkSlicerVisibilityIcons ( const vtkSlicerVisibilityIcons&); // Not implemented
    void operator = (const vtkSlicerVisibilityIcons& ); // not implemented.
    
};
#endif
