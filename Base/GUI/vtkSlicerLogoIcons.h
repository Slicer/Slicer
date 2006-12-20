#ifndef __vtkSlicerLogoIcons_h
#define __vtkSlicerLogoIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
//#include "./Resources/vtkSlicerLogo_ImageData.h"
#include "./Resources/vtkSlicerLogoHorizontal_ImageData.h"

class vtkSlicerLogoIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerLogoIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerLogoIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( SlicerLogo, vtkKWIcon);

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerLogoIcons ( );
    ~vtkSlicerLogoIcons ( );
    vtkKWIcon *SlicerLogo;
    
 private:
    vtkSlicerLogoIcons (const vtkSlicerLogoIcons&); // Not implemented
    void operator = ( const vtkSlicerLogoIcons& ); // Not implemented
    
};

#endif
