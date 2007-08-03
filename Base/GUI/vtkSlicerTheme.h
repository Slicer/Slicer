#ifndef __vtkSlicerTheme_h
#define __vtkSlicerTheme_h

#include <map>
#include <vector>
#include <string>
#include <vtksys/SystemTools.hxx>
#include <vtksys/hash_map.hxx>

#include "vtkKWTheme.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerFont.h"
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
    vtkGetObjectMacro ( SlicerFonts, vtkSlicerFont );
    vtkGetMacro (FontSize0, int );
    vtkGetMacro (FontSize1, int );    
    vtkGetMacro (FontSize2, int );
    
    virtual const char *GetFontFamily ( )
        {
        return ( this->FontFamily );
        }
    virtual void SetFontFamily ( const char *font )
        {
        this->FontFamily = font;
        }
    virtual void SetApplicationFont0 ( const char *str )
        {
        this->ApplicationFont0 = str;
        }
    virtual void SetApplicationFont1 ( const char *str )
        {
        this->ApplicationFont1 = str;
        }
    virtual void SetApplicationFont2 ( const char *str )
        {
        this->ApplicationFont2 = str;
        }
    const char *GetApplicationFont0 ( )
        {
        return this->ApplicationFont0;
        }
    const char *GetApplicationFont1 ( )
        {
        return this->ApplicationFont1;
        }
    const char *GetApplicationFont2 ( )
        {
        return this->ApplicationFont2;
        }

    // Description:
    // Ask the default Slicer theme to install itself
    virtual void Install ( );
    virtual void InstallFonts ( );
    
 protected:
    vtkSlicerTheme ( );
    virtual ~vtkSlicerTheme ( );
    vtkSlicerColor *SlicerColors;
    vtkSlicerFont *SlicerFonts;
    
    const char *FontFamily;
    int FontSize0;
    int FontSize1;
    int FontSize2;

    const char *ApplicationFont0;
    const char *ApplicationFont1;
    const char *ApplicationFont2;
    
    vtkSlicerCheckRadioButtonIcons *CheckRadioIcons;
    
 private:
    vtkSlicerTheme (const vtkSlicerTheme& ); // Not implemented
    void operator = (const vtkSlicerTheme& ); // Not implemented
    
};
#endif
