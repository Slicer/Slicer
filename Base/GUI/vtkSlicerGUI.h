#ifndef __vtkSlicerGUI_h
#define __vtkSlicerGUI_h

#include "vtkKWObject.h"

class vtkSlicerStyle;

// Description:
// Contains Slicer's suggested presentation conventions,
// defined in the vtkSlicerStyle class.
//
class vtkSlicerGUI : public vtkKWObject
{
 public:
    static vtkSlicerGUI* New();
    vtkTypeRevisionMacro ( vtkSlicerGUI, vtkKWObject );
    
 protected:
    vtkSlicerGUI ( );
    ~vtkSlicerGUI ( );

    // Slicer's look and feel
    vtkSlicerStyle *SlicerStyle;

 private:
    vtkSlicerGUI ( const vtkSlicerGUI& );      // Not implemented.
    void operator = ( const vtkSlicerGUI& ); // Not implemented.
};

#endif
