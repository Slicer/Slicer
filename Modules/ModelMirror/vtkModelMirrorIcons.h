#ifndef __vtkModelMirrorIcons_h
#define __vtkModelMirrorIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

#include "vtkSlicerIcons.h"

#include "vtkModelMirrorWin32Header.h"
#include "vtkModelMirror.h"
#include "./Resources/vtkModelMirror_ImageData.h"

class VTK_MODELMIRROR_EXPORT vtkModelMirrorIcons : public vtkSlicerIcons
{
 public:
    static vtkModelMirrorIcons* New ( );
    vtkTypeRevisionMacro ( vtkModelMirrorIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( AxialLOIcon, vtkKWIcon );
    vtkGetObjectMacro ( SaggitalLOIcon, vtkKWIcon );
    vtkGetObjectMacro ( CoronalLOIcon, vtkKWIcon );
    vtkGetObjectMacro ( AxialHIIcon, vtkKWIcon );
    vtkGetObjectMacro ( SaggitalHIIcon, vtkKWIcon );
    vtkGetObjectMacro ( CoronalHIIcon, vtkKWIcon );
    
    void AssignImageDataToIcons ( );
    
 protected:
    vtkModelMirrorIcons ( );
    ~vtkModelMirrorIcons ( );
    vtkKWIcon *AxialLOIcon;
    vtkKWIcon *SaggitalLOIcon;
    vtkKWIcon *CoronalLOIcon;
    vtkKWIcon *AxialHIIcon;
    vtkKWIcon *SaggitalHIIcon;
    vtkKWIcon *CoronalHIIcon;

    
 private:
    vtkModelMirrorIcons (const vtkModelMirrorIcons&); // Not implemented
    void operator = ( const vtkModelMirrorIcons& ); // Not implemented
    
};

#endif
