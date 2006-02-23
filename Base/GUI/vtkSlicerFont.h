#ifndef __vtkSlicerFont_h
#define __vtkSlicerFont_h

#include "vtkObject.h"

// Description:
// Color definitios used in Slicer's style.
//
class vtkSlicerFont : public vtkObject
{
 public:
    static vtkSlicerFont* New ( );
    vtkTypeRevisionMacro ( vtkSlicerFont, vtkObject );

    
    // fonts 
    char *AdobeHelvetica8;
    char *AdobeHelvetica10;
    char *AdobeHelvetica12;

    // text justification
    char *JustifyLeft;
    char *JustifyRight;
    char *JustifyCenter;
    char *JustifyFull;

    char *GetFontAsFamily(char *fontstring);
    
protected:
    
    vtkSlicerFont ( );
    ~vtkSlicerFont ( );
        
 private:
    vtkSlicerFont ( const vtkSlicerFont&); // Not implemented
    void operator = ( const vtkSlicerFont&); // Not implemented
};

#endif
