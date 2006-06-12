#ifndef __vtkSlicerIcons_h
#define __vtkSlicerIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

// Description:
// This class is defined in anticipation of superclass methods that
// all Slicer Icons will use...

class vtkSlicerIcons : public vtkKWObject
{
 public:
    static vtkSlicerIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerIcons, vtkKWObject);
    void PrintSelf ( ostream& os, vtkIndent indent );

 protected:
    vtkSlicerIcons ( );
    ~vtkSlicerIcons ( );
    
 private:
    vtkSlicerIcons (const vtkSlicerIcons& ); // Not implemented
    void operator = ( const vtkSlicerIcons& ); // Not implemented
    
};

#endif
