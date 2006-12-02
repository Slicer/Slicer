#ifndef __vtkSlicerViewControlIcons_h
#define __vtkSlicerViewControlIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerRotateAround_ImageData.h"
#include "./Resources/vtkSlicerLookFrom_ImageData.h"
#include "./Resources/vtkSlicerNavZoom_ImageData.h"
#include "./Resources/vtkSlicerViewControl_ImageData.h"

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

    vtkGetObjectMacro (ViewAxisAIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisAIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisPIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisPIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisRIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisRIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisLIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisLIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisSIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisSIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisIIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisIIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisBottomCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisTopCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisCenterIcon, vtkKWIcon );    

    vtkGetObjectMacro (SpinButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RockButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (OrthoButtonIcon, vtkKWIcon );    
    vtkGetObjectMacro (PerspectiveButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (CenterButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (SelectButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (StereoButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (LookFromButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundButtonIcon, vtkKWIcon );

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
    virtual ~vtkSlicerViewControlIcons ( );

    vtkKWIcon *ViewAxisAIconLO;
    vtkKWIcon *ViewAxisAIconHI;
    vtkKWIcon *ViewAxisPIconLO;
    vtkKWIcon *ViewAxisPIconHI;
    vtkKWIcon *ViewAxisRIconLO;
    vtkKWIcon *ViewAxisRIconHI;
    vtkKWIcon *ViewAxisLIconLO;
    vtkKWIcon *ViewAxisLIconHI;
    vtkKWIcon *ViewAxisSIconLO;
    vtkKWIcon *ViewAxisSIconHI;
    vtkKWIcon *ViewAxisIIconLO;
    vtkKWIcon *ViewAxisIIconHI;
    vtkKWIcon *ViewAxisBottomCornerIcon;
    vtkKWIcon *ViewAxisTopCornerIcon;
    vtkKWIcon *ViewAxisCenterIcon;

    vtkKWIcon *SpinButtonIcon;
    vtkKWIcon *RockButtonIcon;
    vtkKWIcon *OrthoButtonIcon;
    vtkKWIcon *PerspectiveButtonIcon;
    vtkKWIcon *CenterButtonIcon;
    vtkKWIcon *SelectButtonIcon;
    vtkKWIcon *StereoButtonIcon;
    vtkKWIcon *LookFromButtonIcon;
    vtkKWIcon *RotateAroundButtonIcon;    

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
