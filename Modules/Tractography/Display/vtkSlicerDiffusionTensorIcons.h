#ifndef __vtkSlicerDiffusionTensorIcons_h
#define __vtkSlicerDiffusionTensorIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "vtkSlicerTractographyDisplay.h"
#include "./Resources/vtkSlicerDiffusionTensor_ImageData.h"

class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerDiffusionTensorIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerDiffusionTensorIcons* New ( );
    vtkTypeMacro ( vtkSlicerDiffusionTensorIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get tract display mode vtkKWIcons
    vtkGetObjectMacro (LineIcon, vtkKWIcon);
    vtkGetObjectMacro (TubeIcon, vtkKWIcon);
    vtkGetObjectMacro (GlyphIcon, vtkKWIcon);

    virtual void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerDiffusionTensorIcons ( );
    virtual ~vtkSlicerDiffusionTensorIcons ( );
    
    vtkKWIcon *LineIcon;
    vtkKWIcon *TubeIcon;
    vtkKWIcon *GlyphIcon;


 private:
    vtkSlicerDiffusionTensorIcons (const vtkSlicerDiffusionTensorIcons&); // Not implemented
    void operator = ( const vtkSlicerDiffusionTensorIcons& ); // Not implemented
    
};

#endif
