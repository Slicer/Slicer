#ifndef __vtkSlicerFoundationIcons_h
#define __vtkSlicerFoundationIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerFoundation_ImageData.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFoundationIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerFoundationIcons* New ( );
    vtkTypeRevisionMacro (vtkSlicerFoundationIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( GoIcon, vtkKWIcon );
    vtkGetObjectMacro ( DoneIcon, vtkKWIcon );
    vtkGetObjectMacro ( CameraIcon, vtkKWIcon );

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerFoundationIcons ( );
    virtual ~vtkSlicerFoundationIcons ( );
    vtkKWIcon *GoIcon;
    vtkKWIcon *DoneIcon;
    vtkKWIcon *CameraIcon;
    
 private:
    vtkSlicerFoundationIcons ( const vtkSlicerFoundationIcons&); // Not implemented
    void operator = (const vtkSlicerFoundationIcons& ); // not implemented.
    
};
#endif
