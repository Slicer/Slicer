#ifndef __vtkSlicerTheme_h
#define __vtkSlicerTheme_h

#include "vtkKWTheme.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerCheckRadioButtonIcons.h"

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
    vtkGetMacro ( FontPointSize, int);

    // Description:
    // Ask the default Slicer theme to install itself
    virtual void Install ( );

    // Description:
    // Convenience method that sets font for slicer widgets
    // and special-case fonts throughout the Application
    virtual void AddSlicerFontOptions (const char *font);
    
 protected:
    vtkSlicerTheme ( );
    virtual ~vtkSlicerTheme ( );
    vtkSlicerColor *SlicerColors;

    int FontPointSize;
    vtkSlicerCheckRadioButtonIcons *CheckRadioIcons;
    
 private:
    vtkSlicerTheme (const vtkSlicerTheme& ); // Not implemented
    void operator = (const vtkSlicerTheme& ); // Not implemented
    
};
#endif
