#ifndef __vtkSlicerViewControlIcons_h
#define __vtkSlicerViewControlIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerRotateAroundAHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundALO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundPHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundPLO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundSHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundSLO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundIHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundILO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundRHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundRLO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundLHI_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundLLO_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundBottomCorner_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundTopCorner_ImageData.h"
#include "./Resources/vtkSlicerRotateAroundMiddle_ImageData.h"
#include "./Resources/vtkSlicerLookFromAHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromALO_ImageData.h"
#include "./Resources/vtkSlicerLookFromPHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromPLO_ImageData.h"
#include "./Resources/vtkSlicerLookFromSHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromSLO_ImageData.h"
#include "./Resources/vtkSlicerLookFromIHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromILO_ImageData.h"
#include "./Resources/vtkSlicerLookFromRHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromRLO_ImageData.h"
#include "./Resources/vtkSlicerLookFromLHI_ImageData.h"
#include "./Resources/vtkSlicerLookFromLLO_ImageData.h"
#include "./Resources/vtkSlicerLookFromBottomCorner_ImageData.h"
#include "./Resources/vtkSlicerLookFromTopCorner_ImageData.h"
#include "./Resources/vtkSlicerLookFromMiddle_ImageData.h"
#include "./Resources/vtkSlicerNavZoomIn_ImageData.h"
#include "./Resources/vtkSlicerNavZoomOut_ImageData.h"


// Description:
// This class reads in the png format image data for logos and
// for icons in Slicer's main toolbar, and elsewhere in
// slicer's main window. It also creates vtkKWIcons and assigns
// the image data to them. (Elsewhere in the application,
// vtkKWLabels and vtkKWPushbuttons can display these
// vtkKWIcons instead of text.

class vtkSlicerViewControlIcons : public vtkSlicerIcons
{
 public:
    // Description:
    // Usual vtk functions
    static vtkSlicerViewControlIcons* New ();
    vtkTypeRevisionMacro ( vtkSlicerViewControlIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get on Auto camera movement icons
    vtkGetObjectMacro (RotateAroundAIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundAIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundPIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundPIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundRIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundRIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundLIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundLIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundSIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundSIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundIIconLO, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundIIconHI, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundBottomCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundTopCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundMiddleIcon, vtkKWIcon );    

    vtkGetObjectMacro (LookFromAIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromAIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromPIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromPIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromRIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromRIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromLIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromLIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromSIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromSIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromIIconLO, vtkKWIcon );
    vtkGetObjectMacro (LookFromIIconHI, vtkKWIcon );
    vtkGetObjectMacro (LookFromBottomCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (LookFromTopCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (LookFromMiddleIcon, vtkKWIcon );    

    vtkGetObjectMacro (NavZoomInIcon, vtkKWIcon);
    vtkGetObjectMacro (NavZoomOutIcon, vtkKWIcon);    

    // Description:
    // Method for setting images for widgets in Slicer's ViewControlFrame
    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerViewControlIcons ( );
    ~vtkSlicerViewControlIcons ( );

    vtkKWIcon *RotateAroundAIconLO;
    vtkKWIcon *RotateAroundAIconHI;
    vtkKWIcon *RotateAroundPIconLO;
    vtkKWIcon *RotateAroundPIconHI;
    vtkKWIcon *RotateAroundRIconLO;
    vtkKWIcon *RotateAroundRIconHI;
    vtkKWIcon *RotateAroundLIconLO;
    vtkKWIcon *RotateAroundLIconHI;
    vtkKWIcon *RotateAroundSIconLO;
    vtkKWIcon *RotateAroundSIconHI;
    vtkKWIcon *RotateAroundIIconLO;
    vtkKWIcon *RotateAroundIIconHI;
    vtkKWIcon *RotateAroundBottomCornerIcon;
    vtkKWIcon *RotateAroundTopCornerIcon;
    vtkKWIcon *RotateAroundMiddleIcon;

    vtkKWIcon *LookFromAIconLO;
    vtkKWIcon *LookFromAIconHI;
    vtkKWIcon *LookFromPIconLO;
    vtkKWIcon *LookFromPIconHI;
    vtkKWIcon *LookFromRIconLO;
    vtkKWIcon *LookFromRIconHI;
    vtkKWIcon *LookFromLIconLO;
    vtkKWIcon *LookFromLIconHI;
    vtkKWIcon *LookFromSIconLO;
    vtkKWIcon *LookFromSIconHI;
    vtkKWIcon *LookFromIIconLO;
    vtkKWIcon *LookFromIIconHI;
    vtkKWIcon *LookFromBottomCornerIcon;
    vtkKWIcon *LookFromTopCornerIcon;
    vtkKWIcon *LookFromMiddleIcon;

    vtkKWIcon *NavZoomInIcon;
    vtkKWIcon *NavZoomOutIcon;

    
 private:
    vtkSlicerViewControlIcons (const vtkSlicerViewControlIcons& ); // Not implemented
    void operator = (const vtkSlicerViewControlIcons& ); // Not implemented
    
};
#endif
