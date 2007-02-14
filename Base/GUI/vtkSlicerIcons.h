#ifndef __vtkSlicerIcons_h
#define __vtkSlicerIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

#include "vtkSlicerBaseGUI.h"

// Description:
// This class is defined in anticipation of superclass methods that
// all Slicer Icons will use...

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerIcons : public vtkKWObject
{
 public:
    static vtkSlicerIcons* New ( );
    vtkTypeMacro ( vtkSlicerIcons, vtkKWObject);
    void PrintSelf ( ostream& os, vtkIndent indent );

 protected:
    vtkSlicerIcons ( );
    virtual ~vtkSlicerIcons ( );
    vtkSlicerIcons (const vtkSlicerIcons& ); // Not implemented
    void operator = ( const vtkSlicerIcons& ); // Not implemented
    
 private:
    
};

#endif
