#ifndef __vtkSlicerTheme_h
#define __vtkSlicerTheme_h

#include "vtkKWTheme.h"
#include "vtkSlicerColor.h"

class vtkSlicerTheme : public vtkKWTheme
{
 public:
    // Description:
    // Usual vtk functions
    static vtkSlicerTheme* New ();
    vtkTypeRevisionMacro ( vtkSlicerTheme, vtkKWTheme );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get method for SlicerColor class
    vtkGetObjectMacro ( SlicerColors, vtkSlicerColor );

    // Description:
    // Ask the default Slicer theme to install itself
    virtual void Install ( );
    
 protected:
    vtkSlicerTheme ( );
    virtual ~vtkSlicerTheme ( );
    vtkSlicerColor *SlicerColors;
    
 private:
    vtkSlicerTheme (const vtkSlicerTheme& ); // Not implemented
    void operator = (const vtkSlicerTheme& ); // Not implemented
    
};
#endif
